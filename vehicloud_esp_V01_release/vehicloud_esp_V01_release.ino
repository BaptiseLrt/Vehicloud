/*
 * Vehicloud project on ESP82 by Baptiste Lerat 06/12/2021
 */
//Arduino Libraries
#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>

//
////C++ Libs
//#include <math.h>
//
//Sensors Libraries
#include <DHT.h>

//BLE Libraries
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

//GPS
#include <Adafruit_GPS.h>
#include <Adafruit_PMTK.h>

//Gas Sensor
#include "Grove_MultichannelGasSensor.h"

////My Classes 
#include "calcul_moyenne.h"
#include "MyBLECallback.h"

//Defines
#define DHTTYPE DHT11 //Our sensor is the DHT11 
#define DHTPIN 4 //Our DHT sensor is connected on pin D5
#define BUTTON_PIN 12 //Our button is connected on ping D6

#define MAX_DATA 200

//BLE Config 
#define SERVICE_UUID "32a9184d-8ff4-4d7b-8c8d-619dd8cac8c8"
#define CHAR_DATA_UUID "cd4cbd1b-95d6-4295-b0d8-8403145844c2"


#define GPSECHO true
/*  
 * Global Variables
 */
DHT DHT_sensor(DHTPIN, DHTTYPE, 8000000L);
SoftwareSerial mySerial(23,5);
Adafruit_GPS GPS(&mySerial);

//BLE Classes
BLEServer* pServer = NULL;
BLECharacteristic* CharGPS = NULL;
BLECharacteristic* CharData = NULL;
MyBLEServerCallbacks MyCallbacks;
bool oldDeviceConnected = false;

float temperature_values[MAX_DATA];
float humidity_values[MAX_DATA];
float longitude_values[MAX_DATA];
float latitude_values[MAX_DATA];
float gas_values1[MAX_DATA];
float gas_values2[MAX_DATA];
float gas_values3[MAX_DATA];
float hour_values[MAX_DATA];
float minute_values[MAX_DATA];

CalculMoyenne gaz_value;
CalculMoyenne temp_value;
CalculMoyenne hum_value;
         
int sample_number=0;
int timer=0;
int button_state=0;


void tab_to_send(uint16_t * tab_input, uint8_t * tab_output, int size_in=10){

  for(int i=0; i<size_in; i++){
    tab_output[(i*2)+1]=(uint8_t)(tab_input[i]);
    tab_output[(i*2)]=(uint8_t)(tab_input[i]>>8);
  } 
}
 
void setup() {
  DHT_sensor.begin();//Init DHT


  //Initializing gas sensor
  MiCS6814.begin();
  MiCS6814.calibrate();


   delay(5000);
  
   // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);

  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time

  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_100_MILLIHERTZ);   // Update every 10 seconds to be sure we get at least 1 update in 20 seconds
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE); 

  //Initializing BLE
    // Create the BLE Device
  BLEDevice::init("Vehicloud_V01");

  //Serial.println("Initializing BLE");
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(&MyCallbacks);

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  CharData = pService->createCharacteristic(CHAR_DATA_UUID, BLECharacteristic::PROPERTY_READ|BLECharacteristic::PROPERTY_WRITE|BLECharacteristic::PROPERTY_NOTIFY|BLECharacteristic::PROPERTY_INDICATE);

    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  CharData->addDescriptor(new BLE2902());
  
  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  //Serial.println("Waiting a client connection to notify...");
  //BLEDevice::startAdvertising();

}

void loop() {
  

  //  if (button_state==0){
  //    if (digitalRead(BUTTON_PIN)==0){
  //      button_state++;
  //      int previous_millis=millis();
  //      delay(50);//Avoid bouncing
  //      while(millis()-previous_millis<500){
  //        if(digitalRead(BUTTON_PIN)==0){
  //          button_state++;
  //          delay(500);
  //        }
  //      }
  //    }
  //  }
  

 if(!MyCallbacks.deviceIsConnected()){
  float temp_hum_val[2]={0};
  
  //if (button_state==1){
    // if a sentence is received, we can check the checksum, parse it...
    int sample_time =0;
    char c = GPS.read();

        //Calculate distance traveled
   
//    if (distance<=0.1){//We take the mean of the values during 100 meters
//        int sensorValue = analogRead(A0);
//        gaz_value.UpdateMoyenne((float)sensorValue/1024);
//      if (!DHT_sensor.readTempAndHumidity(temp_hum_val)){
//        temp_value.UpdateMoyenne(temp_hum_val[0]);
//        hum_value.UpdateMoyenne(temp_hum_val[1]);
//      }
//    }
    //else if(distance>0.1||(millis()-sample_time>=1000)){ //After 100 meter, we store data (we could/should store it in a flash/eeprom but it is not necessary for our prototype)


    
    // Usefull to debug
    
    if ((c) && (GPSECHO));
    //Serial.write(c);
    if (GPS.newNMEAreceived()) {
     
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
    }
    
      if (millis() - timer > 20000) { //We take values every 20 seconds
        
       // float distance=acos(sin(radians(latitude_values[sample_number]))*sin(radians(GPS.latitudeDegrees))+cos(radians(latitude_values[sample_number]))*cos(radians(GPS.latitudeDegrees))*cos(longitude_values[sample_number]-GPS.longitudeDegrees))*6371;
        if (GPS.fix){
        timer = millis(); // reset the timer
        latitude_values[sample_number]=(GPS.latitudeDegrees);
        //latitude_values[sample_number]+=(GPS.latitude/100.0f);
        longitude_values[sample_number]=(GPS.longitudeDegrees);
        hour_values[sample_number]=GPS.hour;
        minute_values[sample_number]=GPS.minute;

        gas_values1[sample_number]=MiCS6814.get(CO);
        gas_values2[sample_number]=MiCS6814.get(NO2);
        gas_values3[sample_number]=MiCS6814.get(NH3);
         if (!DHT_sensor.readTempAndHumidity(temp_hum_val)) {
          temperature_values[sample_number]=temp_hum_val[0];
          humidity_values[sample_number]=temp_hum_val[1];
        }
        float distance=acos(sin(radians(latitude_values[sample_number-1]))*sin(radians(latitude_values[sample_number]))+cos(radians(latitude_values[sample_number-1]))*cos(radians(latitude_values[sample_number]))*cos(longitude_values[sample_number-1]-longitude_values[sample_number]))*63710;
        if (distance < 5){
          BLEDevice::startAdvertising();
        }
        else {
          BLEDevice::stopAdvertising();
        }
        if (sample_number<200){
          sample_number++;
        }
        else {
          sample_number=100;  
        }
        }
      }
    //}
 }

  //if (button_state==2){ //Double tap
  // Bluetooh
  if (MyCallbacks.deviceIsConnected()&&sample_number>0){
    for (int i=0; i<sample_number;i++){
      uint16_t Data_To_Send[10]={(uint16_t)(latitude_values[i]*1000), (uint16_t)(longitude_values[i]*1000), (uint16_t)(hour_values[i]),(uint16_t)(minute_values[i]),(uint16_t)(temperature_values[i]),(uint16_t)(humidity_values[i]),(uint16_t)(gas_values1[i]*100),(uint16_t)(gas_values2[i]*100),(uint16_t)(gas_values3[i]*100), (uint16_t)(101)};
      uint8_t Data_BLE[20];
      tab_to_send(Data_To_Send, Data_BLE);
      //Serial.print("Sending sample number: ");Serial.println(i);
      CharData->setValue(Data_BLE, sizeof(Data_BLE));
      CharData->indicate();
      delay(10);
    }
    uint8_t Data_BLE[20]={0,255,0,255,0,255,0,255,0,255,0,255,0,255,0,255,0,255,0,255};
    CharData->setValue(Data_BLE, sizeof(Data_BLE));
    CharData->indicate();
    sample_number=0;
  }
  if (!MyCallbacks.deviceIsConnected()&& oldDeviceConnected){
    delay(500); //give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();
    //Serial.println("Restart advertising");
    oldDeviceConnected=MyCallbacks.deviceIsConnected();
  }
  if (MyCallbacks.deviceIsConnected() && !oldDeviceConnected){
    //Reconnection
    oldDeviceConnected = MyCallbacks.deviceIsConnected();
  }    
}
