/*************************************************** 
  This is a sketch to test the camera module
  
  Written by Marco Schwartz for Open Home Automation.
 ****************************************************/

// Include camera libraries
#include <Adafruit_VC0706.h>
#include <SoftwareSerial.h>      

// Software serial & camera instance
SoftwareSerial cameraconnection = SoftwareSerial(A0, A1);
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

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
 
  // Wait 3 seconds before taking a picture
  Serial.println("Picture in 3 seconds ...");
  delay(3000);

  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
}

void loop() {
}

