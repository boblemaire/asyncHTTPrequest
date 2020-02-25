# asyncHTTPrequest

Asynchronous HTTP for ESP8266 and ESP32. 
Subset of HTTP.
Built on ESPAsyncTCP (AsyncTCP for ESP32)
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




Following is a snippet of code using this library, along with a sample of the debug output trace from normal operation.  The context is that this code is in a process that runs as a state machine to post data to an external server. Suffice it to say that none of the calls block, and that the program does other things while waiting for the readyState to become 4.

There are a few different methods available to synchronize on completion and to extract the resonse data, especially long responses.  This is a simpler example.

```
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
    }>
```
and here's what the debug trace looks like:
```
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
Debug(312): _onData handler HTTP/1.1 200 OK... (198)
Debug(313): _collectHeaders()
Debug(315): _setReadyState(2)
Debug(315): _setReadyState(3)
Debug(315): *all data received - closing TCP
Debug(319): _onDisconnect handler
Debug(321): _setReadyState(4)
Debug(921): responseText() ok... (2)
```
