# Lora-wan pm sensor

This sensor measures the fine dust concentration in the air and sends data over [TTN Lora-wan network](https://thethingsnetwork.org). Via a data forwarder the data is pushed to the luftdaten servers

## Project description

This sensor assembly is based on a [NodeMcu ESP8266-e12](https://github.com/nodemcu/nodemcu-devkit-v1.0), [RFM95W lora transeiver](http://www.hoperf.com/rf_transceiver/lora/RFM95W.html), [HPM pm sensor](https://sensing.honeywell.com/sensors/optical-sensors/particle-sensors/hpm-series), DHT22 humidity/temperature sensor.
This repo contains the firmware for a Influencair LoraWan sensor and the helper tools necessary for flashing these sensors.

## Requirements

- [Node.js](https://nodejs.org/en/)
- [Yarn](https://yarnpkg.com/en/docs/install) dependencie manager
- IDE ([VS-Code](https://code.visualstudio.com/), [Atom](https://atom.io/)) with [Platformio](http://platformio.org/)

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
