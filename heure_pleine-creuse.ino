#include "FS.h"
#include "ESP8266WiFi.h"
#include "ESPAsyncWebServer.h"

// Replace with your network credentials
char* ssid = "Alexaya2.4";
char* password = "16092017a";

uint8_t RELAY_PIN = D2;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create schedule array to store time variables
String scheduleArray[4];

void fillArray(){
  if (SPIFFS.exists("/midi1.txt")){
    scheduleArray[0] = spiffsToString("/midi1.txt");
  }
  if (SPIFFS.exists("/midi2.txt")){
    scheduleArray[1] = spiffsToString("/midi2.txt");
  }
  if (SPIFFS.exists("/nuit1.txt")){
    scheduleArray[2] = spiffsToString("/nuit1.txt");
  }
  if (SPIFFS.exists("/nuit2.txt")){
    scheduleArray[3] = spiffsToString("/nuit2.txt");
  }
}

// Function to return as String the SPIFFS .txt file content
String spiffsToString(String fileDir){
  File f = SPIFFS.open(fileDir, "r");
  String fileContent;
  while (f.available()){
    fileContent += char(f.read());
  }
  f.close();
  return fileContent;
}


// Replaces placeholders
String processor(const String& var){
  if(var == "MIDI1"){
    if (scheduleArray[0].length() > 0) {
      return scheduleArray[0];
    }
    return "aucune";
  }
  else if(var == "MIDI2"){
    if (scheduleArray[1].length() > 0) {
      return scheduleArray[1];
    }
    return "aucune";
  }
  else if(var == "NUIT1"){
    if (scheduleArray[2].length() > 0) {
      return scheduleArray[2];
    }
    return "aucune";
  }
  else if(var == "NUIT2"){
    if (scheduleArray[3].length() > 0) {
      return scheduleArray[3];
    }
    return "aucune";
  }
}

int timeInMinutes(String timeInHoursAndMinutes){
  int hourNumber = timeInHoursAndMinutes.substring(0,2).toInt();
  int minuteNumber = timeInHoursAndMinutes.substring(3,5).toInt();
  return (hourNumber*60) + minuteNumber;
}

int getActualTimeInMinutes(){

  time_t timestamp = time( NULL );
  struct tm *now = localtime(&timestamp );
  return (now->tm_hour*60) + now->tm_min;
  
}

bool compareScheduleArrayWithActualTime(int startHour, int endHour){

  int actualTimeInMinutes = getActualTimeInMinutes();
  
  if( endHour < startHour ) return ( ( actualTimeInMinutes <= endHour) || (actualTimeInMinutes >= startHour) );
  else return ( startHour <= actualTimeInMinutes && actualTimeInMinutes <= endHour );

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  pinMode(RELAY_PIN, OUTPUT);

  fillArray();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Define timezone with summer/winter hours
  configTzTime("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", "fr.pool.ntp.org"); 

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to delete saved time in SPIFFS 
  server.on("/resetTime", HTTP_GET, [](AsyncWebServerRequest *request){
    SPIFFS.remove("/midi1.txt");
    scheduleArray[0] = "";
    SPIFFS.remove("/midi2.txt");
    scheduleArray[1] = "";
    SPIFFS.remove("/nuit1.txt");
    scheduleArray[2] = "";
    SPIFFS.remove("/nuit2.txt");
    scheduleArray[3] = "";
    request->redirect("/");
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {    
  // GET input value on <ESP_IP>/get?input=<value>
    if (request->getParam("midi1")->value().length() > 0) {
      scheduleArray[0] = request->getParam("midi1")->value();
      File midi1 = SPIFFS.open("/midi1.txt", "w");
      midi1.print(scheduleArray[0]);
      midi1.close();
      Serial.println("une valeur a été saisie dans midi 1");
    }
    if (request->getParam("midi2")->value().length() > 0) {
      scheduleArray[1] = request->getParam("midi2")->value();
      File midi2 = SPIFFS.open("/midi2.txt", "w");
      midi2.print(scheduleArray[1]);
      midi2.close();
      Serial.println("une valeur a été saisie dans midi 2");
    }
    if (request->getParam("nuit1")->value().length() > 0) {
      scheduleArray[2] = request->getParam("nuit1")->value();
      File nuit1 = SPIFFS.open("/nuit1.txt", "w");
      nuit1.print(scheduleArray[2]);
      nuit1.close();
      Serial.println("une valeur a été saisie dans nuit 1");
    }
    if (request->getParam("nuit2")->value().length() > 0) {
      scheduleArray[3] = request->getParam("nuit2")->value();
      File nuit2 = SPIFFS.open("/nuit2.txt", "w");
      nuit2.print(scheduleArray[3]);
      nuit2.close();
      Serial.println("une valeur a été saisie dans nuit 2");
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  server.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  if ( compareScheduleArrayWithActualTime(timeInMinutes(scheduleArray[0]), timeInMinutes(scheduleArray[1]) ) {
    digitalWrite(RELAY_PIN, LOW);
  } else if ( compareScheduleArrayWithActualTime(timeInMinutes(scheduleArray[2], timeInMinutes(scheduleArray[3]) ) {
    digitalWrite(RELAY_PIN, LOW);
  } else {
    digitalWrite(RELAY_PIN, HIGH);
  }
   delay(10000);
}
