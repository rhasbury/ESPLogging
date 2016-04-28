/*
  This a simple example of the aREST Library for the ESP8266 WiFi chip.
  See the README file for more details.

  Written in 2015 by Marco Schwartz under a GPL license.
*/

// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>
#include <Wire.h>
#include <APDS9930.h>


// Create aREST instance
aREST rest = aREST();

// WiFi parameters
const char* ssid = "grhl2";
const char* password = "$Service";

// The port to listen for incoming TCP connections
#define LISTEN_PORT           80


// Create an instance of the server
WiFiServer server(LISTEN_PORT);

// Variables to be exposed to the API
int temperature;
int humidity;
int ambient_light;

// Declare functions to be exposed to the API
int ledControl(String command);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  // Init variables and expose them to REST API
  temperature = 24;
  humidity = 40;
  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);
  rest.variable("ambient_light",&ambient_light);

  // Function to be exposed
  rest.function("led",ledControl);

  rest.function("ambient_light",ambient_light_read);

  // Give name and ID to device
  rest.set_id("1");
  rest.set_name("esp8266");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {

  // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  rest.handle(client);

}

// Custom function accessible by the API
int ledControl(String command) {

  // Get state from command
  int state = command.toInt();

  digitalWrite(6,state);
  return 1;
}


// Custom function accessible by the API
int ambient_light_read(String command) {
  APDS9930 apds = APDS9930();
  uint16_t ch0 = 0;
  uint16_t ch1 = 1;
  float ambient_light = 0;
  // Get state from command
  // Initialize APDS-9930 (configure I2C and initial values)
  if ( apds.init() ) { Serial.println("APDS-9930 initialization complete");  } 
  else { Serial.println("Something went wrong during APDS-9930 init!"); }
  
  // Start running the APDS-9930 light sensor (no interrupts)
  if ( apds.enableLightSensor(false) ) {
    Serial.println("Light sensor is now running");
  } else {
    Serial.println("Something went wrong during light sensor init!");
  }


  if (  !apds.readAmbientLightLux(ambient_light) ||
        !apds.readCh0Light(ch0) || 
        !apds.readCh1Light(ch1) ) {
          Serial.println("Error reading light values");
  } else {
    Serial.print("Ambient: ");
    Serial.print(ambient_light);
    Serial.print("  Ch0: ");
    Serial.print(ch0);
    Serial.print("  Ch1: ");
    Serial.println(ch1);
  }

  return ambient_light;
}



