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
