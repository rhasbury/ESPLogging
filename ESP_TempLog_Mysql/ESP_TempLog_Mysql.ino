#include <Adafruit_BMP280.h>
//#include <Adafruit_Sensor.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
//#include <APDS9930.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <string.h>
#include <EEPROM.h>
char sID[7];

#define LOOPDELAY 120000
int loopcount = 0;

// disable sql logging
const int loggingenabled = 1;
int loopdelay = LOOPDELAY;
const int buttonPin = 16;
int i = 0;

Adafruit_BMP280 bme; // I2C

// WiFi parameters
const char* ssid = "";
const char* wifi_password = "";
// Address of mysql server
IPAddress server_addr(192, 168, 1, 104);

/* Setup for the Connector/Arduino */
WiFiClient client;
MySQL_Connection my_conn((Client *)&client);

// MySql credentials
char user[] = "monitor";
char db_password[] = "password";

char outstr[15];
char INSERT_SQL[100] = {0};
float tempVal;


void setup() {

    // Start Serial
    Serial.begin(115200);
    EEPROM.begin(6);
    for (int i=0; i<6; i++) {
      sID[i] = EEPROM.read(i);
    }
    pinMode(buttonPin, INPUT);
    WiFi.mode(WIFI_STA);
    Wire.begin(2,0);    
    // Connect to BMP180
    if (!bme.begin()) {
                      Serial.println("Could not find a valid BMP280 sensor, check wiring!");
                      }
  
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



void loop() {
  
  while(WiFi.status() == WL_CONNECTED){
    uint16_t ch0 = 0;
    uint16_t ch1 = 1;

    char outstr[15];
    char INSERT_SQL[100] = {0};
    float tempVal;
    
    if(digitalRead(buttonPin) == LOW)
    {
      Serial.print("button has been pressed");
      i = 120;
      loopdelay = 60000;
    }
   
    delay(2000);
    tempVal = bme.readTemperature();
    // Log to mysql database
    delay(100);   
    dtostrf(tempVal,7, 3, outstr);
    char stringone[] = "INSERT INTO temps.tempdat2 VALUES (NOW(), \"";
    char stringtwo[] = "\", ";      
    char stringthree[] = ")";      
    strcat(INSERT_SQL, stringone);
    strcat(INSERT_SQL, sID);      // sql name is first 6 digits loaded in device eeprom
    strcat(INSERT_SQL, stringtwo);
    strcat(INSERT_SQL, outstr);
    strcat(INSERT_SQL, stringthree);

    Serial.print(INSERT_SQL);
    logLine(INSERT_SQL);
      
    // Restarting every n loops to deal with esp locking up after extended runtimes. 
    loopcount++;
    if(loopcount > 5) { 
      Serial.println("Restarting");
      ESP.restart(); 
    };
    
    // Activity blink
    digitalWrite(5, HIGH);
    digitalWrite(4, LOW);
    delay(loopdelay);
    digitalWrite(4, HIGH);
    digitalWrite(5, LOW);
    delay(loopdelay);
    
    if(i>0){
      i--;      
      }
     else{
      loopdelay = LOOPDELAY;
     }
   
   }


  
  
  if(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, wifi_password);
      
      while (WiFi.status() != WL_CONNECTED) {
        delay(2000);
        Serial.println("Trying to connect to wifi");
        //Serial.println(bme.readTemperature());
      }
      Serial.println("");
      Serial.println("WiFi connected");
      // Print the IP address
      Serial.println(WiFi.localIP());

  }
  //Serial.print(bme.readTemperature());






}
