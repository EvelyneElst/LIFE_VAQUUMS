/**************************************************************************************
 * vmm_helpers.h 
 * 
 * Header file containing all relevant data for vmm_helpers.cpp and other sensor code
 * in order to improve maximum re-usability of the code.
 * 
 * Last updated on 30-jan-2019 by DRoVMM
 *
 **************************************************************************************/

#ifndef __VMM_HELPERS_H__
#define __VMM_HELPERS_H__

#include <Adafruit_BME280.h>
#include <InfluxDb.h>

#include "OPCN2.h"

////////////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////////////

#define VQ_FIRMWARE_VERSION         "1.1.5"

#define USE_SERIAL                  Serial
// #define DEBUG_MODE                  1

#define NO_SENSOR                  -1     // Dummy mode, only the ESP8266
#define UNKNOWN_SENSOR              0     // Unknow letter code
#define HUZZAH_BME280_SENSOR        1     // BME280 (T, p, RH) sensor directly coupled to ESP8266
#define HUZZAH_ALPHASENSE_OPCN2     2     // Alphasense OPC-N2 PM sensor
#define ARDUINO_CITYTECH_NO2        3     // Citytech    sensor outputs 3 data fields
#define ARDUINO_CITYTECH_O3         4     // Citytech    sensor outputs 3 data fields
#define ARDUINO_MEMBRAPOR_NO2_C1    5     // Membrapor   sensor outputs 3 data fields
#define ARDUINO_MEMBRAPOR_NO2_C20   6     // Membrapor   sensor outputs 3 data fields
#define ARDUINO_MEMBRAPOR_O3_C5     7     // Membrapor   sensor outputs 3 data fields
#define ARDUINO_ALPHASENSE_OX       8     // Alphasense sensor outputs 4 data fields 
#define ARDUINO_ALPHASENSE_NO2      9     // Alphasense sensor outputs 4 data fields 
#define ARDUINO_AEROQUAL_O3         10    // Aeroqual sensors outputs 2 data fields
#define ARDUINO_DYLOS_PM            11    // Dylos sensor outputs 2 data fields
#define ARDUINO_SDS011_PM           12    // SDS sensor outputs 2 data fields
#define ARDUINO_PPD42JN_PM          13    // PPD42 outputs 4 data fields (no N)
#define ARDUINO_PPD60PV_PM          14    // PPD60PV outputs 2 data fields (no N)
#define ARDUINO_HPMA_PM             15    // HPMA outputs 2 data fields
#define ARDUINO_WINSENZH03_PM       16    // Winsen outputs 3 data fields
#define ARDUINO_PLANTOWER7003_PM    17    // PMS7003 outputs 12 data fields
#define ARDUINO_SDS_SIMULATOR       666

#define SEALEVELPRESSURE_HPA (1013.25)    // for BME280 approx. altitude reading

const char MAGIC_STX = 0x02;
const char MAGIC_ETX = 0x03;

#define BME280_SENSOR_I2C_ADDRESS    0x77

	#define BME_SCK 12
	#define BME_MISO 13
	#define BME_MOSI 2
	#define BME_CS 14 //10


//const int   AlphaSense_pins[] = {12,13,14,15};

////////////////////////////////////////////////////////////////////////////////////////
// Global Variables
////////////////////////////////////////////////////////////////////////////////////////

//static
const char ssid[]     = "wifi_ssid";
//static 
const char password[] = "myawesomewifipassword";

// static String http_protocol  = "http://";
// static String http_host      = "192.168.1.3";
// static String http_url       = "/ping";
// static String full_url       = http_protocol + http_host + http_url;

static String influxdb_name  = "vaquums_test";
//static int    influxdb_prepared_cache = 1;     // send to influx db every cache times

// for internal LAN testing
// #define       INFLUX_SRVR1_IP   "192.168.1.2"
// #define       INFLUX_SRVR2_IP   "192.168.1.3"

// on site LAN IP
#define       INFLUX_SRVR1_IP   "10.1.1.2"
#define       INFLUX_SRVR2_IP   "10.1.1.3"

////////////////////////////////////////////////////////////////////////////////////////
// Alphasense OPC-N2 specific
////////////////////////////////////////////////////////////////////////////////////////

#define HUZZAH_ALPHASENSE_CS_PIN 15 //4  // to what pin on the ESP is the CS of 
//                                     // Alphasense connected?

////////////////////////////////////////////////////////////////////////////////////////
// Custom structures
////////////////////////////////////////////////////////////////////////////////////////

struct SensorDataStruct{
    char    SensorId[256];      // sensor ID -- VQ<code>
    int     CRC_OK;             // was the CRC correct?
    int     N;                  // number of measurements
    int     num_data_fields;    // actual number of data fields
    float   meas_data[16];      // up to 16 data fields from sensor
};

////////////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////////////

void    Ratamahatta();
int     get_ESP8266_uniqueId();

int     DetectConnectedSensor();
boolean Is_BME280_Sensor_Connected();
boolean Is_AlphaSense_Sensor_Connected();
String  readSensorData(int connectedSensor, Adafruit_BME280 bme, OPCN2 alpha, int unique_id);
String  getSensorInternalId(int connectedSensor);

String  read_SerialSensorData();
int     sensorIdFromSerialSensorData(String Sensor_data);

char    GetCheckSum(String pIn);
String  read_SensorData_Dummy();
String  read_SensorData_BME280(Adafruit_BME280 bme, char Sensor_ID, int Sensor_Nr);
String  read_SensorData_AlphaSense_OPCN2(OPCN2 alpha, char Sensor_ID, int Sensor_Nr);
// String  read_SensorData_Arduino();
String  read_SensorData(int ConnectedSensorType);

void    printSensorDataStruct(SensorDataStruct *sds);
void    chopSensorDataString(int ConnectedSensorType, String Sensor_data, SensorDataStruct *sds);
SensorDataStruct     parseSensorDataString(int ConnectedSensorType, String Sensor_data);

boolean IsValidSerialSensorData(SensorDataStruct *sds);

// void    http_post_SensorData();
// boolean influx_api_post_SensorData(String sensor_id, Influxdb influx);

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

#endif