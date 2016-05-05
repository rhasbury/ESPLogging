<<<<<<< .mine
#include <Adafruit_BMP085_U.h>
#include <Adafruit_Sensor.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <APDS9930.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <string.h>


// WiFi parameters
//const char* ssid = "ssid";
//const char* wifi_password = "password";
// Address of mysql server
//IPAddress server_addr(192, 168, 1, 104);





/* Setup for the Connector/Arduino */
WiFiClient client;
MySQL_Connection my_conn((Client *)&client);

// MySql credentials
char user[] = "monitor";
char db_password[] = "password";


Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

APDS9930 apds = APDS9930();

void setup() {
  // put your setup code here, to run once:
  // Start Serial
  Serial.begin(115200);
  pinMode(A0, INPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(D0, OUTPUT);

  // Connect to BMP180
  if (!bmp.begin(BMP085_MODE_STANDARD)) {
                    Serial.println("Could not find a valid BMP085 sensor, check wiring!");                    
    }

  // Connect to WiFi
  //WiFi.begin(ssid, wifi_password);
  //while (WiFi.status() != WL_CONNECTED) {
  //  delay(500);
  //  Serial.print(".");
  //}
  //Serial.println("");
  //Serial.println("WiFi connected");
  
  // Print the IP address
  //Serial.println(WiFi.localIP());

  // Initialize APDS-9930 (configure I2C and initial values)
  if ( apds.init() ) { Serial.println("APDS-9930 initialization complete");  } 
  else { Serial.println("Something went wrong during APDS-9930 init!"); }
  
  // Start running the APDS-9930 light sensor (no interrupts)
  if ( apds.enableLightSensor(false) ) {
    Serial.println("Light sensor is now running");
  } else {
    Serial.println("Something went wrong during light sensor init!");
  }
}

void loop() {
  
  while(WiFi.status() == WL_CONNECTED){
    uint16_t ch0 = 0;
    uint16_t ch1 = 1;
    float ambient_light = 0;
    char outstr[15];
    char INSERT_SQL[100] = {0};
    
    if (  !apds.readAmbientLightLux(ambient_light) ||
          !apds.readCh0Light(ch0) || 
          !apds.readCh1Light(ch1) ) {
            Serial.println("Error reading light values");
    } else {
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
    
    
    
    Serial.print("Gas Reading: ");
    Serial.print(analogRead(A0));
    Serial.print("\n");
    
    memset( INSERT_SQL, 0, sizeof(INSERT_SQL) );
    char stringone[] = "INSERT INTO temps.gasdat VALUES (NOW(), 'NodeMCU', 'MQ-7', ";
    char stringtwo[] = ")";      
    strcat(INSERT_SQL, stringone);
    strcat(INSERT_SQL, itoa(analogRead(A0),outstr,10));
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
      //memset( stringone, 0, sizeof(stringone));
      strcpy(stringone, "INSERT INTO temps.tempdat VALUES (NOW(), 'NodeMCU', ");
      //char stringone[] = "INSERT INTO temps.tempdat VALUES (NOW(), 'NodeMCU', ";
      //char stringtwo[] = ")";      
      dtostrf(temperature,7, 3, outstr);
      strcat(INSERT_SQL, stringone);
      strcat(INSERT_SQL, outstr);
      strcat(INSERT_SQL, stringtwo);
      logLine(INSERT_SQL);
  
      /* Then convert the atmospheric pressure, and SLP to altitude         */
      /* Update this next line with the current SLP for better results      */
//      float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
//      Serial.print("Altitude:    "); 
//      Serial.print(bmp.pressureToAltitude(seaLevelPressure,
//                                          event.pressure)); 
//      Serial.println(" m");
//      Serial.println("");
    }
    else
    {
      Serial.println("Sensor error");
    }

    // Activity blink
    digitalWrite(5, HIGH);
    digitalWrite(4, LOW);
    delay(500);
    digitalWrite(4, HIGH);
    digitalWrite(5, LOW);
    delay(150000);
   
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

||||||| .r3

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
     delay(300000);
     MySQL_Cursor *cur_mem = new MySQL_Cursor(&my_conn);     
     cur_mem->execute(INSERT_SQL);
     delete cur_mem;
     Serial.println("Query Success!"); 
  } 
  else
  {
    Serial.println("Connection failed.");
  }

=======

#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <APDS9930.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <string.h>


// WiFi parameters
const char* ssid = "wifi2";
const char* wifi_password = "connectome";

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
>>>>>>> .r5
<<<<<<< .mine

void logLine(char line[]){

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


//
//void displaySensorDetails(void)
//{
//  sensor_t sensor;
//  bmp.getSensor(&sensor);
//  Serial.println("------------------------------------");
//  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
//  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
//  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
//  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" hPa");
//  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" hPa");
//  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" hPa");  
//  Serial.println("------------------------------------");
//  Serial.println("");
//  delay(500);
//}
||||||| .r3
}
=======
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
     delay(300000);
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
>>>>>>> .r5
