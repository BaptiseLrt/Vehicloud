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
#define DHTPIN 14 //Our DHT sensor is connected on pin D5
#define BUTTON_PIN 12 //Our button is connected on ping D6

#define MAX_DATA 200

//BLE Config 
#define SERVICE_UUID "32a9184d-8ff4-4d7b-8c8d-619dd8cac8c8"
#define CHAR_GPS_UUID "26567f76-b97e-4664-b9d4-a75ec7129bd7"
#define CHAR_TIME_UUID "cd4cbd1b-95d6-4295-b0d8-8403145844c2"
#define CHAR_TEMP_UUID "cd4cbd1b-95d6-4295-b0d8-8403145844c3"

#define GPSECHO true
/*  
 * Global Variables
 */
DHT DHT_sensor(DHTPIN, DHTTYPE);
SoftwareSerial mySerial(23,5);
Adafruit_GPS GPS(&mySerial);

//BLE Classes
BLEServer* pServer = NULL;
BLECharacteristic* CharGPS = NULL;
BLECharacteristic* CharTime = NULL;
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
  
   Serial.begin(115200);//Init UART
   //DHT_sensor.begin();//Init DHT
   Serial.println("Temperature sensor initialized");

   delay(5000);


  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
   // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);

  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time

  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE);

  //Initializing gas sensor
  MiCS6814.begin();
  MiCS6814.calibrate();

  //Initializing BLE
    // Create the BLE Device
  BLEDevice::init("ESP32");

  Serial.println("Initializing BLE");
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(&MyCallbacks);

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  CharGPS = pService->createCharacteristic(CHAR_GPS_UUID, BLECharacteristic::PROPERTY_READ|BLECharacteristic::PROPERTY_WRITE|BLECharacteristic::PROPERTY_NOTIFY|BLECharacteristic::PROPERTY_INDICATE);
  CharTime = pService->createCharacteristic(CHAR_TIME_UUID, BLECharacteristic::PROPERTY_READ|BLECharacteristic::PROPERTY_WRITE|BLECharacteristic::PROPERTY_NOTIFY|BLECharacteristic::PROPERTY_INDICATE);

    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  CharGPS->addDescriptor(new BLE2902());
  CharTime->addDescriptor(new BLE2902());
  
  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {

 if(!MyCallbacks.deviceIsConnected()){
  float temp_hum_val[2]={0};
  
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

  //if (button_state==1){
    // if a sentence is received, we can check the checksum, parse it...
    int sample_time =0;
    char c = GPS.read();
    // if you want to debug, this is a good time to do it!
    
    if ((c) && (GPSECHO))
    Serial.write(c);
    if (GPS.newNMEAreceived()) {
     
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
    }
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
    
      if (millis() - timer > 5000) {
        float distance=acos(sin(radians(latitude_values[sample_number]))*sin(radians(GPS.latitudeDegrees))+cos(radians(latitude_values[sample_number]))*cos(radians(GPS.latitudeDegrees))*cos(longitude_values[sample_number]-GPS.longitudeDegrees))*6371;
         
        timer = millis(); // reset the timer
        Serial.print("Latitude:");Serial.println(GPS.latitudeDegrees);
        Serial.print("Longitude:"); Serial.println(GPS.longitudeDegrees);
        Serial.print("Hour:"); Serial.println(GPS.hour);
        Serial.print("Minute:"); Serial.println(GPS.minute);
        latitude_values[sample_number]=GPS.latitudeDegrees;
        longitude_values[sample_number]=GPS.longitudeDegrees;
        hour_values[sample_number]=GPS.hour;
        minute_values[sample_number]=GPS.minute;
        
        gas_values1[sample_number]=MiCS6814.get(CO);
        Serial.print("gas_values: CO");Serial.println(gas_values1[sample_number]);
        gas_values2[sample_number]=MiCS6814.get(NO2);
        Serial.print("gas_values: NO2");Serial.println(gas_values2[sample_number]);
        gas_values3[sample_number]=MiCS6814.get(NH3);
        Serial.print("gas_values: NH3");Serial.println(gas_values3[sample_number]);
        //gaz_value.get_moyenne(); gaz_value.initialize();
        //temperature_values[sample_number]=
        //temp_value.get_moyenne(); temp_value.initialize();
        //humidity_values[sample_number]=
        //hum_value.get_moyenne();hum_value.initialize();
        sample_number++;
      }
    //}
 }

  //if (button_state==2){ //Double tap
  // Bluetooh
  if (MyCallbacks.deviceIsConnected()&&sample_number>0){
    Serial.println("device connected, sending data");
    for (int i=0; i<sample_number;i++){
      Serial.print("Latitude send :");Serial.println(latitude_values[i]);
      uint16_t Data_To_Send[10]={(uint16_t)((latitude_values[i]*100)), (uint16_t)(longitude_values[i]*100), (uint16_t)(hour_values[i]),(uint16_t)(minute_values[i]),(uint16_t)(gas_values1[i]),(uint16_t)(gas_values2[i]),(uint16_t)(gas_values3[i]),270,280,290};
      uint8_t Data_BLE[20];
      tab_to_send(Data_To_Send, Data_BLE);
      Serial.print("Sending sample number: ");Serial.println(i);
      CharTime->setValue(Data_BLE, sizeof(Data_BLE));
      CharTime->indicate();
      delay(3);
    }
    uint8_t Data_BLE[20]={0,255,0,255,0,255,0,255,0,255,0,255,0,255,0,255,0,255,0,255};
    CharTime->setValue(Data_BLE, sizeof(Data_BLE));
    CharTime->indicate();
    sample_number=0;
  }
  if (!MyCallbacks.deviceIsConnected()&& oldDeviceConnected){
    delay(500); //give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();
    Serial.println("Restart advertising");
    oldDeviceConnected=MyCallbacks.deviceIsConnected();
  }
  if (MyCallbacks.deviceIsConnected() && !oldDeviceConnected){
    //Reconnection
    oldDeviceConnected = MyCallbacks.deviceIsConnected();
  }    
}
