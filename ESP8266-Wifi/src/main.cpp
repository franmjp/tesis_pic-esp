#include <Arduino.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h> // For tests purpouses
#define RXpin 10            // MOD
#define TXpin 11            // MOD

void setup()
{
  // Configuro el baudRate del puerto serial
  Serial.begin(9600);

  WiFiManager wifiManager;

  //wifiManager.resetSettings();

  //Metodo bloqueante para la conexion al WiFi
  wifiManager.autoConnect("Tesis-ESP");

  //Mensaje para saber que se conecto
  Serial.println("connected...yeey :)");
}

bool enviarStringAServer(String datos, int tipo)
{
  // Creo el Cliente que generará los requests
  HTTPClient http;
  // Direccion a la cual le pegará el POST
  int httpCode;
  switch (tipo)
  {
  case 0:
    http.begin("http://ptsv2.com/t/emilio/post");
    http.addHeader("Content-Type", "application/json");
    httpCode = http.POST("{temperatura: '" + datos + "'}");
    break;
  case 1:
    http.begin("http://ptsv2.com/t/emilio/post");
    http.addHeader("Content-Type", "application/json");
    httpCode = http.POST("{puerta: '" + datos + "'}");
    break;
  case 2:
    http.begin("http://ptsv2.com/t/emilio/post");
    http.addHeader("Content-Type", "application/json");
    httpCode = http.POST("{corriente: '" + datos + "'}");
    break;
  default:
    httpCode = 321;
    break;
  }
  String payload = http.getString();
  // Serial.println(httpCode); //Print HTTP return code
  // Serial.println(payload);  //Print request response payload
  http.end(); //Close connection
  Serial.println("Request Enviado!");
  if (httpCode == 200)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void loop()
{
  if (Serial.available() > 0) //Checks is there any data in buffer
  {
    String tipoDato = Serial.readString();
    bool enviado = false;
    if (tipoDato.equalsIgnoreCase("temperatura"))
    {
      while (Serial.available() == 0)
      {
        // no hacer nada
      }
      enviado = enviarStringAServer(Serial.readString(), 0);
    }
    if (tipoDato.equalsIgnoreCase("puerta"))
    {
      while (Serial.available() == 0)
      {
        // no hacer nada
      }
      enviado = enviarStringAServer(Serial.readString(), 1);
    }
    if (tipoDato.equalsIgnoreCase("corriente"))
    {
      while (Serial.available() == 0)
      {
        // no hacer nada
      }
      enviado = enviarStringAServer(Serial.readString(), 2);
    }
    if (enviado)
    {
      Serial.println("Envio Exitoso de datos");
    }
    else
    {
      Serial.println("Si algo podia fallar, falló");
    }
  }
}