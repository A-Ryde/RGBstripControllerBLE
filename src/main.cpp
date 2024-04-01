#include <Arduino.h> 
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <stripLED.hpp>

BLEUUID SERVICE_UUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
BLEUUID CHARACTERISTIC_UUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

static BLEClient *pClient;
static BLERemoteService *pRemoteService;

const uint16_t NUM_LEDS = 300;

void initBLEClient()
{
  BLEDevice::init("");
  pClient = BLEDevice::createClient();
  BLEScan *pScan = BLEDevice::getScan();
  pScan->setActiveScan(true); //active scan uses more power, but get results faster
  pScan->setInterval(100);
  pScan->setWindow(99);

  while(!pClient->isConnected())
  {
    BLEScanResults pScanResults = pScan->start(5, false);
    for(int i = 0; i < pScanResults.getCount(); i++)
    {
      BLEAdvertisedDevice pAdvertisedDevice = pScanResults.getDevice(i);
      std::string pServiceName = pAdvertisedDevice.getName();
      Serial.println(pServiceName.c_str());
      if(pServiceName ==  "Long name works now")
      {
        pClient->connect(&pAdvertisedDevice);
      }
    }
  }
}

void bleUpdateState(void* /*pvParams*/)
{
  //std::string state;
  std::string brightness;

  uint32_t state_int;
  uint8_t brightness_int;

  const TickType_t xbleUpdateDelay = pdMS_TO_TICKS(1000);

  for(;;)
  {
    std::string state = pClient->getValue(SERVICE_UUID, CHARACTERISTIC_UUID);

    LED::setState(static_cast<LED::led_state_t>(stoi(state)));



    vTaskDelay(xbleUpdateDelay);
  } 
}


void setup() {
  Serial.begin(115200);
  vTaskDelay(1000);
  Serial.println("Scanning...");

  initBLEClient();

  LED::startLEDs(NUM_LEDS);
  LED::setState(static_cast<LED::led_state_t>(1));

  xTaskCreate(bleUpdateState, "updateState", 8192, NULL, 1, NULL);
}

void loop() 
{
  vTaskSuspend(NULL);
  std::string value = pClient->getValue(SERVICE_UUID, CHARACTERISTIC_UUID);
  Serial.println(value.c_str());

  LED::setState(static_cast<LED::led_state_t>(stoi(value)));

  vTaskDelay(1000);

}