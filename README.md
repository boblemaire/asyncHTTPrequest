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

Simple to use.

More details to follow - or read the code.
