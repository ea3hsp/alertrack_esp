// Libraries
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include "FS.h"

// Config struct
struct Config
{
  char myDRIVER[20];
  char myHOST[64];
  char mySSID[64];
  char myPASSWORD[64];
};

// time variables
unsigned long currentTime = 0;
unsigned long updatedTime = 0;
unsigned long delayTime = 5 * 60 * 1000;



// current gps point variables
float lat = 0;
float lng = 0;

// Global configuration
Config conf;

// Clients
WiFiClient espClient;
HTTPClient http;

// Load configuration file
bool loadConfig(Config &config)
{
  // Open from spiffs config file
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile)
  {
    Serial.println("Failed to open config file");
    return false;
  }
  size_t size = configFile.size();
  // Check the size of file
  if (size > 1024)
  {
    Serial.println("Config file size is too large");
    return false;
  }
  // content buffer
  std::unique_ptr<char[]> buf(new char[size]);
  // read config file
  configFile.readBytes(buf.get(), size);
  // Declare json object
  StaticJsonBuffer<512> jsonBuffer;
  // Parse content
  JsonObject &json = jsonBuffer.parseObject(buf.get());
  if (!json.success())
  {
    Serial.println("Failed to parse config file");
    return false;
  }

  strlcpy(conf.myDRIVER, json["myDRIVER"], sizeof(conf.myDRIVER));
  strlcpy(conf.myHOST, json["myHOST"], sizeof(conf.myHOST));
  // load wifi parameters
  strlcpy(conf.mySSID, json["mySSID"], sizeof(conf.mySSID));
  strlcpy(conf.myPASSWORD, json["myPASSWORD"], sizeof(conf.myPASSWORD));
  // close file handler
  configFile.close();
  return true;
}

// Disable wifi connection
void disableWifi()
{
  WiFi.disconnect(true);
  Serial.println("Turn wifi off.");
}

// Connect to wifi network
void connectToWiFi()
{
  // Declara com estació Wifi i desconneta d'un AP si ho estava abans
  WiFi.mode(WIFI_STA);
  //  WiFi.disconnect();
  delay(100);
  // Connexió al Wifi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(conf.mySSID);
  WiFi.begin(conf.mySSID, conf.myPASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // Serial output messages
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void postGpsDriverPoint()
{
  // prevents resets
  //ESP.wdtFeed();
  //char json[] = "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
  char json[] ="{\"driver\":{\"name\":\"Albert\"},\"vehicle\":{\"type\":\"car\",\"brand\":\"Dacia\",\"model\":\"Lodggy\",	\"color\":\"blue\",	\"regisPlate\":\"\"	},\"point\":[42.56407944511193,-0.5709161516278983],\"timestamp\":1538205371}";

  if(WiFi.status()== WL_CONNECTED){
    //Specify request destination
    http.begin(String(conf.myHOST) + "/api/v1/driver/tracking");     
    //Specify content-type header
    http.addHeader("Content-Type", "application/json");
    //Send the request
    int httpCode = http.POST(String(json));
    //Get the response payload
    String payload = http.getString();
    //Print HTTP return code
    Serial.println(httpCode);
    //Print request response payload
    Serial.println(payload);    
    //Close connection
    http.end();  
  }else{
    Serial.println("Error in WiFi connection");   
 }
}

// Setup function
void setup()
{
  // Set port serial speed
  Serial.begin(9600);
  // Check for spiffs
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return;
  }
  // Load configuration from file
  if (!loadConfig(conf))
  {
    Serial.println("Failed to load config");
  }
  else
  {
    Serial.println("Config loaded");
  }
  // Connects to wifi network
  connectToWiFi();
  delay(1500);
  // First time call
  postGpsDriverPoint();
}


void loop(){
  // Current time
  currentTime = millis();
  // Check if time arrives
  if ( currentTime > updatedTime + delayTime) {
    updatedTime = currentTime;
    postGpsDriverPoint();
  }
}