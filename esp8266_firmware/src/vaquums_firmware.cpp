/***************************************************************************
  ____   _________  ________   ____ ___ ____ ___  _____    _________
  \   \ /   /  _  \ \_____  \ |    |   |    |   \/     \  /   _____/
   \   Y   /  /_\  \ /  / \  \|    |   |    |   /  \ /  \ \_____  \ 
    \     /    |    /   \_/.  |    |  /|    |  /    Y    \/        \
     \___/\____|__  \_____\ \_|______/ |______/\____|__  /_______  /
                  \/       \__>                        \/        \/ 
				  
				  
	VAQUUMS firmware for the (Huzzah Feather) ESP8266 to read out all 
	attached air quality sensors, connected either directly to the ESP
	or with an Arduino coupled to the ESP's serial RX input.
	
	All sensor data is sent over WiFi to an InfluxDB running on a 
	dedicated host/server.
	
	.-----
	Last updated on 29-jan-2019 by D. Roet (d.roet@vmm.be)
	
 ***************************************************************************/
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>

#include "vmm_helpers.h"
#include "vmm_helpers_influx.h"
#include "vmm_helpers_ota.h"

OPCN2               alpha(HUZZAH_ALPHASENSE_CS_PIN);

Adafruit_BME280     bme;                // BME sensor -- I2C connection
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

ESP8266WiFiMulti    WiFiMulti;          // WiFi multi AP
Influxdb            influx_server1(INFLUX_SRVR1_IP);  // Influx instance
Influxdb            influx_server2(INFLUX_SRVR2_IP);  // Influx instance

int             unique_id;

const int       sampleTime = 1000;
float           ulActMilli;
float           ulActSampleTime;

int             NumInfluxPrepared = 0;

int             connectedSensor = -1;

bool            BME_status;

String              Sensor_datastring="";
SensorDataStruct    sds;

String          sensor_hostname="VQHost";
char            sensor_hostname_char[256]="VQHost";

void setup() {

    Serial.begin(9600);

    // init 
    Serial.println();
    Serial.println("VQ firmware v"+String(VQ_FIRMWARE_VERSION));

    unique_id = get_ESP8266_uniqueId();
    Serial.println("Unique id for this ESP ="+String(unique_id));

    // detect connected sensor type
    Serial.println("Detecting sensor");
    connectedSensor = DetectConnectedSensor();

    // BME280 setup
    if (connectedSensor == HUZZAH_BME280_SENSOR){

        // Serial.println("Detected BME280");
        BME_status = bme.begin();  
        if (!BME_status) {
            Serial.println("Could not find a valid BME280 sensor, check wiring!");
            while (1);
        }

    }

    if (connectedSensor == HUZZAH_ALPHASENSE_OPCN2){
        Status stat = alpha.read_status();
        Serial.print("status > fanOn    "); Serial.println(stat.fanON);
        Serial.print("status > laserON  "); Serial.println(stat.laserON);
        Serial.print("status > fanDAC   "); Serial.println(stat.fanDAC);
        Serial.print("status > laserDAC "); Serial.println(stat.laserDAC);

        delay(1000);

        // Turn on the fan
        Serial.println(".on()");
        Serial.println("\t" + String(alpha.on()));

        // Wait for a bit
        delay(3000);
    }

    // build hostname from unique sensorId + FW version
    sensor_hostname=getSensorInternalId(connectedSensor)+"-"+"v"+String(VQ_FIRMWARE_VERSION);
    sensor_hostname.replace(".","_");
    sensor_hostname.toCharArray(sensor_hostname_char, 256);

    Serial.print("Setting sensor hostname:"+sensor_hostname+" ... ");
    if (wifi_station_set_hostname(sensor_hostname_char)){
        Serial.println("success !");
    }
    else{
        Serial.println("failed ?!");
    }

    //Serial.println("Hostname:"+WiFi.hostname());

    WiFi.setAutoConnect(false);

    if (!wifi_station_set_auto_connect(false)){
        Serial.println("Disable autoconnect failed");
    }

    // initialize WiFi
    Serial.printf("Connecting to SSID [%s]\n", ssid);
  
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);      // Connect in multi-mode, useful to add fallback AP's later

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
  
    Serial.println("");
    Serial.print("WiFi connected, IP address: ");
    Serial.println(WiFi.localIP());

    // setup OTA
    setUp_httpUpdate();     // over http
    setUp_OTA();            // over wifi

    // initialize Influx connections
    influx_server1.setDb(influxdb_name);
    influx_server2.setDb(influxdb_name);

    // initialize timings
    ulActMilli = millis();
    ulActSampleTime = ulActMilli + sampleTime;

    // influx
    NumInfluxPrepared = 0;
}


void loop() { 

    boolean             InfluxWriteSucces;

    // Activate OTA handler
    ArduinoOTA.handle();
    
    // main code 
    ulActMilli = millis();

    if (ulActMilli > ulActSampleTime){
    
        Sensor_datastring=readSensorData(connectedSensor, bme, alpha, unique_id);
        sds=parseSensorDataString(connectedSensor, Sensor_datastring);
    
        // if (!IsValidSerialSensorData(&sds))
        //     Serial.println("Invalid NA data!");

        if (connectedSensor == HUZZAH_ALPHASENSE_OPCN2 && sds.meas_data[0] < 0){
            Serial.println("OPCN2 returing negative measurements, restarting ESP.");
            ESP.restart();
        }

        // // only attempt to send to InfluxDB when data received from sensor and WiFi connected
        if (IsValidSerialSensorData(&sds) && WiFi.status() == WL_CONNECTED){

// #ifdef DEBUG_MODE
            printSensorDataStruct(&sds);
// #endif
            InfluxData SensorDataInflux = SensorData_to_Influx(connectedSensor, &sds);
            
                InfluxWriteSucces = influx_server1.write(SensorDataInflux);

                if (!InfluxWriteSucces){
                    Serial.println("Error: Writing to InfluxDB on server 1 failed");
                }

                InfluxWriteSucces = influx_server2.write(SensorDataInflux);

                if (!InfluxWriteSucces){
                    Serial.println("Error: Writing to InfluxDB on server 2 failed");
                }

        }
        // else {
        //     Serial.println("Error: Can't write to InfluxDB, no sensor data and/or WiFi!");
        // }

        // next sampleTime stamp
        ulActSampleTime = ulActMilli + sampleTime;

    }

}

