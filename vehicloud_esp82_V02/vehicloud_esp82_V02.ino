/*
 * Vehicloud project on ESP82 by Baptiste Lerat 02/10/2021
 */

//Arduino Libraries
#include <Arduino.h>
#include <Wire.h> 
#include <SoftwareSerial.h>

//test flo_dev_branch

//ESP82 Libraries
#include <dummy.h>

//Wifi & HTTP
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>

//C++ Libs
#include <math.h>

//Sensors Libraries
#include <DHT.h>

//GPS
#include <Adafruit_GPS.h>
#include <Adafruit_PMTK.h>

#include "config.h"
//My Classes 
#include "calcul_moyenne.h"

#define DHTTYPE DHT11 //Our sensor is the DHT11 
#define DHTPIN 14 //Our DHT sensor is connected on pin D5
#define BUTTON_PIN 12 //Our button is connected on ping D6

#define MAX_DATA 200
/*
 * Global Variables
 */
SoftwareSerial mySerial(13, 15);
Adafruit_GPS GPS(&mySerial);

DHT DHT_sensor(DHTPIN, DHTTYPE);
WiFiClient my_wificlient;

float temperature_values[MAX_DATA];
float humidity_values[MAX_DATA];
float longitude_values[MAX_DATA];
float latitude_values[MAX_DATA];
float gas_values[MAX_DATA];
float hour_values[MAX_DATA];
float minute_values[MAX_DATA];

CalculMoyenne gaz_value;
CalculMoyenne temp_value;
CalculMoyenne hum_value;
         
int sample_number=0;
int button_state=0;
 
void setup() {
  
   Serial.begin(115200);//Init UART
   Serial.println("bordel de merde");
   DHT_sensor.begin();//Init DHT
   Serial.println("Temperature sensor initialized");
   // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
    GPS.begin(9600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
;
  
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  
  // uncomment this line to turn on only the "minimum recommended" data
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  Serial.println("GPS Initialized");
}

void loop() {
  int httpResponseCode;
  //float temp_hum_val[2]={0};
  
    if (digitalRead(BUTTON_PIN)==LOW){
      button_state++;
      delay(200);
    }

  if (button_state==1){
    // if a sentence is received, we can check the checksum, parse it...
    if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
    }
    //Calculate distance traveled
    float distance=acos(sin(radians(latitude_values[sample_number]))*sin(radians(GPS.latitudeDegrees))+cos(radians(latitude_values[sample_number]))*cos(radians(GPS.latitudeDegrees))*cos(longitude_values[sample_number]-GPS.longitudeDegrees))*6371;
    if (distance<=0.1){//We take the mean of the values during 100 meters
        int sensorValue = analogRead(A0);
        gaz_value.UpdateMoyenne((float)sensorValue/1024);
      if (!DHT_sensor.readHumidity()){
        hum_value.UpdateMoyenne(DHT_sensor.readHumidity());
      }
      if (!DHT_sensor.readTemperature()){
        temp_value.UpdateMoyenne(DHT_sensor.readTemperature());
      }
    }
    else if(distance>0.1){ //After 100 meter, we store data (we could/should store it in a flash/eeprom but it is not necessary for our prototype)
       sample_number++;
       latitude_values[sample_number]=GPS.latitudeDegrees;
       longitude_values[sample_number]=GPS.longitudeDegrees;
       gas_values[sample_number]=gaz_value.get_moyenne(); gaz_value.initialize();
       temperature_values[sample_number]=temp_value.get_moyenne(); temp_value.initialize();
       humidity_values[sample_number]=hum_value.get_moyenne();hum_value.initialize();
       hour_values[sample_number]=GPS.hour;
       minute_values[sample_number]=GPS.minute;   
    }
  }

  if (button_state==2){
  // Connexion to WIFI
  WiFi.begin(ssid, wifipass);
  Serial.println("Connecting");
    
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  
     //ALWAYS check connexion status
   if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;

    for (int i=0; i<sample_number; i++){
    // Your Domain name with URL path or IP address with path
    http.begin(my_wificlient, serverName+"ESP-1"+"/DATA");
  
    // If you need an HTTP request with a content type: application/json, use the following: 
    http.addHeader("X-M2M-Origin", "admin:admin");
    http.addHeader("Content-Type", "application/json;ty=4"); //Create a contentInstance

    // Create the Request body and send it with information of Temperature, Gas, humidity, localisation and time 
    httpResponseCode = http.POST("{ \"m2m:cin\" : { \"con\" : \"<obj> <str name='lat' val='"+String(latitude_values[i])+"'/><str name='long' val='"+String(longitude_values[i])+"'/><str name='gas' val='"+String(gas_values[i])+"'/> <str name='temp' val='"+String(temperature_values[i])+"'/><str name='hum' val='"+String(humidity_values[i])+"'/><str name='hour' val='"+String(hour_values[i])+"'/><str name='minute' val='"+String(minute_values[i])+"'/></obj>\" } }");
    Serial.print("HTTP Sending :");
    Serial.println("{ \"m2m:cin\" : { \"con\" : \"<obj> <str name='GAZ' val='"+String(DHT_sensor.readTemperature())+"'/> </obj>\" } }");
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    if(httpResponseCode ==-1){  Serial.print("SERVER UNREACHABLE  !!");}
    // Free resources
    http.end(); 
    }
    button_state=0;
  }
  else {
       Serial.println("WiFi Disconnected");
       button_state=0;
  }
  }
}
