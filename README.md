# asyncHTTPrequest

Fully asynchronous HTTP for ESP using ESPasyncTCP.
Adds subset of HTTP onto ESPasyncTCP.
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
