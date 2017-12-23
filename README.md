# Lora-wan pm sensor

This sensor measures the fine dust concentration in the air and sends data over [TTN Lora-wan network](https://thethingsnetwork.org). Via a data forwarder the data is pushed to the luftdaten servers

## Project description

This is the firmware for a Influencair LoraWan sensor based on a Honeywell HPM series pm sensor, ESP8266 and RFM95W. For the LoraWan network we use The Things Network.

This repo contains the firmware for a Influencair LoraWan sensor and the helper tools necessary for flashing these sensors.

This sensor assembly is based on a [NodeMcu ESP8266-e12](https://github.com/nodemcu/nodemcu-devkit-v1.0), [RFM95W lora transeiver](http://www.hoperf.com/rf_transceiver/lora/RFM95W.html), [HPM pm sensor](https://sensing.honeywell.com/sensors/optical-sensors/particle-sensors/hpm-series), DHT22 humidity/temperature sensor.

## Requirements
- [Node.js](https://nodejs.org/en/)
- [Yarn](https://yarnpkg.com/en/docs/install) dependencie manager
- IDE ([VS-Code](https://code.visualstudio.com/), [Atom](https://atom.io/)) with [Platformio](http://platformio.org/)

## Hardware

### HPM sensor
```
- pin 1: Not connected
- pin 2: NodeMCU UV (USB Voltage = 5V)
- pin 3: Not connected
- pin 4: Not connected
- pin 5: Not connected
- pin 6: NodeMCU D3
- pin 7: NodeMCU D4
- pin 8: NodeMCU GND
```

### DHT22 sensor
```
- pin 1: NodeMCU UV (USB Voltage = 5V)
- pin 2: NodeMCU D1
- pin 3: Not connected
- pin 4: GND
```

### RFM95W
```
- GND : GND
- MISO: NodeMCU D6
- MOSI: NodeMCU D7
- SCK : NodeMCU D5
- NSS : NodeMCU D8
- RST : Not connected
- GND : GND
- DIO2: Not connected
- DIO1: NodeMCU D2
- DIO0: NodeMCU D0
- 3.3V: NodeMCU 3.3V
- DIO4: Not connected
- DIO3: Not connected
- GND : GND
- ANT : ANTENNA
```

## LoraWan Setup

You will need an account at https://console.thethingsnetwork.org and create an application. 
In the application go to _**payload formats**_ and add this decode function:
```
function Decoder(bytes, port) {
  var decoded = {};

  // Decode bytes to int
  var p10int = (bytes[0] << 8) | bytes[1];
  var p25int = (bytes[2] << 8) | bytes[3];
  var hum = (bytes[4] << 8) | bytes[5];
  var temp = (bytes[6] << 8) | bytes[7];

  // Decode int to float
  decoded.pm10 = p10int / 100;
  decoded.pm25 = p25int / 100;
  decoded.humidity = hum / 100;
  decoded.temperature = temp / 100;

  return decoded;
}
```

Next create a new 'ABP' device in TTN console https://console.thethingsnetwork.org/applications/your_appID and copy the 'deviceId'

## Use

1. open this project within Platformio.
2. Run `yarn` in the project root
3. Go to [TTN console](https://console.thethingsnetwork.org/applications/) and open your TTN application
4. Copy your appID
5. Set your TTN appID in your environment variables `export appID=your_appID`
6. Copy your application accessKey, make sure this key has access to 'devices' & 'settings'
7. set your TTN accessKey in your environment variables `export accessKey=your_accessKey`

### Flash a sensor
1. Create a new 'ABP' device in the TTN console and  copy the 'deviceId'
2. Run `deviceId='deviceId' node getDevice.js`

## By [Influencair](http://influencair.be)
