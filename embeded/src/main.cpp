/* 620141014
JADA CAMPBELL
ECSE3038 PROJECT  */
#include <Arduino.h>
#include <Wifi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "env.h"
#include <cstdlib>
#include <iostream>
#include <string>
#define fanPin 22
#define lightPin 23
#define oneWirePin 19       //digital temp sensor
#define pirPin 18

OneWire oneWire(oneWirePin);          //instance of oneWire sensor 
DallasTemperature sensors(&oneWire);

void setup() 
{
  pinMode (fanPin, OUTPUT);
  pinMode (lightPin, OUTPUT);
  pinMode (pirPin, INPUT);
  sensors.begin();
  Serial.begin(9600);
  WiFi.begin(WIFI_USER, WIFI_PASS);
  Serial.println("Connecting");

  while(WiFi.status() != WL_CONNECTED)    
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" ");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() 
{
  if(WiFi.status()== WL_CONNECTED)
  {
    String endpoint = "https://i-likedthisprojectngl.onrender.com/embed";
    HTTPClient http;  
    http.begin(endpoint);

    http.addHeader("Content-Type", "application/json");
    StaticJsonDocument<1024> doc;               // Empty JSONDocument
    String httpRequestData;                     //Emtpy string to be used to store HTTP request data string

    sensors.requestTemperatures();              //reading temperature
    float celTemp = sensors.getTempCByIndex(0);
    doc["temp_reading"] = celTemp;
    Serial.print("temp: ");
    Serial.println(celTemp);
    Serial.println();
   
    bool presenceResult = digitalRead(pirPin);       //reading PIR sensor
    doc["presence"] = presenceResult;
    Serial.print("Presence: ");
    Serial.println(presenceResult);
    Serial.println();
  
    serializeJson(doc, httpRequestData);    //copies json doc into httpRequestData
    int httpResponseCode = http.PUT(httpRequestData);   //PUT REQUEST, returns response code
    String http_response;
    Serial.println("put success");
    if (httpResponseCode>0) 
    {
      Serial.print("HTTP Response code from request: ");
      Serial.println(httpResponseCode);

      Serial.print("HTTP Response from server: ");
      http_response = http.getString();
      Serial.println(http_response);
      Serial.println(" ");
    }
    else                                      //if http response code is negative
    {
      Serial.print("Error code: ");
      http_response = http.getString();
      Serial.println(httpResponseCode);
    }
    http.end();

    //GET REQUEST:
    Serial.println("GET REQUEST");
    http.begin(endpoint);

    int httpResponseCode2 = http.GET();      //performs get request and receives status code response
    
    if (httpResponseCode2>0) 
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode2);

      Serial.print("Response from server: ");
      http_response = http.getString();       //gets worded/verbose response
      Serial.println(http_response);
    }
    else 
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode2);
      Serial.print("error message: ");
      http_response = http.getString();       //gets worded/verbose response
      Serial.println(http_response);
    }
    
    StaticJsonDocument<1024> doc1;            //document to store deserialized json
    DeserializationError error = deserializeJson(doc1, http_response);
    if (error) 
    {
      Serial.print("Could not deserialize json");
      Serial.println(error.c_str());
      return;
    }
    const bool fanState = doc1["fan"];
    const bool lightState = doc1["light"]; 
    
    Serial.println("data:");
    Serial.print("fan state:   ");
    Serial.println(fanState);
    Serial.print("light state:  ");
    Serial.println(lightState);

    digitalWrite (fanPin, fanState);
    digitalWrite(lightPin, lightState);
    http.end();
  }
  else 
  {
    Serial.println("WiFi Disconnected");
  }
}