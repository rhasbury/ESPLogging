#include "Adafruit_CCS811.h"
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <ESP8266WiFi.h>
#include <string.h>
#include <EEPROM.h>


#define LOOPDELAY 5000; //120000
int loopcount = 0;
int connectiontries = 0;

String freeHeap;

Adafruit_CCS811 ccs;

// disable sql logging
const int loggingenabled = 1;
int loopdelay = LOOPDELAY;
int i = 0;

struct { 
    char sID[7] = "";
    char ssid[50] = "";
    char wifi_password[50] = "";
  } StoredData;
int addr = 0;

//char sID[300] = "";

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
    pinMode(D5, OUTPUT);
    digitalWrite(D5, LOW);
    EEPROM.begin(512);
    EEPROM.get(addr,StoredData);  
    
    WiFi.mode(WIFI_STA);
    delay(500);
    // Connect to CSS811
    if(!ccs.begin()){
       Serial.println("Failed to start sensor! Please check your wiring.");
       while(1);
    }

    // Wait for the sensor to be ready
    Serial.println("Waiting for available css811 sensor.");
    while(!ccs.available());
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
    int co2val;
    int tvocval;
  
    delay(2000);
    if(ccs.available()){
      if(!ccs.readData()){        
        co2val = ccs.geteCO2();        
        tvocval = ccs.getTVOC();
      }
    else{
      Serial.println("ERROR!");
      while(1);
      }
    }


    // Log to mysql database
    delay(100);   
    dtostrf(co2val,7, 3, outstr);
    
    char stringone[] = "INSERT INTO temps.gasdat2 VALUES (NOW(), \"";
    char stringtwo[] = "\", \"co2\", ";      
    char stringthree[] = ")";      
    strcat(INSERT_SQL, stringone);
    strcat(INSERT_SQL, StoredData.sID);      // sql name is first 6 digits loaded in device eeprom
    strcat(INSERT_SQL, stringtwo);
    strcat(INSERT_SQL, outstr);
    strcat(INSERT_SQL, stringthree);

    Serial.println(INSERT_SQL);
    logLine(INSERT_SQL);

    // Log to mysql database
    delay(100);   
    dtostrf(tvocval,7, 3, outstr);
    INSERT_SQL[0] = 0;
    //stringone[] = "INSERT INTO temps.gasdat2 VALUES (NOW(), \"";
    char stringfour[] = "\", \"TVOC\", ";           
    strcat(INSERT_SQL, stringone);
    strcat(INSERT_SQL, StoredData.sID);      // sql name is first 6 digits loaded in device eeprom
    strcat(INSERT_SQL, stringfour);
    strcat(INSERT_SQL, outstr);
    strcat(INSERT_SQL, stringthree);

    Serial.println(INSERT_SQL);
    logLine(INSERT_SQL);
    Serial.println(ESP.getFreeHeap());
      
    

    delay(loopdelay);
    
    if(i>0){
      i--;      
      }
     else{
      loopdelay = LOOPDELAY;
     }
    // Restarting every n loops to deal with esp locking up after extended runtimes. 
    loopcount++;
    if(loopcount > 5) { 
      Serial.println("Wifi disconnecting");
      WiFi.disconnect();
      delay(1000);
      Serial.println("Restarting");
      ESP.restart(); 
    };   
   }


  
  
  if(WiFi.status() != WL_CONNECTED){
      WiFi.begin(StoredData.ssid, StoredData.wifi_password);
      
      while (WiFi.status() != WL_CONNECTED) {
        delay(2000);
        Serial.println("Trying to connect to wifi");
        Serial.println("SSID is: "+String(StoredData.ssid));
        Serial.printf("Connection status: %d\n", WiFi.status());
        if(connectiontries > 5) { 
          Serial.println("Fit Wifi disconnecting");
          WiFi.disconnect();
          delay(1000);
          Serial.println("Restarting");
          ESP.restart(); 
        }; 
      connectiontries++;
      }
      Serial.println("");
      Serial.println("WiFi connected");
      // Print the IP address
      Serial.println(WiFi.localIP());

  }
  //Serial.print(bme.readTemperature());






}
