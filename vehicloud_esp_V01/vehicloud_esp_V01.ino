/*
 * Vehicloud project on ESP32 by Baptiste Lerat 16/10/2021
 */


//Arduino Libraries
#include <Arduino.h>
#include <Wire.h> 

//ESP32 Libraries
#include <dummy.h>
//BLE
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

//Sensors Libraries
#include <DHT.h>
#include <MutichannelGasSensor.h>

//GPS
#include <Adafruit_GPS.h>
#include <Adafruit_PMTK.h>
#include <NMEA_data.h>

//My Classes 
#include "MyData.hpp"


/*
 * PIN DEFINITION
 */
#define DHTTYPE DHT11 //Our sensor is the DHT11 
#define DHTPIN 2 //Our DHT sensor is connected on PIN 2

/*
 * Global Variables
 */
DHT DHT_sensor;
MyData sampled_data;

void setup() {
  
  Serial.begin(115200);//Init UART
  Wire.begin(); //Init I2C

  DHT_sensor.begin();//Init DHT

}

void loop() {
  /*
   * Definition des variables
   */

   float temp_hum_val[2]={0};

   

}
