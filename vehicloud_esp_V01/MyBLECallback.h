#ifndef _MY_BLE_CALLBACKS_H_
#define _MY_BLE_CALLBACKS_H_

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>


class MyBLEServerCallbacks: public BLEServerCallbacks {
  private :
   bool deviceConnected=false;
  public :
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }

    bool deviceIsConnected(){
      return deviceConnected;
    }
};

class MyBLECharacteristicCallbacks: public BLECharacteristicCallbacks{
  void onRead(BLECharacteristic * pCharacteristic){
  
  }

  void onWrite(BLECharacteristic * pCharacteristic){
    
  }
};


#endif
