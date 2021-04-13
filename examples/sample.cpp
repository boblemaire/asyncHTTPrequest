//************************************************************************************************************
//
// There are scores of ways to use asyncHTTPrequest.  The important thing to keep in mind is that
// it is asynchronous and just like in JavaScript, everything is event driven.  You will have some
// reason to initiate an asynchronous HTTP request in your program, but then sending the request
// headers and payload, gathering the response headers and any payload, and processing
// of that response, can (and probably should) all be done asynchronously.
//
// In this example, a Ticker function is setup to fire every 5 seconds to initiate a request.
// Everything is handled in asyncHTTPrequest without blocking.
// The callback onReadyStateChange is made progressively and like most JS scripts, we look for 
// readyState == 4 (complete) here.  At that time the response is retrieved and printed.
//
// Note that there is no code in loop().  A code entered into loop would run oblivious to
// the ongoing HTTP requests.  The Ticker could be removed and periodic calls to sendRequest()
// could be made in loop(), resulting in the same asynchronous handling.
//
// For demo purposes, debug is turned on for handling of the first request.  These are the
// events that are being handled in asyncHTTPrequest.  They all begin with Debug(nnn) where
// nnn is the elapsed time in milliseconds since the transaction was started.
//
//*************************************************************************************************************


#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <asyncHTTPrequest.h>
#include <Ticker.h>

WiFiManager wifiManager;
asyncHTTPrequest request;
Ticker ticker;

void sendRequest(){
    if(request.readyState() == 0 || request.readyState() == 4){
        request.open("GET", "http://worldtimeapi.org/api/timezone/Europe/London.txt");
        request.send();
    }
}

void requestCB(void* optParm, asyncHTTPrequest* request, int readyState){
    if(readyState == 4){
        Serial.println(request->responseText());
        Serial.println();
        request->setDebug(false);
    }
}
    

void setup(){
    Serial.begin(115200);
    WiFi.setAutoConnect(true);
    WiFi.begin();
    while(WiFi.status() != WL_CONNECTED){
        wifiManager.setDebugOutput(false);
        wifiManager.setConfigPortalTimeout(180);
        Serial.println("Connecting with WiFiManager");
        wifiManager.autoConnect("ESP8266", "ESP8266");
        yield();
    }
    request.setDebug(true);
    request.onReadyStateChange(requestCB);
    ticker.attach(5, sendRequest);
}

void loop(){

}