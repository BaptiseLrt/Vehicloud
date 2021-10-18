#include "MyData.hpp"

MyData::MyData(){
  temperature_value_=0;
  humidity_value=0;
}

void MyData::set_temperature(float temp){
  this->temperature_.UpdateMoyenne(temp);  
}

float MyData::get_temperature(void){
  return this-> temperature.get_moyenne();
}

void MyData::set_humidity(float hum){
  this->humidity_.UpdateMoyenne(hum);  
}

float MyData::get_humidity(void){
  return this-> humidity.get_moyenne();
}
