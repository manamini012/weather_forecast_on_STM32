#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
const char* ssid = "1";
const char* password = "25102004";
WiFiClient wifiClient;
String receivedURL = "";

void setup() 
{
  Serial.begin(9600);
  delay(5000);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
  }
}

void loop() 
{  
  if (Serial.available()) 
  {
    char incomingChar = Serial.read();
    if (incomingChar == '\n') 
    {
      String jsonData = fetchWeatherData(receivedURL);
      Serial.println(jsonData + "!");
      receivedURL = "";
    }
    else 
    {
      receivedURL += incomingChar;
    }
  }
}

String trimWhiteSpace(String input) 
{
  input.trim(); 
  return input;
}

String fetchWeatherData(String URL) 
{
  if (WiFi.status() == WL_CONNECTED) 
  {
    URL = trimWhiteSpace(URL);
    HTTPClient http;
    http.begin(wifiClient, URL); 
    int httpCode = http.GET(); 
    if (httpCode > 0) 
    {
      if (httpCode == HTTP_CODE_OK) 
      {
        String payload = http.getString();
        http.end();
        return payload;
      } 
    } 
    http.end();
  } 
  return "{}?";
}
