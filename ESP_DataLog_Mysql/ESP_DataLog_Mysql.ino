//#include <Adafruit_ADS1015.h>

#include <Adafruit_BMP085_U.h>
#include <Adafruit_Sensor.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <APDS9930.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <string.h>

#include "ADS1115.h"

ADS1115 adc0(0x49);

// Wire ADS1115 ALERT/RDY pin to Arduino pin 2
const int alertReadyPin = 13;

// constants for Sharp GP2Y1010AU0F Particle Sensor
const int ledPower = 12;
const int delayTime=280;
const int dustPin=A0;
const int delayTime2=40;

// disable sql logging
const int loggingenabled = 1;

int loopdelay = 600;
int dustSensorCoefficent = 32000;

// WiFi parameters

// WiFi parameters
const char* ssid = "ssid";
const char* wifi_password = "password";
// Address of mysql server
IPAddress server_addr(192, 168, 0, 105);


/* Setup for the Connector/Arduino */
WiFiClient client;
MySQL_Connection my_conn((Client *)&client);

// MySql credentials
char user[] = "monitor";
char db_password[] = "password";


Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
APDS9930 apds = APDS9930();


void setup() {

    // Start Serial
    Serial.begin(115200);
    // Setup digital pins
    pinMode(A0, INPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(D0, OUTPUT);
    pinMode(ledPower,OUTPUT);
  
    // Connect to BMP180
    if (!bmp.begin(BMP085_MODE_STANDARD)) {
                      Serial.println("Could not find a valid BMP085 sensor, check wiring!");
                      }
    
    
    // Initialize APDS-9930 (configure I2C and initial values)
    if ( apds.init() ) { 
                      Serial.println("APDS-9930 initialization complete");  
                      } 
    else { 
                      Serial.println("Something went wrong during APDS-9930 init!"); 
                      }
    
    // Start running the APDS-9930 light sensor (no interrupts)
    if ( apds.enableLightSensor(false) ) {
                      Serial.println("Light sensor is now running");
                      } 
    else {
                      Serial.println("Something went wrong during light sensor init!");
                      }
    
    // Setup for the ADS1115 ADC
    Wire.begin(2, 14);    
    Serial.println("Testing device connections...");
    Serial.println(adc0.testConnection() ? "ADS1115 connection successful" : "ADS1115 connection failed");
    adc0.initialize(); // initialize ADS1115 16 bit A/D chip
    // We're going to do single shot sampling
    adc0.setMode(ADS1115_MODE_SINGLESHOT);
    adc0.setRate(ADS1115_RATE_128);
    adc0.setGain(ADS1115_PGA_4P096);
    // ALERT/RDY pin will indicate when conversion is ready    
    pinMode(alertReadyPin,INPUT_PULLUP);
    adc0.setConversionReadyPinMode();
  
  
}

void loop() {
  
  while(WiFi.status() == WL_CONNECTED){
    uint16_t ch0 = 0;
    uint16_t ch1 = 1;
    float ambient_light = 0;
    char outstr[15];
    char INSERT_SQL[100] = {0};
    float dustVal;
    float RainVal;
    
    if (  !apds.readAmbientLightLux(ambient_light) ||
          !apds.readCh0Light(ch0) || 
          !apds.readCh1Light(ch1) ) {
            Serial.println("Error reading light values");
    } 
    else {
      // print ambient light level
      Serial.print("Ambient: ");
      Serial.print(ambient_light);
      Serial.print("\n");
      
      // Log to mysql database
      dtostrf(ambient_light,7, 3, outstr);
      char stringone[] = "INSERT INTO temps.lightdat VALUES (NOW(), \"NodeMCU\", ";
      char stringtwo[] = ")";      
      strcat(INSERT_SQL, stringone);
      strcat(INSERT_SQL, outstr);
      strcat(INSERT_SQL, stringtwo);
      
      logLine(INSERT_SQL);
      
    }

     
    Serial.print("about to set LED on: ");  
    digitalWrite(ledPower,HIGH); // power on the LED
    delayMicroseconds(delayTime);
    Serial.println("Led on. Now reading dust ");  
    //dustVal=analogRead(dustPin); // read the dust value
    adc0.setMultiplexer(ADS1115_MUX_P1_NG);
    adc0.triggerConversion();
    pollAlertReadyPin();
    dustVal = adc0.getMilliVolts(false); // * dustSensorCoefficent;    
    delayMicroseconds(delayTime2);
    digitalWrite(ledPower,LOW); // turn the LED off
    //delayMicroseconds(offTime);
    

      
    Serial.print("Dust Reading: ");
    Serial.print(dustVal);
    Serial.print("\n");
  
    memset( INSERT_SQL, 0, sizeof(INSERT_SQL) );
    char stringone[] = "INSERT INTO temps.gasdat VALUES (NOW(), 'NodeMCU', 'GY-Dust', ";
    char stringtwo[] = ")";      
    strcat(INSERT_SQL, stringone);
    strcat(INSERT_SQL, itoa(dustVal,outstr,10));
    strcat(INSERT_SQL, stringtwo);
    logLine(INSERT_SQL);
      

    sensors_event_t event;
    bmp.getEvent(&event);     
    if (event.pressure)
    {
      //displaySensorDetails();
      /* Display atmospheric pressue in hPa */
      //Serial.print("Pressure:    ");
      //Serial.print(event.pressure);
      //Serial.println(" hPa");  
      memset( INSERT_SQL, 0, sizeof(INSERT_SQL) );
      char stringone[] = "INSERT INTO temps.pressdat VALUES (NOW(), 'NodeMCU', ";
      char stringtwo[] = ")";      
      float pressure;
      bmp.getPressure(&pressure);
      dtostrf(pressure,7, 3, outstr);
      strcat(INSERT_SQL, stringone);
      strcat(INSERT_SQL, outstr);
      strcat(INSERT_SQL, stringtwo);
      
      logLine(INSERT_SQL);
      
      /* First we get the current temperature from the BMP085 */
      float temperature;
      bmp.getTemperature(&temperature);
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" C");
      memset( INSERT_SQL, 0, sizeof(INSERT_SQL) );
      memset( stringone, 0, sizeof(stringone));
      strcpy(stringone, "INSERT INTO temps.tempdat VALUES (NOW(), 'NodeMCU', ");
      dtostrf(temperature,7, 3, outstr);
      strcat(INSERT_SQL, stringone);
      strcat(INSERT_SQL, outstr);
      strcat(INSERT_SQL, stringtwo);
  
      logLine(INSERT_SQL);
    

    }
    else
    {
        Serial.println("Sensor error");
    }


         
    adc0.setMultiplexer(ADS1115_MUX_P0_NG);
    adc0.triggerConversion();
    pollAlertReadyPin();
    RainVal = adc0.getMilliVolts(false); 
    
    Serial.print("Rain Reading: ");
    Serial.print(RainVal);
    Serial.print("\n");  
    
    memset( INSERT_SQL, 0, sizeof(INSERT_SQL) );
    memset( stringone, 0, sizeof(stringone));    
    strcpy(stringone, "INSERT INTO temps.raindat VALUES (NOW(), 'NodeMCU', 'Rain-1', ");    
    dtostrf(RainVal,7, 3, outstr);
    strcat(INSERT_SQL, stringone);
    strcat(INSERT_SQL, outstr);
    strcat(INSERT_SQL, stringtwo);

    logLine(INSERT_SQL);

    
    // Activity blink
    digitalWrite(5, HIGH);
    digitalWrite(4, LOW);
    delay(loopdelay);
    digitalWrite(4, HIGH);
    digitalWrite(5, LOW);
    delay(loopdelay);
    
   
   }


  
  
  if(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, wifi_password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("");
      Serial.println("WiFi connected");
      // Print the IP address
      Serial.println(WiFi.localIP());

  }
   
}


/** Poll the assigned pin for conversion status 
 */
void pollAlertReadyPin() {
  for (uint32_t i = 0; i<100000; i++)
    if (!digitalRead(alertReadyPin)) return;
   Serial.println("Failed to wait for AlertReadyPin, it's stuck high!");
}


void logLine(char line[]){
    if(loggingenabled){
      if (my_conn.connect(server_addr, 3306, user, db_password))
      {   
         //Serial.println(line);
         MySQL_Cursor *cur_mem = new MySQL_Cursor(&my_conn);     
         cur_mem->execute(line);
         delete cur_mem;
         Serial.println("Query Success!"); 
      } 
      else
      {
        Serial.println("Connection failed.");
      }
    }
}




