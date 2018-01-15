#include <Servo.h>
#include <ArduinoJson.h>
#include "WiFiEsp.h"
// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif
String section="header";

const char ssid[] = "";            // your network SSID (name)
const char pass[] = "";        // your network password
int status = WL_IDLE_STATUS;// the Wifi radio's status

const char server[] = "smkbatu8.000webhostapp.com";

Servo servo9;        //initialize a servo object for the connected servo  
Servo servo8;                
Servo servo10;
Servo servo11;
const uint8_t runOnce = 0;
uint8_t current = 0;
const uint8_t inputPin = 2; 
const uint8_t led1 = 12; 
const uint8_t led2 = 13; 
// Initialize the Ethernet client object
WiFiEspClient client;

void setup()
{
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  //key initialize at lock position for checking purpose
  servo9.attach(9);
  servo8.attach(8);
  servo10.attach(10);
  servo11.attach(11);
  servo9.write(50);
  servo10.write(180);
  servo11.write(60);
  delay(1000);
  servo8.write(180);
  servo10.write(90);
  // initialize serial for debugging
   Serial.begin(115200);
   // initialize serial for ESP module
   Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);
   // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }
  // you're connected now, so print out the data
  Serial.println("You're connected to the network");
  
  //printWifiStatus();
  Serial.println();
  Serial.println("Starting connection to server...");
  // if you get a connection, report back via serial
  //keystatus();
  //check();
}

//check the presence of the keys
void keystatus(){
  int sensorVal = digitalRead(3);
  int sensorVal2 = digitalRead(4);
  if (sensorVal == LOW) {
    //check 2 key at once
     if (sensorVal2 == LOW) {
          if (client.connect(server, 80)) {
              client.println("GET /allpres.php HTTP/1.1");
              client.print("Host: ");
              client.println("smkbatu8.000webhostapp.com");
              client.println("Connection: close");
              client.println();
              }
              Serial.println("Key 1 and 2 present");
        } 
        else {
          if (client.connect(server, 80)) {
              client.println("GET /key1preskey2abs.php HTTP/1.1");
              client.print("Host: ");
              client.println("smkbatu8.000webhostapp.com");
              client.println("Connection: close");
              client.println();
              }
              Serial.println("Key 1 present, key 2 absent");
        }
   //check done
  } 
  else {   
   //check 2 key at once
            if (sensorVal2 == LOW) {
          if (client.connect(server, 80)) {
              client.println("GET /key1abskey2pres.php HTTP/1.1");
              client.print("Host: ");
              client.println("smkbatu8.000webhostapp.com");
              client.println("Connection: close");
              client.println();
              }
              Serial.println("Key 1 absent, key 2 present");
        } 
        else {
          if (client.connect(server, 80)) {
              client.println("GET /allabs.php HTTP/1.1");
              client.print("Host: ");
              client.println("smkbatu8.000webhostapp.com");
              client.println("Connection: close");
              client.println();
              }
              Serial.println("Key 1 and 2 absent");
        }
   //check done
  }
}

void check(){
    if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request
    client.print(String("GET ") + "/fetch.json" + " HTTP/1.1\r\n" +               "Host: " + "smkbatu8.000webhostapp.com" + "\r\n" +                "Connection: closed\r\n\r\n");
  }
 }
void(* resetFunc) (void) = 0;

void run1(){
    if(client.available()){
    String line = client.readStringUntil('\r');
    // Serial.print(line);    // we’ll parse the HTML body here
    if (section=="header") { // headers..
      Serial.print(".");
      if (line=="\n") { // skips the empty space at the beginning
         section="json";
      }
    }
    else if (section=="json") {  // print the good stuff
      section="ignore";
      String result = line.substring(1);      // Parse JSON
      int size = result.length() + 1;
      char json[size];
      result.toCharArray(json, size);
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& json_parsed = jsonBuffer.parseObject(json);
      if (!json_parsed.success())
      {
        Serial.println("parseObject() failed");
        return;
      }
      // Make the decision to lock and unlock the key
      if (strcmp(json_parsed["key1"], "locked") == 0) {
         //key1 do ntg
         servo10.write(180);
         delay(1000);
         servo11.write(180);
         //for servo 10
         Serial.println("Key 1 Locked, Key 2 Unlocked");
         delay(5000);
         client.stop();
         keystatus();
         client.stop();
      }
      else if (strcmp(json_parsed["key2"], "locked") == 0) {
        servo8.write(90);
        delay(1000);
        servo9.write(0);
        //key2 do ntg
        Serial.println("Key 2 Locked, Key 1 Unlocked");
        delay(5000);
        client.stop();
        keystatus();
        client.stop();
      }
       else if (strcmp(json_parsed["all"], "unlocked") == 0) {
        servo8.write(90);
        servo10.write(180);
        delay(1000);
        servo9.write(0);
        servo11.write(180);
       //for servo 10
        Serial.println("Key 1 and 2 Unlocked");
        delay(5000);
        client.stop();
        keystatus();
        client.stop();
      }
       else if (strcmp(json_parsed["all"], "locked") == 0) {
        //do ntg
          Serial.println("Key 1 and 2 Locked");
          delay(5000);
          client.stop();
          keystatus();
          client.stop();
      }
      else {
       // do ntg
        Serial.println("Error data fetched");
      }
    }
  }
}

void loop(){
  int sensorVal = digitalRead(3);
  int sensorVal2 = digitalRead(4);
  if(sensorVal == LOW){digitalWrite(led1, HIGH);}
  else if(sensorVal == HIGH){digitalWrite(led1, LOW);}
  if(sensorVal2 == LOW){digitalWrite(led2, HIGH);}
  else if(sensorVal2 == HIGH){digitalWrite(led2, LOW);} 
  if(current == runOnce){
    check();
    current = 1;
  }
  else{    
  }
  uint8_t val = digitalRead(inputPin); 
  if (val == HIGH) {
    resetFunc();
   }
  run1();
}
