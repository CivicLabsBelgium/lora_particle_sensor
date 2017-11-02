#include <ESP8266WiFi.h>
#include <Esp.h>

#include <SoftwareSerial.h>

#include <DHT.h>

#define SEND_MESSAGE 3*60*1000
#define SENSOR_WARMUP 10000
#define SENSOR_READ 70*1000


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

void read_DHT() {
    float h = dht.readHumidity(); //Read Humidity
	float t = dht.readTemperature(); //Read Temperature

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
                Serial.println("Checksum is bad");
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

}

void setup() {
    Serial.begin(115200);

    // switch WiFi OFF
    //WiFi.disconnect();
    //WiFi.forceSleepBegin();
    delay(1);

    prev_message_time = millis();

    dht.begin();
}

void loop() {
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

            sendData();
            mode = 0;
            prev_message_time = millis();
            Serial.println("Lets wait a bit");
        break;
    }
}
