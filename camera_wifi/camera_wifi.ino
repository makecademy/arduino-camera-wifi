/*************************************************** 
  This is a sketch to test the camera module with the CC3000 WiFi chip
  
  Written by Marco Schwartz for Open Home Automation
  Code inspired by the work done on the Adafruit_VC0706 & CC3000 libraries
  BSD license, all text above must be included in any redistribution
 ****************************************************/

// Include camera  
#include <Adafruit_VC0706.h>
#include <SoftwareSerial.h>
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"
#include<stdlib.h>

// Software serial
SoftwareSerial cameraconnection = SoftwareSerial(A0, A1);
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

// Define CC3000 chip pins
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

// WiFi network (change with your settings !)
#define WLAN_SSID       "yourNetwork"        // cannot be longer than 32 characters!
#define WLAN_PASS       "yourPassword"
#define WLAN_SECURITY   WLAN_SEC_WPA2 // This can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2

// Create CC3000 instances
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIV2);
                                         
// Local server IP, port, and repository (change with your settings !)
uint32_t ip = cc3000.IP2U32(192,168,0,1);
int port = 80;
String repository = "/arduino-camera-wifi/";                                         
                                      
void setup() {
 
  Serial.begin(115200);
  Serial.println("Camera test");
  
  // Try to locate the camera
  if (cam.begin()) {
    Serial.println("Camera found:");
  } else {
    Serial.println("Camera not found !");
    return;
  }
  
  // Set picture size
  cam.setImageSize(VC0706_640x480);
  
  // Initialise the module
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }

  // Connect to  WiFi network
  cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY);
  Serial.println(F("Connected!"));
    
  // Display connection details
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }
  
  cc3000.printIPdotsRev(ip);

  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
  
  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image.");
  
  // Prepare request
  String start_request = "";
  String end_request = "";
  start_request = start_request + "\n" + "--AaB03x" + "\n" + "Content-Disposition: form-data; name=\"picture\"; filename=\"CAM.JPG\"" + "\n" + "Content-Type: image/jpeg" + "\n" + "Content-Transfer-Encoding: binary" + "\n" + "\n";  
  end_request = end_request + "\n" + "--AaB03x--" + "\n";
  
  uint16_t extra_length;
  extra_length = start_request.length() + end_request.length();
  Serial.println("Extra length:");
  Serial.println(extra_length);
  
  uint16_t len = jpglen + extra_length;
  
   Serial.println("Full request:");
   Serial.println(F("POST /arduino-camera-wifi/camera.php HTTP/1.1"));
   Serial.println(F("Host: 192.168.0.1:80"));
   Serial.println(F("Content-Type: multipart/form-data; boundary=AaB03x"));
   Serial.print(F("Content-Length: "));
   Serial.println(len);
   Serial.print(start_request);
   Serial.print("binary data");
   Serial.print(end_request);
 
  Serial.println("Starting connection to server...");
  Adafruit_CC3000_Client client = cc3000.connectTCP(ip, port);
  
  // Connect to the server, please change your IP address !
  if (client.connected()) {
      Serial.println(F("Connected !"));
      client.println(F("POST /arduino-camera-wifi/camera.php HTTP/1.1"));
      client.println(F("Host: 192.168.0.1:80"));
      client.println(F("Content-Type: multipart/form-data; boundary=AaB03x"));
      client.print(F("Content-Length: "));
      client.println(len);
      client.print(start_request);
            
      // Read all the data up to # bytes!
      byte wCount = 0; // For counting # of writes
      while (jpglen > 0) {
        
        uint8_t *buffer;
        uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
        
        buffer = cam.readPicture(bytesToRead);
        client.write(buffer, bytesToRead);
    
        if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
          Serial.print('.');
          wCount = 0;
        }
        jpglen -= bytesToRead; 
      }
      
      client.print(end_request);
      client.println();
      
  Serial.println("Transmission over");
  } 
  
  else {
      Serial.println(F("Connection failed"));    
    }
    
     while (client.connected()) {

      while (client.available()) {

    // Read answer
        char c = client.read();
        Serial.print(c);
        //result = result + c;
      }
    }
    client.close();
  
}

void loop() {
}

