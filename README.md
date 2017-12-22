# README
## Project description

This is the firmware for a Influencair LoraWan sensor. This repository also provides the helper tools necessary for programming these sensors.

You will need to create a new 'ABP' device in TTN console https://console.thethingsnetwork.org/applications/your_appID and copy the 'deviceId'

## Use

import this project within platformio.

### Set global Variables

This uses node.js and yarn. Make sure you have both installed.
Before you move on run `yarn install`, this will install all dependencies for the node.js provisioning script.

1. first set your TTN appID in your environment variables `export appID=your_appID`
2. set your TTN accessKey in your environment variables `export accessKey=your_accessKey`, make sure this key has access to 'devices' & 'settings'
3. Run `deviceId='deviceId for the device you like to program' node getDevice.js`
