
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <APDS9930.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <string.h>


// WiFi parameters
const char* ssid = "grhl2";
const char* wifi_password = "$Service";

// Address of mysql server
IPAddress server_addr(192, 168, 0, 103);


/* Setup for the Connector/Arduino */
WiFiClient client;
MySQL_Connection my_conn((Client *)&client);

// MySql credentials
char user[] = "monitor";
char db_password[] = "password";



void setup() {
  // put your setup code here, to run once:
  // Start Serial
  Serial.begin(115200);

  // Connect to WiFi
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

void loop() {
  
  APDS9930 apds = APDS9930();
  uint16_t ch0 = 0;
  uint16_t ch1 = 1;
  float ambient_light = 0;
  char outstr[15];
  char INSERT_SQL[100] = {0};
  
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
    dtostrf(ambient_light,7, 3, outstr);
    char stringone[] = "INSERT INTO temps.lightdat VALUES (NOW(), \"NodeMCU\", ";
    char stringtwo[] = ")";
    
    strcat(INSERT_SQL, stringone);
    strcat(INSERT_SQL, outstr);
    strcat(INSERT_SQL, stringtwo);
  }

  
  if (my_conn.connect(server_addr, 3306, user, db_password))
  {   
     delay(5000);
     MySQL_Cursor *cur_mem = new MySQL_Cursor(&my_conn);     
     cur_mem->execute(INSERT_SQL);
     delete cur_mem;
     Serial.println("Query Success!"); 
  } 
  else
  {
    Serial.println("Connection failed.");
  }

}
