#include <Arduino.h>
#include <SPI.h>
#include <BLEPeripheral.h>

BLEPeripheral ledPeripheral = BLEPeripheral();

BLEService ledService = BLEService("19b10000e8f2537e4f6cd104768a1214");
BLECharCharacteristic ledCharacteristic = BLECharCharacteristic("19b10001e8f2537e4f6cd104768a1214", BLERead | BLEWrite);

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  ledPeripheral.setAdvertisedServiceUuid(ledService.uuid());
  ledPeripheral.setAdvertisingInterval(3000);
  ledPeripheral.addAttribute(ledService);
  ledPeripheral.addAttribute(ledCharacteristic);
  ledPeripheral.setLocalName("Nordic NRF52 DK");
  ledPeripheral.begin();
}

void loop()
{
  BLECentral central = ledPeripheral.central();
  int value = 0;
  if (central)
  {
    while (central.connected())
    {
      ledCharacteristic.setValue(value);
      ledPeripheral.setTxPower(4);
      delay(1000);
      if(value == 10){
        value = 0;
        central.disconnect();
        ledPeripheral.setConnectable(false);
      }
      value++;
      // if (ledCharacteristic.written())
      // {
      //   if (ledCharacteristic.value())
      //   {
      //     digitalWrite(LED_BUILTIN, HIGH);
      //   }
      //   else
      //   {
      //     digitalWrite(LED_BUILTIN, LOW);
      //   }
      // }
    }
    //q: i only want to send once a day to save a lot of power, how do i do this, i dont want it to be constantly advertising
    //a: you can set the advertising interval to 0, and then call startAdvertising() when you want to advertise
  }
}