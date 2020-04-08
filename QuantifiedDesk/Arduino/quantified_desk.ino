/*******************************************************************
This code implements the Quantified Desk project, which consists of
an Arduino/Genuino MKR1000 hooked up to an ultrasonic sensor to measure
the distance to the floor (height) of a standing desk.

It keeps track of the time duration (in minutes) in each state (Sitting/Standing)
which is defined by a distance threshold above which the desk is considered to be
in standing position, and below which it is considered in sitting position.

A time counter for each state increases if the corresponding state is the current state.
This data is periodically sent to Azure IoT Hub along with a timestamp. 

Complete Project with Dashboard visualization
https://github.com/vmehmeri/az-iot/QuantifiedDesk

PRE-REQUISITE
Setup Azure IoT Hub and upload SSL certificate to device (for your 
IoT Hub hostname). With older firmware versions the HTTP Post function
will be unreliable and may fail on several requests, so it's recommended
to update your MKR1000 firmware to the latest version.

  - Instructions for setting up Azure IoT Hub:
  https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-create-through-portal
  
  - Instructions for updating Firmware and adding SSL certificate to device:
  https://www.arduino.cc/en/Tutorial/FirmwareUpdater

 *******************************************************************/

#include <Ultrasonic.h>
#include <WiFi101.h>
#include <RTCZero.h>
#include <WiFiUdp.h>

#include "arduino_secrets.h"
// SECRETS CONFIG -- PLEASE SET THESE IN arduino_secrets.h FILE
//WiFi creds -------------------------------------------------------------------------------------------
char ssid[] = SECRET_SSID; //  your network SSID (name)
char pass[] = SECRET_WIFIPASSWD;    // your network password (use for WPA, or use as key for WEP)

//Azure IoT Hub Secrets Config -------------------------------------------------------------------------
char hostname[] = SECRET_IOTHUB_HOSTNAME;
char uri[]= SECRET_DEVICE_POST_URI;
char authSAS[] = SECRET_DEVICE_SAS;
//------------------------------------------------------------------------------------------------------

#define slotNumber 1 //This will vary for multi slot devices 

// Project Config variables
unsigned int distanceThreshold = 82; // Define here the distance (in cm) that marks the threshold between sitting and standing
const int GMT = 2; //change this to adapt it to your time zone
const int TrigPin = 4; //number of the Trigger pin
const int EchoPin = 5; //number of the Echo pin

RTCZero rtc;
WiFiSSLClient client;
Ultrasonic ultrasonic(TrigPin, EchoPin);

unsigned int count = 0;
unsigned long distanceSum = 0;
unsigned int timeStanding = 0;
unsigned int timeSitting = 0;
unsigned long startMillis; 
unsigned long distanceAvg;
unsigned long distance;

int status = WL_IDLE_STATUS;

void setup() {
  Serial.begin(9600);
  
  while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
    
   startMillis = millis();
   rtc.begin();
   
   // check for the presence of the shield:
   if (WiFi.status() == WL_NO_SHIELD) {
     Serial.println("WiFi shield not present");
     // don't continue:
     while (true);
   }

   // attempt to connect to Wifi network:
   while ( status != WL_CONNECTED) {
     Serial.print("Attempting to connect to SSID: ");
     Serial.println(ssid);
     status = WiFi.begin(ssid, pass);

     // wait 10 seconds for connection:
     delay(10000);
   }
  
  Serial.println("Connected to Wi-Fi");
  
  // Get Real-Time from NTP using built-in RTC module
  unsigned long epoch;
  int numberOfTries = 0, maxTries = 6;
  do {
    epoch = WiFi.getTime();
    numberOfTries++;
  }
  while ((epoch == 0) && (numberOfTries < maxTries));

  if (numberOfTries == maxTries) {
    Serial.print("NTP unreachable!!");
    while (1);
  }
  else {
    Serial.print("Epoch received: ");
    Serial.println(epoch);
    rtc.setEpoch(epoch);

    Serial.println();
  }
  
}

void loop() {

  delay(5000); // wait 5 seconds

  ultrasonic.measure();
  distance = ultrasonic.get_cm();
  
  Serial.print("Sensor(cm): ");
  Serial.println(distance);
  
  distanceSum = distanceSum + distance;
  count = count + 1;
  
  /* Takes the average of the last 12 measurements (the number 12 is arbitrary, but
   *  with a 5-second delay between measurements and 12 measurements, that means 
   *  data is aggregated and sent to Azure every 60 seconds, which seems reasonable for
   *  this project.
   */
  if (count == 12) {
    distanceAvg = distanceSum / count;
    count = 0;
    distanceSum = 0;

    if (distanceAvg < distanceThreshold) {
      // Add elapsed time since last measurement to sitting time
      timeSitting = timeSitting + ((millis()-startMillis)/1000);  
    } else {
      // Add elapsed time since last measurement to standing time
      timeStanding = timeStanding + ((millis()-startMillis)/1000);  
    }
    
    startMillis = millis();
    
    // Show current aggregate numbers
    printRTCDate();
    printRTCTime();
    Serial.println();
    Serial.print("Time sitting: ");
    Serial.print(timeSitting/60); 
    Serial.println("min");
    Serial.print("Time standing: ");
    Serial.print(timeStanding/60);
    Serial.println("min");
    Serial.println("");

    // Creates a string to send to Azure IoT HUB. 
    // It's simply comma-separated string of values for sitting and standing, followed by date and time (for timestamping)
    String data_string = (String(timeSitting/60) + "," + String(timeStanding/60) + "," + getRTCDate() + "," + getRTCTime());
    //Serial.println(data_string);
    
    // Send to Azure
    httpPost(data_string); 
    String response = "";
    char c;
    while (client.available()) {
      c = client.read();
      response.concat(c);
    }
  
  }

}


void httpPost(String content)
{
  
    if (client.connectSSL(hostname, 443)) {
      client.print("POST ");
      client.print(uri);
      client.println(" HTTP/1.1"); 
      client.print("Host: "); 
      client.println(hostname);
      client.print("Authorization: ");
      client.println(authSAS); 
      client.println("Connection: close");

      client.print("Content-Type: ");
      client.println("text/plain");
      client.print("Content-Length: ");
      client.println(content.length());
      client.println();
      client.println(content);
      delay(500);
      
    } else {
      Serial.println("HTTP POST connection failed");
      Serial.println(client.read());
    }
    
      // close connection
      client.stop();
}

void printRTCTime()
{
  print2digits(rtc.getHours() + GMT);
  Serial.print(":");
  print2digits(rtc.getMinutes());
  Serial.print(":");
  print2digits(rtc.getSeconds());
  Serial.println();
}

void printRTCDate()
{
  Serial.print(rtc.getDay());
  Serial.print("/");
  Serial.print(rtc.getMonth());
  Serial.print("/");
  Serial.print(rtc.getYear());

  Serial.print(" ");
}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}

String getRTCDate()
{
  String date_str = String(rtc.getDay()) + "/" + String(rtc.getMonth()) + "/" + String(rtc.getYear());
  return date_str;
}

String getRTCTime()
{
  String time_str = get2digits(rtc.getHours() + GMT) + ":" + get2digits(rtc.getMinutes()) + ":" + get2digits(rtc.getSeconds());
  return time_str;
}

String get2digits(int number) {
  if (number < 10) {
    return "0" + String(number);
  }
  return String(number);
}