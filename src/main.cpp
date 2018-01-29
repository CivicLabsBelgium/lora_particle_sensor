/* Honeywell HPMA TTN LORAWAN NODE */

#include <ESP8266WiFi.h>
#include <Esp.h>
#include <base64.h>

// All specific changes needed for ESP8266 need be made in hal.cpp if possible
// Include ESP environment definitions in lmic.h (lmic/limic.h) if needed
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#include <SoftwareSerial.h>

#include "DHT.h"

#include "global.h"

//---------------------------------------------------------
// APPLICATION CALLBACKS
//---------------------------------------------------------

// provide application router ID (8 bytes, LSBF)
void os_getArtEui (u1_t* buf) {
    memcpy(buf, APPEUI, 8);
}

// provide device ID (8 bytes, LSBF)
void os_getDevEui (u1_t* buf) {
    memcpy(buf, DEVEUI, 8);
}

// provide device key (16 bytes)
void os_getDevKey (u1_t* buf) {
    memcpy(buf, DEVKEY, 16);
}

// Pin mapping for RFM95
lmic_pinmap pins = {
  .nss = D8,			// Make D8/GPIO15, is nSS on ESP8266
  .rxtx = 0xFF, 		// Not used, Do not connected on RFM92/RFM95
  .rst = 0xFF,  		// Not used
//  .dio = { LMIC_UNUSED_PIN, LMIC_UNUSED_PIN, LMIC_UNUSED_PIN },
  .dio = {D0, D2, 0xFF},	// Specify pin numbers for DIO0, 1, 2;
};
// Reset, DIO2 and RxTx are not connected AND in hal.cpp NOT initialised.



void onEvent (ev_t ev) {
    //debug_event(ev);
    switch(ev) {
      // scheduled data sent (optionally data received)
      // note: this includes the receive window!
      case EV_TXCOMPLETE:
          // use this event to keep track of actual transmissions
          Serial.print("Event EV_TXCOMPLETE, time: ");
          Serial.println(millis() / 1000);
          if(LMIC.dataLen) { // data received in rx slot after tx
              //debug_buf(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
              Serial.println("Data Received!");
          }
          break;
       default:
          break;
    }
}

#define SEND_MESSAGE 2.5*60*1000 // 3*60*1000
#define SENSOR_WARMUP 10000 //10000
#define SENSOR_READ 60*1000 // 70*1000


#define DHT_PIN D1
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

SoftwareSerial HPMSerial(D3, D4, false, 128);

bool HPM_is_running = false;
int mode = 0;
unsigned long prev_message_time;

const uint8_t start_HPM_cmd[] = { 0x68, 0x01, 0x01, 0x96 };
const uint8_t stop_HPM_cmd[] = { 0x68, 0x01, 0x02, 0x95 };

int hpm_pm10_sum = 0;
int hpm_pm25_sum = 0;
int hpm_val_count = 0;
int hpm_pm10_max = 0;
int hpm_pm10_min = 20000;
int hpm_pm25_max = 0;
int hpm_pm25_min = 20000;
int len = 0;
int pm10_serial = 0;
int pm25_serial = 0;
int checksum_is;
int checksum_should;

int dht_temp_sum = 0;
int dht_hum_sum = 0;
int dht_val_count = 0;

static osjob_t sendjob;

void read_DHT() {
    float h = dht.readHumidity(); //Read Humidity
	float t = dht.readTemperature(); //Read Temperature
    if (isnan(t) || isnan(h)) {
        delay(100);
        h = dht.readHumidity(true); //Read Humidity
        t = dht.readTemperature(false,true); //Read Temperature
    }
    if (isnan(t) || isnan(h)) {
		Serial.println("DHT22 couldn't be read");
	} else {
        dht_temp_sum += t;
        dht_hum_sum += h;
        dht_val_count++;
    }
}

bool read_HPM_Sensor() {
    char buffer;
    int value;
    int checksum_ok = 0;

    while (HPMSerial.available() > 0) {
        buffer = HPMSerial.read();
        value = int(buffer);
        switch (len) {
        case (0):
            if (value != 66) {
                len = -1;
            };
            break;
        case (1):
            if (value != 77) {
                len = -1;
            };
            break;
        case (2):
            checksum_is = value;
            break;
        case (3):
            checksum_is += value;
            break;
        case (4):
            checksum_is += value;
            break;
        case (5):
            checksum_is += value;
            break;
        case (6):
            pm25_serial += ( value << 8);
            checksum_is += value;
            break;
        case (7):
            pm25_serial += value;
            checksum_is += value;
            break;
        case (8):
            pm10_serial = ( value << 8);
            checksum_is += value;
            break;
        case (9):
            pm10_serial += value;
            checksum_is += value;
            break;
        case (10):
            checksum_is += value;
            break;
        case (11):
            checksum_is += value;
            break;
        case (12):
            checksum_is += value;
            break;
        case (13):
            checksum_is += value;
            break;
        case (14):
            checksum_is += value;
            break;
        case (15):
            checksum_is += value;
            break;
        case (16):
            checksum_is += value;
            break;
        case (17):
            checksum_is += value;
            break;
        case (18):
            checksum_is += value;
            break;
        case (19):
            checksum_is += value;
            break;
        case (20):
            checksum_is += value;
            break;
        case (21):
            checksum_is += value;
            break;
        case (22):
            checksum_is += value;
            break;
        case (23):
            checksum_is += value;
            break;
        case (24):
            checksum_is += value;
            break;
        case (25):
            checksum_is += value;
            break;
        case (26):
            checksum_is += value;
            break;
        case (27):
            checksum_is += value;
            break;
        case (28):
            checksum_is += value;
            break;
        case (29):
            checksum_is += value;
            break;
        case (30):
            checksum_should = ( value << 8 );
            break;
        case (31):
            checksum_should += value;
            break;
        }
        len++;
        if (len == 32) {
            if (checksum_should == (checksum_is + 143)) {
                checksum_ok = 1;
            }
            else {
                len = 0;
                pm10_serial = 0;
                pm25_serial = 0;
                checksum_is;
                checksum_should;
            };
        }
        if (len == 32 && checksum_ok == 1) {
            if ((! isnan(pm10_serial)) && (! isnan(pm25_serial))) {
                hpm_pm10_sum += pm10_serial;
                hpm_pm25_sum += pm25_serial;
                if (hpm_pm10_min > pm10_serial) {
                    hpm_pm10_min = pm10_serial;
                }
                if (hpm_pm10_max < pm10_serial) {
                    hpm_pm10_max = pm10_serial;
                }
                if (hpm_pm25_min > pm25_serial) {
                    hpm_pm25_min = pm25_serial;
                }
                if (hpm_pm25_max < pm25_serial) {
                    hpm_pm25_max = pm25_serial;
                }
                hpm_val_count++;
                len = 0;
                pm10_serial = 0;
                pm25_serial = 0;
                checksum_is;
                checksum_should;
            }
        }
    }

    if (checksum_ok == 1) {
        return true;
    } else {
        return false;
    }
}

void sendData () {
    /*
    Serial.print("PM2.5 (mean.): ");
    Serial.println(float(hpm_pm25_sum/hpm_val_count));
    Serial.print("PM2.5 (min.): ");
    Serial.println(float(hpm_pm25_min));
    Serial.print("PM2.5 (max.): ");
    Serial.println(float(hpm_pm25_max));
    Serial.print("PM10 (mean.): ");
    Serial.println(float(hpm_pm10_sum/hpm_val_count));
    Serial.print("PM10 (min.): ");
    Serial.println(float(hpm_pm10_min));
    Serial.print("PM10 (max.): ");
    Serial.println(float(hpm_pm10_max));
    Serial.print("PM Samples: ");
    Serial.println(hpm_val_count);

    Serial.print("temp (mean.): ");
    Serial.println(float(dht_temp_sum/dht_val_count));
    Serial.print("hum (mean.): ");
    Serial.println(float(dht_hum_sum/dht_val_count));
    Serial.print("DHT Samples: ");
    Serial.println(dht_val_count);

    Serial.println();
    Serial.print("Time: "); Serial.println(millis() / 1000);
    // Show TX channel (channel numbers are local to LMIC)
    Serial.print("Send, txCnhl: "); Serial.println(LMIC.txChnl);
    Serial.print("Opmode check: ");
    // Check if there is not a current TX/RX job running
    */
    if (LMIC.opmode & (1 << 7)) {
      Serial.println("OP_TXRXPEND, not sending");
    } else {
      Serial.print("ok, ready to send: ");
      Serial.println();

      byte bytsend[10];                   // !!!! MAx 10 Bytes to send !!!!
      int idx = 0;

      int pm10 = 0;
      int pm25 = 0;
      if (hpm_val_count != 0) {
        pm10 = (int)((hpm_pm10_sum/hpm_val_count)*100);
        pm25 = (int)((hpm_pm25_sum/hpm_val_count)*100);
      }
      
      int temp = 0;
      int hum = 0;
      if (dht_val_count != 0) {
        temp = (int)((dht_temp_sum/dht_val_count)*100);
        hum = (int)((dht_hum_sum/dht_val_count)*100);
      }
      
      bytsend[0] = highByte(pm10);
      bytsend[1] = lowByte(pm10);
      bytsend[2] = highByte(pm25);
      bytsend[3] = lowByte(pm25);
      bytsend[4] = highByte(hum);
      bytsend[5] = lowByte(hum);
      bytsend[6] = highByte(temp);
      bytsend[7] = lowByte(temp);
      bytsend[8] = byte(2); // 2 : HPM sensor
      bytsend[9] = byte(1); // 1 : DHT sensor

      uint8_t mydata[64];
      memcpy((char *)mydata, (char *)bytsend, 10);
      int k;
      for(k = 0; k < 10; k++) {
        Serial.print(mydata[k],HEX);
        Serial.print(" ");
      }

      Serial.println();

      // Prepare upstream data transmission at the next possible time.
      // LMIC_setTxData2(1, mydata, strlen((char *)mydata), 0);
      LMIC_setTxData2(1, mydata, 10, 0);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.print("ok");

    // switch WiFi OFF
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin(0);
    delay(1);

    // LMIC init
    os_init();

    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Set static session parameters. Instead of dynamically establishing a session
    // by joining the network, precomputed session parameters are be provided.
    LMIC_setSession (0x1, DEVADDR, (uint8_t*)DEVKEY, (uint8_t*)ARTKEY);

    // Disable data rate adaptation
    LMIC_setAdrMode(0);

    // Disable link check validation
    LMIC_setLinkCheckMode(0);
    // Disable beacon tracking
    LMIC_disableTracking ();
    // Stop listening for downstream data (periodical reception)
    LMIC_stopPingable();
    // Set data rate and transmit power (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);

    prev_message_time = millis();

    dht.begin();

}

void loop() {
    while(1) {
        switch (mode) {
            case (0):
                // wait till later
                if (millis() > prev_message_time + SEND_MESSAGE - SENSOR_READ - SENSOR_WARMUP) {
                    mode = 1;
                    Serial.println("Lets start the sensor");
                }
            break;
            case (1):
                // start sensor
                HPMSerial.write(start_HPM_cmd,sizeof(start_HPM_cmd));
                mode = 2;
                Serial.println("Warm up the sensor");
            break;
            case (2):
                // Warmup time
                if (millis() > prev_message_time + SEND_MESSAGE - SENSOR_READ) {
                    mode = 3;
                    Serial.println("Lets read the data");
                    HPMSerial.flush();
                    hpm_pm10_sum = 0;
                    hpm_pm25_sum = 0;
                    hpm_val_count = 0;
                    hpm_pm10_max = 0;
                    hpm_pm10_min = 20000;
                    hpm_pm25_max = 0;
                    hpm_pm25_min = 20000;

                    dht_temp_sum = 0;
                    dht_hum_sum = 0;
                    dht_val_count = 0;
                }
            break;
            case (3):
                if (read_HPM_Sensor()) {
                    read_DHT();
                }
                if (millis() > prev_message_time + SEND_MESSAGE) {
                    mode = 4;
                    HPMSerial.write(stop_HPM_cmd,sizeof(stop_HPM_cmd));
                    Serial.println("Lets send the data");
                }
            break;
            case (4):
                if (hpm_val_count > 0 && dht_val_count > 0) {
                    sendData();
                }
                mode = 0;
                prev_message_time = millis();
                Serial.println("Lets wait a bit");
            break;
        }

        os_runloop_once();
        delay(10);
    }
}
