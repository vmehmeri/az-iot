

/*******************************************************************
This code implements the Quantified Desk project, which consists of
an Arduino/Genuino MKR1000 hooked up to an ultrasonic sensor to measure
the distance to the floor (height) of a standing desk.

It keeps track of the time duration (in minutes) in each state (Sitting/Standing)
which is defined by a distance threshold above which the desk is considered to be
in standing position, and below which it is considered in sitting position.

This data is periodically sent to Azure IoT Hub. 

Arduino ultrasonic library
https://github.com/JRodrigoTech/Ultrasonic-HC-SR04

WiFi101 library
https://github.com/arduino-libraries/WiFi101

Complete Project with Dashboard visualization
https://github.com/vmehmeri/az-iot/QuantifiedDesk

PRE-REQUISITE
Setup Azure IoT Hub and upload SSL certificate to device (for your 
IoT Hub hostname)

 *******************************************************************/

#include "Ultrasonic.h"
#include <WiFi101.h>
//#include <ArduinoHttpClient.h>

//WiFi creds ----------------------------------------------------------------------------------
char ssid[] = ""; //  your network SSID (name)
char pass[] = "";    // your network password (use for WPA, or use as key for WEP)
//------------------------------------------------------------------------------------------------------

#define slotNumber 1 //This will vary for multi slot devices 

const int TrigPin = 4;     //DIO number of the trigger pin
const int EchoPin = 5;     //DIO number of the echo pin

///*** Azure IoT Hub Config ***///
char hostname[] = "<HUB-NAME>.azure-devices.net";    // host name address for your Azure IoT Hub
char feeduri[] = "/devices/<DEVICE-NAME>/messages/devicebound?api-version=2016-02-03"; //feed URI 
char uri[]= "/devices/<DEVICE-NAME>/messages/events?api-version=2016-02-03";// POST Uri
char authSAS[] = "SharedAccessSignature sr=<hub-NAME>.azure-devices.net%2Fdevices%2F<DEVICE-NAME>&sig=<SIGNATURE>"; 

// 7800 Gives a maximum distance of a little over 100cm
Ultrasonic ultrasonic(TrigPin, EchoPin, 7800);

// Define here the distance (in cm) that marks the threshold between sitting and standing
unsigned int DISTANCE_THRESHOLD = 82

int sensor;
unsigned int count = 0;
unsigned long distance_sum = 0;
unsigned int time_standing = 0;
unsigned int time_sitting = 0;
unsigned long startMillis; 
unsigned long distance_avg;
unsigned long distance;

int status = WL_IDLE_STATUS;

WiFiSSLClient client;

void setup() {
  Serial.begin(9600);
  startMillis = millis();
  //pinMode(EchoPin, INPUT);
  //pinMode(TrigPin, OUTPUT);
  
  while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
   }

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
  
}

void loop() {

  delay(5000); // wait 5 seconds

  distance = ultrasonic.Ranging(CM);
  Serial.print("Sensor(cm): ");
  Serial.println(distance);
  
  distance_sum = distance_sum + distance;
  count = count + 1;
  distance_avg = distance_sum / count;
  
  /* Takes the average of the last 12 measurements (the number 12 is arbitrary, but
   *  with a 5-second delay between measurements and 12 measurements, that means 
   *  data is aggregated and sent to Azure every 60 seconds, which seems reasonable for
   *  this project.
   */
  if (count == 12) {
    count = 0;
    distance_sum = 0;

    if (distance_avg < DISTANCE_THRESHOLD) {
      // Add elapsed time since last measurement to sitting time
      time_sitting = time_sitting + ((millis()-startMillis)/1000);  
    } else {
      // Add elapsed time since last measurement to standing time
      time_standing = time_standing + ((millis()-startMillis)/1000);  
    }
    
    startMillis = millis();
    
    // Show current aggregate numbers
    Serial.print("Time sitting: ");
    Serial.print(time_sitting/60); 
    Serial.println("min");
    Serial.print("Time standing: ");
    Serial.print(time_standing/60);
    Serial.println("min");
    Serial.println("");

    // Creates a string to send to Azure IoT HUB. 
    // It's simply comma-separated string of values for sitting and standing
    String data_string = (String(time_sitting/60) + "," + String(time_standing/60));
    
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
