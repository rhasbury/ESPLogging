//#include <Adafruit_ADS1015.h>

#include <Adafruit_BMP085_U.h>
#include <Adafruit_Sensor.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <string.h>
#include <Adafruit_ADS1015.h>
#include <EEPROM.h>
 
//#include "ADS1115.h"

//ADS1115 adc0(0x49);
Adafruit_ADS1115 adc0(0x49);


// Wire ADS1115 ALERT/RDY pin to Arduino pin 2
const int alertReadyPin = 13;

// constants for Sharp GP2Y1010AU0F Particle Sensor
const int ledPower = 12;
const int delayTime=280;
const int dustPin=A0;
const int delayTime2=40;


// disable sql logging
const int loggingenabled = 1;

int loopdelay = 120000;
int sleepdelay = 60e6;
int dustSensorCoefficent = 32000;


// Address of mysql server
IPAddress server_addr(192, 168, 1, 104);


/* Setup for the Connector/Arduino */
WiFiClient client;
MySQL_Connection my_conn((Client *)&client);

// MySql credentials
char user[] = "monitor";
char db_password[] = "password";


Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
//APDS9930 apds = APDS9930();


// Struct and address for wifi info in eeprom
struct { 
    char sID[7] = "";
    char ssid[50] = "";
    char wifi_password[50] = "";
  } StoredData;
int addr = 0;


void setup() {

    // Start Serial
    Serial.begin(115200);
    // Get wifi information from eeprom
    EEPROM.begin(512);
    delay(200);
    EEPROM.get(addr,StoredData);
    delay(200);
    Serial.println("SSID is: "+String(StoredData.ssid) + " " + String(StoredData.wifi_password));
    
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

    // Setup for the ADS1115 ADC
    //Wire.begin(2, 14);    
    Wire.begin();        
    adc0.setGain(GAIN_ONE);
    adc0.begin(); // initialize ADS1115 16 bit A/D chip

    
  
}

void loop() {

  // temporarily outside the connected loop for testing
//  Get_GyDust_Reading();
//  Get_BMP_Reading();
//  Get_Rain_Reading();
//  Get_MQ7_Reading();
  
  while(WiFi.status() == WL_CONNECTED){

    Get_GyDust_Reading();
    Get_BMP_Reading();
    Get_Rain_Reading();
    Get_MQ7_Reading();

    WiFi.disconnect();

    //Serial.println("Done logging, wifi disconnect");                  
    //WiFi.disconnect();
    //ESP.deepSleep(sleepdelay); // Deep sleep mode, wiring must be installed before enabling. 
    delay(loopdelay);
    Serial.println("Resetting");                  
    ESP.restart(); 
  }

    
  if(WiFi.status() != WL_CONNECTED){
      WiFi.begin(StoredData.ssid, StoredData.wifi_password);
      int connectiontries = 0; 
      while (WiFi.status() != WL_CONNECTED) {
        delay(2000);
        Serial.println("Trying to connect to wifi");
        Serial.println("SSID is: "+String(StoredData.ssid));
        Serial.printf("Connection status: %d\n", WiFi.status());
        if(connectiontries > 2) { 
          Serial.println("F'it Wifi disconnecting");
          WiFi.disconnect();
          delay(1000);
          Serial.println("Connection failed. Restarting");
          //ESP.deepSleep(sleepdelay);
          ESP.restart(); 
          
        }; 
      connectiontries++;
      }
      Serial.println("");
      Serial.println("WiFi connected");
      // Print the IP address
      Serial.println(WiFi.localIP());

  }
  Serial.println("Outside of loop for some reason");
  Serial.println(WiFi.status());
  delay(1000);
  
}


/** Poll the assigned pin for conversion status 
 */
void pollAlertReadyPin() {
  for (uint32_t i = 0; i<100000; i++)
    if (!digitalRead(alertReadyPin)) return;
   Serial.println("Failed to wait for AlertReadyPin, it's stuck high!");
}



void Get_BMP_Reading(){
    //char INSERT_SQL[100] = {0};
    String INSERT_TO_SQL; 
    char outstr[15];
    float pressure;
    float temperature;
    
    sensors_event_t event;
    bmp.getEvent(&event);     
    if (event.pressure)
    {
      //displaySensorDetails();
      /* Display atmospheric pressue in hPa */
      //Serial.print("Pressure:    ");
      //Serial.print(event.pressure);
      //Serial.println(" hPa");  
      
      bmp.getPressure(&pressure);
      dtostrf(pressure,7, 3, outstr);
      
      INSERT_TO_SQL = String("INSERT INTO temps.pressdat VALUES (NOW(), '") + StoredData.sID + String("', ") + String(outstr) + String(")");             
      logLine(INSERT_TO_SQL);
      
            
      bmp.getTemperature(&temperature);
      dtostrf(temperature,7, 3, outstr);

      INSERT_TO_SQL = String("INSERT INTO temps.tempdat2 VALUES (NOW(), '") + StoredData.sID + String("', ") + outstr + String(")");             
      logLine(INSERT_TO_SQL);

    }
    else
    {
        Serial.println("Sensor error");
    }
}


void Get_GyDust_Reading(){
    
    String INSERT_TO_SQL; 
    char outstr[15];

    float dustVal;
    
    Serial.println("about to set LED on: ");  
    digitalWrite(ledPower,HIGH); // power on the LED
    delayMicroseconds(delayTime);
    //delay(2000);
    Serial.println("Led on. Now reading dust ");  
    dustVal = adc0.readADC_SingleEnded(1);
    delayMicroseconds(delayTime2);
    //delay(2000);
    digitalWrite(ledPower,LOW); // turn the LED off
    //delayMicroseconds(offTime);
    delay(2000);
    Serial.println("Dust Reading: ");
    Serial.println(dustVal);
    


    INSERT_TO_SQL = String("INSERT INTO temps.gasdat VALUES (NOW(), '") + StoredData.sID + String("', 'GY-Dust', ") + itoa(dustVal,outstr,10) + String(")");             
    logLine(INSERT_TO_SQL);
    

}



void Get_Rain_Reading(){
    int RainVal;
    String INSERT_TO_SQL; 
    char outstr[15];
    
    RainVal = adc0.readADC_SingleEnded(0);
    
    Serial.print("Rain Reading: ");
    Serial.print(RainVal);
    Serial.print("\n");  

    dtostrf(RainVal,7, 3, outstr);
    INSERT_TO_SQL = String("INSERT INTO temps.raindat VALUES (NOW(), '") + StoredData.sID + String("', 'Rain-1', ") + outstr + String(")");             
    logLine(INSERT_TO_SQL);

}



void Get_MQ7_Reading(){
    int CO;
    String INSERT_TO_SQL; 
    char outstr[15];
    
    CO = adc0.readADC_SingleEnded(2);
    
    Serial.print("MQ-7 Reading: ");
    Serial.print(CO);
    Serial.print("\n");  

    dtostrf(CO,7, 3, outstr);
    INSERT_TO_SQL = String("INSERT INTO temps.gasdat VALUES (NOW(), '") + StoredData.sID + String("', 'MQ-7', ") + outstr + String(")");
    logLine(INSERT_TO_SQL);
}


void logLine(String line){
    char INSERT_SQL[100] = {0};
    line.toCharArray(INSERT_SQL, 100);
    if(loggingenabled){
      Serial.println("Getting ready for logging"); 
      if (my_conn.connect(server_addr, 3306, user, db_password))
      {   
         Serial.println("Logging this line:"); 
         Serial.println(INSERT_SQL);
         Serial.println("Creating cursor:"); 
         MySQL_Cursor *cur_mem = new MySQL_Cursor(&my_conn);     
         Serial.println("Cursor created, executing query"); 
         cur_mem->execute(INSERT_SQL);
         Serial.println("Query executed, delteing cursor"); 
         delete cur_mem;
         Serial.println("Deleted cursor. Closing Connection");          
         my_conn.close();
         Serial.println("Connection closed. Done logline()"); 
      } 
      else
      {
        Serial.println("Connection failed.");
      }
    }
    else
    {
         Serial.println("Logging disabled this is what would have been logged:"); 
         Serial.println(INSERT_SQL);
    }
}
