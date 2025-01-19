#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "Max6675.h"

const int thermoDO = D2;
const int thermoCS = D3;
const int thermoCLK = D4;

String t;
#define ON_Board_LED 2 

const char* ssid = "WIFI name";
const char* password = "password";

const char* host = "script.google.com";
const int httpsPort = xxx;

WiFiClient client;

long now = millis();
long lastMeasure = 0;

String GAS_ID = "GAS_ID";

Max6675 ts(thermoCLK, thermoCS, thermoDO);
const int Relay1 = D7;

void setup()
{
  ts.setOffset(0);
  Serial.begin(115200);
  Serial.println("MAX6675 test");
  delay(3000);
  Serial.println("");
  pinMode(ON_Board_LED, OUTPUT);
  digitalWrite(ON_Board_LED, HIGH);
  WiFi.disconnect();    
  WiFi.begin("WIFI name","password");   
  while ((!(WiFi.status() == WL_CONNECTED)))
  {     
    Serial.print(".");
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);   
  }
  digitalWrite(ON_Board_LED, HIGH);
  Serial.println("");
  Serial.print("Successfully conneted to : ");
  Serial.println(ssid);
  Serial.print("IP address : ");
  Serial.println(WiFi.localIP());
  Serial.println();

  WiFiClient connect();
}
void loop()
{
  now = millis();
  if(now - lastMeasure > 3000)
  {
    lastMeasure = now;
    float C = ts.getCelsius();
    float F = ts.getFahrenheit();
    float K = ts.getKelvin();
    //float f = ts.setoffset(true);
    if(isnan(C) || isnan(F) || isnan(K))
    {
      Serial.println("Failed to read from MAX6675 sensor!"); 
      return;
    }
    //float hic = ts.computureHeatIndex(C, F, K);
    static char temperatureTemp[7];
    //dtostrf(hic, 6, 2, temperatureTemp);
  }
  Serial.print(ts.getCelsius(), 2);
  Serial.print(" C / ");
  Serial.print(ts.getFahrenheit(), 2);
  Serial.print(" F / ");
  Serial.print(ts.getKelvin(), 2);
  Serial.print(" K\n");
  sendData(ts.getCelsius(), ts.getFahrenheit(), ts.getKelvin());
  
  if(ts.getCelsius() > 25)
  {
    pinMode(Relay1, OUTPUT);
    digitalWrite(Relay1, HIGH);
    delay(500);
  }
  else if(ts.getCelsius() <= 25)
  {
    digitalWrite(Relay1, LOW);
    delay(500);
  }
}

void sendData(float value1, float value2, float value3)
{
  Serial.println("==========");
  Serial.print("Connect to");
  Serial.println(host);

  if(!client.connect(host, httpsPort))
  {
    Serial.println("Connecttion Failed");
    return;
  }
  float string_Celsius = value1;
  float string_Fahrenheit = value2;
  float string_Kelvin = value3;
  String url = "/macross/s/" + GAS_ID + "/exec?temp(C)=" + string_Celsius + "&temp(F)" + string_Fahrenheit + "&temp(K)" + string_Kelvin;
  Serial.print("requesting URL : ");
  Serial.println(url);

  client.print(String("GET ") + url + "HTTP/1.1\r\n" + "Host : " + host + "\r\n" + "User-Agent : BuildFailureDetectorESP8266\r\n" + "Connection : close\r\n\r\n");
  Serial.println("request sent");

  while(client.connected())
  {
    String line = client.readStringUntil('\n');
    if(line == "\r")
    {
      Serial.println("Headers Received");
      break;
    }
    else if(line.startsWith("{\"state\":\"success\""))
    {
      Serial.println("esp8266/Arduino CI successfull!");
    }
    else
    {
      Serial.println("esp8266/Arduino CI has failled");
    }
  }
  Serial.print("reply was : ");
  //Serial.println(line);
  Serial.println("closing connection");
  Serial.println("================");
  Serial.println();
}
