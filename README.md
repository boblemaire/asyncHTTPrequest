# asyncHTTPrequest

Asynchronous HTTP for ESP8266 and possibly ESP32 (untested). 
Subset of HTTP.
Built on ESPAsyncTCP.
Methods similar in format and use to XmlHTTPrequest in Javascript.

Supports:
* GET and POST
* Request and response headers
* Chunked response
* Single String response for short (<~5K) responses (heap permitting).
* optional onData callback.
* optional onReadyStatechange callback.

This library adds a simple HTTP layer on top of the ESPAsyncTCP library to facilitate REST communication from a client to a server. The paradigm is similar to the XMLHttpRequest in Javascript, employing the notion of a ready-state progression through the transaction request.

Synchronization can be accomplished using callbacks on ready-state change, a callback on data receipt, or simply polling for ready-state change. Data retrieval can be incremental as received, or bulk retrieved when the transaction completes provided there is enough heap to buffer the entire response.

The underlying buffering uses a new xbuf class. It handles both character and binary data. xbuf uses a chain of small (64 byte) segments that are allocated and added to the tail as data is added and deallocated from the head as data is read, achieving the same result as a dynamic circular buffer limited only by the size of heap. The xbuf implements indexOf and readUntil functions.

For short transactions, buffer space should not be an issue. In fact, it can be more economical than other methods that use larger fixed length buffers. Data is acked when retrieved by the caller, so there is some limited flow control to limit heap usage for larger transfers.

Request and response headers are handled in the typical fashion.

Chunked responses are recognized and handled transparently.

Testing has not been extensive, but it is a fairly lean library, and all of the functions were tested to some degree. It is working flawlessly in the application for which it was designed.

Possibly I'll revisit this in the future and add support for additional HTTP request types like PUT.

See the Wiki for an explanation of the various methods.

Following is a snippet of code using this library, along with a sample of the debug output trace from normal operation.  The context is that this code is in a process that runs as a state machine to post data to an external server.  There is a scheduler that dispatches the service code and the service must return with the time that it wants to run again.  When it returns 1, that means it will be dispatched again as soon as higher priority tasks are completed.  If it returns with a value of unixtime that is in the future, it will be dispatched again at that time.  Resolution is one second.  With this async stuff, higher resolution would be nice.

So the prior state "post" accumulates a string of data that must be sent to the server.  When it has enough, state goes to sendPost and this is where the story begins.  

First a complete URL is constructed. 
hen we tell asyncHTTPrequest (AHR) to timeout after 1 second of inactivity. 
Set the debug trace on.
Drop a timestamp into the Serial stream to make this meaningful.
Open the request.  The URL is parsed, the connect request is initiated.
Add a couple of needed headers.
Send the request data (POST). The request stream is built and queued. If the connection has been established (typically not at this stage) the request will be sent.
Set the service state to the waitPost case and return for immediate redispatch.

At this point, the IoTaWatt (that's the application running this code) scheduler will continue to dispatch this service with respect to higher priority tasks like sampling AC power cycles.  Each time it gets around to dispatching this service, control passes to the waitPost case. Meanwhile, stuff is happening asynchronously.  When the onConnect callback is entered, ready state goes to 1 and the request payload is sent.  When the response comes in and the onData handler is entered, the the response is copied to a buffer and the headers are stripped off.  In this example, when all of the data is in, ready state is set to 4... 

If readyState has not gone to 4 (Done) then the process just return for redispatch in one second.  There is no time pressure, so just let other things run and come back when it's soup. There are other ways to synchronize for more time sensitive or longer responses.
When ready state is 4, the response text is read and checked for success.
The process state is set to build another post request in about 9 seconds.

The important thing here is that there is no blocking.  The IoTaWatt is sampling an electric power cycle for 20ms (50Hz) and then running these processes during the next 10ms half-cycle, after which it samples another AC cycle. So not blocking to connect or wait for a response enables steady sampling.


asyncHTTPrequest request;
	.
	.
	.

case sendPost:{
      String URL = EmonURL + ":" + String(EmonPort) + EmonURI + "/input/bulk";
      request.setTimeout(1);
      request.setDebug(true);
	  if(request.debug()){
        DateTime now = DateTime(UNIXtime() + (localTimeDiff * 3600));
        String msg = timeString(now.hour()) + ':' + timeString(now.minute()) + ':' + timeString(now.second());
        Serial.println(msg);
      }
      request.open("POST", URL.c_str());
      String auth = "Bearer " + apiKey;
      request.setReqHeader("Authorization", auth.c_str());
      request.setReqHeader("Content-Type","application/x-www-form-urlencoded");
      request.send(reqData);
      state = waitPost;
      return 1;
    } 

    case waitPost: {
      if(request.readyState() != 4){
        return UNIXtime() + 1; 
      }
      if(request.responseHTTPcode() != 200){
        msgLog("EmonService: Post failed: ", request.responseHTTPcode);  
        state = sendPost;
        return UNIXtime() + 1;
      }
      String response = request.responseText();
      if( ! response.startsWith("ok")){
        msgLog("EmonService: response not ok. Retrying.");
        state = sendPost;
        return UNIXtime() + 1;
      }
      reqData = "";
      reqEntries = 0;    
      state = post;
      return UnixNextPost;
    }

and here's what the debug trace looks like:

Debug(1991): setDebug(on)
16:47:50
Debug(1992): open(POST, emoncms.org:80/input/bulk)
Debug(  0): _parseURL() HTTP://emoncms.org:80/input/bulk
Debug(  3): _connect()
Debug(  6): send(String) time=1519854470&... (43)
Debug( 10): _buildRequest()
Debug( 12): _send() 230
Debug( 14): *can't send
Debug(190): _onConnect handler
Debug(190): _setReadyState(1)
Debug(191): _send() 230
Debug(191): *sent 230
Debug(312): _onData handler HTTP/1.1 200 OK
... (198)
Debug(313): _collectHeaders()
Debug(315): _setReadyState(2)
Debug(315): _setReadyState(3)
Debug(315): *all data received - closing TCP
Debug(319): _onDisconnect handler
Debug(321): _setReadyState(4)
Debug(921): responseText() Debug(921): responseText() ok... (2)
