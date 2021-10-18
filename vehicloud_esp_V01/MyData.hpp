#ifndef _MYDATA_HPP_
#define _MYDATA_HPP_

#define DATA_LENGTH 10

#include "calcul_moyenne.h"

class MyData {
  
  private :

  int data_buff_[DATA_LENGTH]={0};
  CalculMoyenne temperature_value_;
  CalculMoyenne humidity_value_;
    
  public :

  //Constructeurs
  MyData();

  //Getters
  float get_temperature(void);
  float get_humidity(void);
  
  //Setters (updating moyenne)
  void add_temperature(float temp);
  void add_humidity(float hum);
  
};

#endif
