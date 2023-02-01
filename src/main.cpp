#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "DHT.h"
#include <MQUnifiedsensor.h>

// DHT Sensor
#define DHTPIN 33
#define DHTTYPE DHT22


// Air Quality Sensor
#define MQ135_THRESHOLD_1 1000 // Fresh Air threshold
#define placa "ESP-32"
#define Voltage_Resolution 3.3
#define pin 34
#define mqType "MQ-135"
#define ADC_Bit_Resolution 12
#define RatioMQ135CleanAir 3.6
double CO2 = (0);
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, mqType);

const char WIFI_SSID[] = "HOME";
const char WIFI_PASSWORD[] = "AkJ197219992001";

WebServer webServer(80);

// DHT
DHT dht(DHTPIN, DHTTYPE);
float humidity;
float temperature;

// *********************************************************************
// this function examines the URL from the client and based on the extension
// determines the type of response to send.
bool loadFromLittleFS(String path)
{
  bool bStatus;
  String contentType;

  bStatus = false; // set bStatus to false assuming this will not work.

  contentType = "text/plain"; // assume this will be the content type returned, unless path extension indicates something else

  // DEBUG:  print request URI to user:
  Serial.print("Requested URI: ");
  Serial.println(path.c_str());

  // if no path extension is given, assume index.html is requested.
  if (path.endsWith("/"))
    path += "index.html";

  // look at the URI extension to determine what kind of data to send to client.
  if (path.endsWith(".src"))
    path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html"))
    contentType = "text/html";
  else if (path.endsWith(".htm"))
    contentType = "text/html";
  else if (path.endsWith(".css"))
    contentType = "text/css";
  else if (path.endsWith(".js"))
    contentType = "application/javascript";
  else if (path.endsWith(".png"))
    contentType = "image/png";
  else if (path.endsWith(".gif"))
    contentType = "image/gif";
  else if (path.endsWith(".jpg"))
    contentType = "image/jpeg";
  else if (path.endsWith(".ico"))
    contentType = "image/x-icon";
  else if (path.endsWith(".xml"))
    contentType = "text/xml";
  else if (path.endsWith(".pdf"))
    contentType = "application/pdf";
  else if (path.endsWith(".zip"))
    contentType = "application/zip";

  // try to open file in LittleFS filesystem
  File dataFile = LittleFS.open(path.c_str(), "r");
  // if dataFile <> 0, then it was opened successfully.
  if (dataFile)
  {
    if (webServer.hasArg("download"))
      contentType = "application/octet-stream";
    // stream the file to the client.  check that it was completely sent.
    if (webServer.streamFile(dataFile, contentType) != dataFile.size())
    {
      Serial.println("Error streaming file: " + String(path.c_str()));
    }
    // close the file
    dataFile.close();
    // indicate success
    bStatus = true;
  }
  return bStatus;
}

// *********************************************************************
void handleWebRequests()
{
  if (!loadFromLittleFS(webServer.uri()))
  {
    // file not found.  Send 404 response code to client.
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += webServer.uri();
    message += "\nMethod: ";
    message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += webServer.args();
    message += "\n";
    for (uint8_t i = 0; i < webServer.args(); i++)
    {
      message += " NAME:" + webServer.argName(i) + "\n VALUE:" + webServer.arg(i) + "\n";
    }
    webServer.send(404, "text/plain", message);
    Serial.println(message);
  }
}

// ***********************************************************
void mqSetup()
{
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.setA(110.47);
  MQ135.setB(-2.862);

  // Configurate the ecuation values to get NH4 concentration
  MQ135.init();
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;

  for (int i = 1; i <= 10; i++)
  {
    MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }

  MQ135.setR0(calcR0 / 10);
  Serial.println("  done!.");

  if (isinf(calcR0))
  {
    Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply");
    while (1)
      ;
  }
  if (calcR0 == 0)
  {
    Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply");
    while (1)
      ;
  }

  Serial.println("** Values from MQ-135 ****");
  Serial.println("|    CO   |  Alcohol |   CO2  |  Toluen  |  NH4  |  Aceton  |");

  // MQ CAlibration
  MQ135.serialDebug(false);
}

// ***********************************************************
void checkInput()
{
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  MQ135.update(); // Update data, the arduino will read the voltage from the analog pin

  MQ135.setA(605.18);
  MQ135.setB(-3.937);            // Configure the equation to calculate CO concentration value
  float CO = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(77.255);
  MQ135.setB(-3.18);                  // Configure the equation to calculate Alcohol concentration value
  float Alcohol = MQ135.readSensor(); // SSensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(110.47);
  MQ135.setB(-2.862);             // Configure the equation to calculate CO2 concentration value
  float CO2 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(44.947);
  MQ135.setB(-3.445);                // Configure the equation to calculate Toluen concentration value
  float Toluen = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(102.2);
  MQ135.setB(-2.473);             // Configure the equation to calculate NH4 concentration value
  float NH4 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(34.668);
  MQ135.setB(-3.369);                // Configure the equation to calculate Aceton concentration value
  float Aceton = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  Serial.print("|   ");
  Serial.print(CO);
  Serial.print("   |   ");
  Serial.print(Alcohol);
  // Note: 400 Offset for CO2 source: https://github.com/miguel5612/MQSensorsLib/issues/29
  /*
  Motivation:
  We have added 400 PPM because when the library is calibrated it assumes the current state of the
  air as 0 PPM, and it is considered today that the CO2 present in the atmosphere is around 400 PPM.
  https://www.lavanguardia.com/natural/20190514/462242832581/concentracion-dioxido-cabono-co2-atmosfera-bate-record-historia-humanidad.html
  */
  Serial.print("   |   ");
  Serial.print(CO2 + 400);
  Serial.print("   |   ");
  Serial.print(Toluen);
  Serial.print("   |   ");
  Serial.print(NH4);
  Serial.print("   |   ");
  Serial.print(Aceton);
  Serial.println("   |");
  /*
    Exponential regression:
  GAS      | a      | b
  CO       | 605.18 | -3.937
  Alcohol  | 77.255 | -3.18
  CO2      | 110.47 | -2.862
  Toluen  | 44.947 | -3.445
  NH4      | 102.2  | -2.473
  Aceton  | 34.668 | -3.369
  */

  String json;
  json.reserve(88);
  json = "{\"time\":";
  json += millis();
  json += ", \"temperature\":";
  json += temperature;
  json += ", \"humidity\":";
  json += humidity;
  json += ", \"co\":";
  json += CO;
  json += ", \"alcohol\":";
  json += Alcohol;
  json += ", \"co2\":";
  json += (CO2 + 400);
  json += ", \"toluen\":";
  json += Toluen;
  json += ", \"nh4\":";
  json += NH4;
  json += ", \"aceton\":";
  json += Aceton;
  json += "}";
  Serial.println(json);
  webServer.send(200, "text/json", json); // Sends the json string to the web server

  delay(500); // Sampling frequency
}

// ***********************************************************
void wifiSetup()
{
  // Initialize File System
  LittleFS.begin();
  Serial.println("File System Initialized");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // handler for time and temperature
  webServer.on("/checkInput", HTTP_GET, checkInput);

  // send all other web requests here by default.
  // The filesystem will be searched for the requested resource
  webServer.onNotFound(handleWebRequests);

  // start web server and print it's address.
  webServer.begin();
  Serial.printf("\nWeb server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());
}

// ***********************************************************
void otaSetup()
{
  ArduinoOTA
      .onStart([]()
               {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(115200);

  dht.begin();

  mqSetup();
  wifiSetup();
  otaSetup();
}

void loop()
{
  ArduinoOTA.handle();
  webServer.handleClient();
}