/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define Status_UUID "beb5483e-36e2-4688-b7f5-ea07361b26a8"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <ArduinoNvs.h>
#include <WiFi.h>
#include <HTTPClient.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


BLECharacteristic *pCharacteristic = NULL;
BLECharacteristic *pCharacteristic2 = NULL;
BLEServer *pServer = NULL;
BLEService *pService=NULL;
String s = "Hello World says Neil";
String WIFI_Name = "";
String WIFI_Pass = "";
String BLE_Tag = "";
String BLEMac="";
int man_code = 0x02E5;
bool isInit = 0;
int num = 0;
 BLEAdvertising *pAdvertising=NULL;
class MyCallbacks1 : public BLEServerCallbacks{
  void onConnect(BLEServer* pServer) {
    Serial.println("Connected");
      BLEDevice::startAdvertising();
    };
};

class MyCallbacks : public BLECharacteristicCallbacks
{

  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0)
    { Serial.println("Recive");
      Serial.println(rxValue.c_str());
      String mesg=String(rxValue.c_str());
      int x1=mesg.indexOf(":");
      int x2=mesg.lastIndexOf(":");



      WIFI_Name = mesg.substring(0,x1);
      WIFI_Pass = mesg.substring(x1+1,x2);
      BLE_Tag = mesg.substring(x2+1);
      Serial.println("Recive");
      Serial.println(rxValue.c_str());
            Serial.println(WIFI_Name.c_str());
            Serial.println(WIFI_Pass.c_str());

      WiFi.begin(WIFI_Name.c_str(), WIFI_Pass.c_str());
     

        if (WiFi.waitForConnectResult() == WL_CONNECTED)
        {
          pCharacteristic2->setValue("1");
          pCharacteristic2->notify();
          Serial.println("Connected to WIFI");
          
          NVS.setString("WIFI_Name", WIFI_Name);
          NVS.setString("WIFI_Pass", WIFI_Pass);
          NVS.setString("BLE_Tag",  BLE_Tag );
          isInit = 1;
          return;
        }else {
          pCharacteristic2->setValue("0");
          pCharacteristic2->notify();
          return;
        }
      
    }
  }
  std::string getValue(std::string data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.at(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substr(strIndex[0], strIndex[1]) : "";
}
};

void inits()
{
  Serial.println("Init");
  BLEDevice::init("BLE Init");
  pServer = BLEDevice::createServer();
  pService = pServer->createService("4fafc201-1fb6-459e-8fcc-c5c9c331914b");
  BLECharacteristic *pCharacteristic1 = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic2 = pService->createCharacteristic(
      Status_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY);
pServer->setCallbacks(new MyCallbacks1());
  pCharacteristic1->setCallbacks(new MyCallbacks());
  pCharacteristic1->setValue("Hello World says Neil");
  pCharacteristic2->setValue("-1");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID("4fafc201-1fb6-459e-8fcc-c5c9c331914b");
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
  while (1)
  {
    
    if (isInit)
    {

      break;
    }
            delay(100);

  }
  pServer->removeService(pService);
  BLEDevice::deinit();
  delay(500);
}
void setup()
{
  Serial.begin(115200);
  Serial.println("Starting NVS work!");

  NVS.begin();
  WIFI_Name = NVS.getString("WIFI_Name");
  WIFI_Pass = NVS.getString("WIFI_Pass");
  BLE_Tag = NVS.getString("BLE_Tag");
  if (WIFI_Name.isEmpty() || WIFI_Pass.isEmpty() || BLE_Tag.isEmpty())
  {
    inits();
  }
  if(WiFi.status()!=WL_CONNECTED){
    WiFi.begin(WIFI_Name.c_str(),WIFI_Pass.c_str());
  //  while (WiFi.status()!= WL_CONNECTED) {
  //   delay(500);
  //   Serial.println("Connecting to WiFi..");
  // }
  if (WiFi.waitForConnectResult() == WL_CONNECTED)
        {
          Serial.println("Connected to WiFi..");
        }


  }
  BLEDevice::init(BLE_Tag.c_str());
 BLEMac=BLEDevice::getAddress().toString().c_str();

  pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setValue("Hello World says Neil");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
  Serial.println("Starting BLE work!");
}
void setManData(String c, int c_size, BLEAdvertisementData &adv, int m_code) {
  
  String s;
  char b2 = (char)(m_code >> 8);
  m_code <<= 8;
  char b1 = (char)(m_code >> 8);
  s.concat(b1);
  s.concat(b2);
  s.concat(c);
    // Serial.println(s);
    adv.setName(BLE_Tag.c_str());
          



  adv.setManufacturerData(s.c_str());
  
}
void loop()
{
  // put your main code here, to run repeatedly:
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
 
    HTTPClient http;
 
    http.begin("http://192.168.124.5:5000/board"); //Specify the URL and certificate
          http.addHeader("Content-Type", "application/json");
       int httpCode=   http.POST("{\"mac\":\""+BLEMac+"\",\"name\":\""+BLE_Tag+"\"}");
                                                     //Make the request
 
    if (httpCode > 0) { //Check for the returning code
 
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
        pCharacteristic->setValue(payload.c_str());
        BLEAdvertisementData scan_response;
        
  setManData(payload, payload.length() , scan_response, man_code);
        pAdvertising->stop();
  pAdvertising->setScanResponseData(scan_response);

  pAdvertising->start();
      }
 
    else {
      Serial.println("Error on HTTP request");
    }
 
    http.end(); //Free the resources
  }
 
  delay(600000);
  // num++;

  // // s.concat(String(num));
  // String t = s + num;
  // pCharacteristic->setValue(t.c_str());
  // // BLEDevice::startAdvertising();
  // pServer->startAdvertising();
}