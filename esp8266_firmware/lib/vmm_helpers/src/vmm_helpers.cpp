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
	
	Helper functions to keep the main code as readable, as possible.
	
	.-----
	Last updated on 29-jan-2019 by DRoVMM
	
 ***************************************************************************/
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>

#include <Wire.h>

#include "vmm_helpers.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
String readSensorData(int connectedSensor, Adafruit_BME280 bme, OPCN2 alpha, int unique_id){

    String Sensor_datastring = "";

    switch(connectedSensor){
        case HUZZAH_BME280_SENSOR:  Sensor_datastring=read_SensorData_BME280(bme, 'Z', unique_id);
                                    break;
        case HUZZAH_ALPHASENSE_OPCN2: Sensor_datastring=read_SensorData_AlphaSense_OPCN2(alpha, 'Y', unique_id);
                                    break;
        default:                    Sensor_datastring= read_SerialSensorData();
        }

    return(Sensor_datastring);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// get_ESP8266_uniqueId()
//
// Generate a unique ESP8266 identifier based on the combo of the chip and flashchip id's
/////////////////////////////////////////////////////////////////////////////////////////////////////
int get_ESP8266_uniqueId(){

  int chipId = ESP.getChipId();
  int flashChipId = ESP.getFlashChipId();

  return( (chipId + flashChipId) % 255);
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// getSensorInternalId()
//
// get the internal (VQ...) name of a sensor, for BME280 and the OPCN2 this
// is generated from the ESP8266's unique id, all other connected sensors 
// send their id's over serial
/////////////////////////////////////////////////////////////////////////////////////////////////////
String getSensorInternalId(int connectedSensor){

    String SensorInternalId = "UnknownId";
    String Sensor_datastring = "";
    SensorDataStruct sds;
    int    esp_unique_id = get_ESP8266_uniqueId();

    switch(connectedSensor){
        case HUZZAH_BME280_SENSOR:  SensorInternalId="VQZ"+String(esp_unique_id);
                                    break;
        case HUZZAH_ALPHASENSE_OPCN2: SensorInternalId="VQY"+String(esp_unique_id);
                                    break;
        default:                    Sensor_datastring= read_SerialSensorData();
                                    sds=parseSensorDataString(connectedSensor, Sensor_datastring);
                                    SensorInternalId=String(sds.SensorId);
        }

    return(SensorInternalId);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Is_BME280_Sensor_Connected()
//
// Check for connection on the I2C address, should be at 0x77
/////////////////////////////////////////////////////////////////////////////////////////////////////
boolean Is_BME280_Sensor_Connected(){

/*     Adafruit_BME280 bme;

    Serial.println("Checking I2C for BME280 ...");

    bool status=bme.begin();

    return(status);

*/
   Wire.begin();
   Wire.beginTransmission(BME280_SENSOR_I2C_ADDRESS);
    
    // endTransmission code 0 = SUCCESS
   if (Wire.endTransmission () == 0){
       Serial.println("Found!!");
       return(true);
    }
   else{
       Serial.println("Not found!!");
       return(false);
   }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Is_AlphaSense_Sensor_Connected()
//
// Check if the AlphaSense PM sensor is connected to the ESP8266 at predefined pins.
/////////////////////////////////////////////////////////////////////////////////////////////////////
boolean Is_AlphaSense_Sensor_Connected(){

    OPCN2 alpha(HUZZAH_ALPHASENSE_CS_PIN);

    if (alpha.ping() == 1 && alpha.firm_ver.major >= 18){
        return(true);
    }
    else{
        return(false);
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// DetectConnectedSensor()
//
// Wrapper function to detect the sensor connected to the ESP8266
/////////////////////////////////////////////////////////////////////////////////////////////////////
int DetectConnectedSensor(){

    int connectedSensor = -1;
    int detectionRetryDelay = 1000;

    while (connectedSensor == -1){
        // detect connected sensor type

        if (Is_BME280_Sensor_Connected()){
            Serial.println("BME sensor found at I2C address "+String(BME280_SENSOR_I2C_ADDRESS, HEX));

            connectedSensor=HUZZAH_BME280_SENSOR;
        }
        else if (Is_AlphaSense_Sensor_Connected()){
            Serial.println("AlphaSense connected!");
            
            connectedSensor=HUZZAH_ALPHASENSE_OPCN2;
        }
        else{
            Serial.println("No sensor found directly connected to ESP8266, checking Serial input");

            String sensorSerial = read_SerialSensorData();
            connectedSensor=sensorIdFromSerialSensorData(sensorSerial);

            if (connectedSensor == UNKNOWN_SENSOR)
                connectedSensor=-1;

        }

        delay(detectionRetryDelay);
    }

    return(connectedSensor);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// GetCheckSum()
//
// Performs a XOR on all consecutive characters and returns the result.
/////////////////////////////////////////////////////////////////////////////////////////////////////
char GetCheckSum(String pIn){

    int iXor = 0;
    int i = 0;

    while (pIn[i] != '\0'){
        iXor ^= pIn[i++];
    }

    if (iXor == 0)
        iXor = 1;

    return char(iXor);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//	read_SerialSensorData()
//
//  main function to read out all the sensor data incoming on the Serial (RX) port of the ESP8266
/////////////////////////////////////////////////////////////////////////////////////////////////////
String    read_SerialSensorData(){

        String  sensorSerial = "";
        char    serialChar = 0x00;
        bool    STX_Received=false;
        bool    sensorSerialReceived=false;
        int     lastTime=millis();
        int     timePassed=0;
        int     actTime=0;
        int     timeOut=10;

        // while (Serial.available() > 0){
        //     serialChar = char(Serial.read());
        //     if (serialChar == MAGIC_STX)
        //         sensorSerial="";
        //     sensorSerial += String(serialChar);
        //     delay(1);
        // }

        // Read serial characters and detect valid dataString
        while (!sensorSerialReceived && timePassed < timeOut){

            actTime=millis();

            if (Serial.available() > 0){

                serialChar = char(Serial.read());

                switch(serialChar){
                    case    MAGIC_ETX:  if (STX_Received)
                                            sensorSerialReceived=true;
                                        // Serial.println("STOP!");
                    case    MAGIC_STX:  if (!sensorSerialReceived)
                                            sensorSerial=""; 
                                        STX_Received=true;
                                        // Serial.println("GO!");
                    default:    sensorSerial += String(serialChar);
                                lastTime=actTime;
                                // Serial.print("["+String(serialChar)+"]");
                }
            }   

            timePassed=actTime-lastTime;

        }

        // Get the CRC, the extra character after ETX
        if (sensorSerialReceived){

            while (!Serial.available())
                delay(1);

            serialChar=char(Serial.read());
            sensorSerial+=String(serialChar);

        }

#ifdef DEBUG_MODE
        if (sensorSerial.length() > 0)
            Serial.println("<<"+sensorSerial+">>");
#endif

        return(sensorSerial);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// read_SensorData_Dummy()
//
// dummy test function
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
String  read_SensorData_Dummy(){

    String  Sensor_datastring="";
    int     Sensor_ID = int(random(1,26));
    int     Sensor_Nr = int(random(0,9));
    int     N         = int(random(1,100));
    int     PM25      = random(1,25);
    int     PM10      = PM25+random(1,25);
    char    CRC;

    Sensor_datastring    =   MAGIC_STX;         // STX
    Sensor_datastring   +=   "VQ";
    Sensor_datastring   +=   String(Sensor_ID);
    Sensor_datastring   +=   String(Sensor_Nr);
    Sensor_datastring   +=   ";";
    Sensor_datastring   +=   String(N);         // N
    Sensor_datastring   +=   ";";
    Sensor_datastring   +=   String(PM25);      // PM2,5
    Sensor_datastring   +=   ";";
    Sensor_datastring   +=   String(PM10);      // PM10
    Sensor_datastring   +=   ";";
    Sensor_datastring   +=   MAGIC_ETX;         // ETX

    CRC=GetCheckSum(Sensor_datastring);

    Sensor_datastring += String(CRC);

    Serial.print("No sensor attached, invented data [");
    Serial.print(Sensor_datastring);
    Serial.println("]");

    return(Sensor_datastring);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// read_SensorData_BME280()
//
// read data from the BME280 (T, p, RH) sensor which is directly connected to the ESP8266 by I2C connection
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
String  read_SensorData_BME280(Adafruit_BME280 bme, char Sensor_ID, int Sensor_Nr){

    String  Sensor_datastring="";
    int     N         = 1;
    char    CRC;
    float   T, p, RH, altitude;

    Serial.println("Reading data from I2C coupled BME280");

    T        = bme.readTemperature();
    p        = bme.readPressure() / 100.0F;
    RH       = bme.readHumidity();
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);

    Serial.println("Data from sensor "+String(T)+" "+String(p)+" "+String(RH)+" "+String(altitude));

	// Build the sensor data string
    Sensor_datastring    =   MAGIC_STX;      // STX
    Sensor_datastring   +=   "VQ";
    Sensor_datastring   +=   String(Sensor_ID);
    Sensor_datastring   +=   String(Sensor_Nr);
    Sensor_datastring   +=   ";";
    Sensor_datastring   +=   String(N);    // N
    Sensor_datastring   +=   ";";
    Sensor_datastring   +=   String(T);     // Temperature (deg C)
    Sensor_datastring   +=   ";";
    Sensor_datastring   +=   String(p);     // Pressure (hPa)
    Sensor_datastring   +=   ";";
    Sensor_datastring   +=   String(RH);    // Rel. humidity (%)
    Sensor_datastring   +=   ";";
    Sensor_datastring   +=   String(altitude);   // approx. altitude (m)
    Sensor_datastring   +=   ";";
    Sensor_datastring   +=   MAGIC_ETX;

    CRC=GetCheckSum(Sensor_datastring);

    Sensor_datastring += String(CRC);

    return(Sensor_datastring);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// read_SensorData_AlphaSense_OPCN2()
//
// read PM-data from the Alphasense OPC-N2 sensor which is directly connected 
// to the ESP8266 by SPI connection
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
String  read_SensorData_AlphaSense_OPCN2(OPCN2 alpha, char Sensor_ID, int Sensor_Nr){

    String  Sensor_datastring="";
    int     N         = 1;
    char    CRC;
    float   pm1, pm25, pm10;
    PMData  pm;

    Serial.println("Reading data from SPI coupled AlphaSense OPC-N2");

    pm         = alpha.read_pm_data();
    pm1        = pm.pm1;
    pm25       = pm.pm25;
    pm10       = pm.pm10;

    Serial.println("Data from sensor "+String(pm1)+" "+String(pm25)+" "+String(pm10));

	// Build the sensor data string
    Sensor_datastring    =   MAGIC_STX;      // STX
    Sensor_datastring   +=   "VQ";
    Sensor_datastring   +=   String(Sensor_ID);
    Sensor_datastring   +=   String(Sensor_Nr);
    Sensor_datastring   +=   ";";
    Sensor_datastring   +=   String(N);    // N
    Sensor_datastring   +=   ";";
    Sensor_datastring   +=   String(pm1);  // PM1   (ug/m3)
    Sensor_datastring   +=   ";";
    Sensor_datastring   +=   String(pm25); // PM2,5 (ug/m3)
    Sensor_datastring   +=   ";";
    Sensor_datastring   +=   String(pm10); // PM10  (ug/m3)
    Sensor_datastring   +=   ";";
    Sensor_datastring   +=   MAGIC_ETX;

    CRC=GetCheckSum(Sensor_datastring);

    Sensor_datastring += String(CRC);

    return(Sensor_datastring);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// read_SensorData_Arduino()
//
// read data from any sensor connected to an Arduino which is then connected to the ESP8266's RX pin
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// String  read_SensorData_Arduino(){

//     return("Reading data from RX coupled Arduino Sensor");
// }

// /////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////
// void http_post_SensorData(){

//         HTTPClient http;

//         USE_SERIAL.print("[HTTP] begin...\n");
//         USE_SERIAL.println(full_url);
        
//         //HTTP
//         if (!http.begin(full_url)){
//           Serial.println("http.begin() failed?"); 
//         }
//         else{
//           USE_SERIAL.print("[HTTP] GET...\n");
//           // start connection and send HTTP header
//           int httpCode = http.GET();
  
//           // httpCode will be negative on error
//           if(httpCode > 0) {
//               // HTTP header has been send and Server response header has been handled
//               USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
  
//               // file found at server
//               if(httpCode == HTTP_CODE_OK) {
//                   String payload = http.getString();
//                   USE_SERIAL.println(payload);
//               }

          
//           } else {
//               USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
//           }
//         }

//         delay(5000);
//         http.end();

// }

// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// boolean influx_api_post_SensorData(String sensor_id, Influxdb influx){

//     InfluxData row("sensor_data");
//     row.addTag("sensor_id", sensor_id);
//     row.addValue("PM25", random(10, 40));
//     row.addValue("PM10", random(20, 60));
    
//     boolean success = influx.write(row);

//     return(success);
//     // InfluxData  Temp     = random(1,10);
//     // InfluxData  RH       = random(10, 100);
//     // InfluxData  pressure = random(100, 10000);

//     // influx.prepare(Temp);
//     // influx.prepare(RH);
//     // influx.prepare(pressure);
//     // boolean success = influx.write();

// }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void    printSensorDataStruct(SensorDataStruct *sds){

    Serial.println("  Id: "+String(sds->SensorId));
    Serial.println(" CRC: "+String(sds->CRC_OK));
    Serial.println("   N: "+String(sds->N));
    Serial.print(  "data: ");

    for (int i=0; i < sds->num_data_fields; i++)
        Serial.print(String(sds->meas_data[i])+" ");
    
    Serial.println();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void    chopSensorDataString(int ConnectedSensorType, String Sensor_data, SensorDataStruct *sds){

    char    buff[256]   =   {'\0'};
    int     strlen      = Sensor_data.length();

    // // initialize some fields
    // sds->N=0;
    // sds->num_data_fields=0;

    // if we reached this part then the CRC check passed
    sds->CRC_OK=1;

    // strip the Sensor_data String of STX,ETX and CRC and convert that String into char[] for sscanf
    Sensor_data.substring(1, strlen-2).toCharArray(buff, 256);

    // to make sure sscanf finds the SensorId correctly as the first string before the semicolon (;)  
    // we use %[^;] as the format specifier, meaning any string NOT containing semicolon (;)
    switch(ConnectedSensorType){
        // -- sensors with 2 measurements, but no "N"
        case ARDUINO_PPD60PV_PM:
                                    sds->num_data_fields=2;
                                    sds->N=1;
                                    sscanf(buff,"%[^;];%f;%f", sds->SensorId,  
                                                               &sds->meas_data[0], &sds->meas_data[1]);
                                    break;
        // -- sensors with 4 measurements, but no "N"
        case ARDUINO_PPD42JN_PM:
                                    sds->num_data_fields=4;
                                    sds->N=1;
                                    sscanf(buff,"%[^;];%f;%f;%f;%f", sds->SensorId,  
                                                                  &sds->meas_data[0], &sds->meas_data[1],
                                                                  &sds->meas_data[2], &sds->meas_data[3]);
                                    break;

        // -- sensors with 2 measurements
        case ARDUINO_HPMA_PM:
        case ARDUINO_SDS011_PM:
        case ARDUINO_DYLOS_PM:
        case ARDUINO_AEROQUAL_O3:
                                    sds->num_data_fields=2;
                                    sscanf(buff,"%[^;];%d;%f;%f", sds->SensorId, &sds->N, 
                                                                  &sds->meas_data[0], &sds->meas_data[1]);
                                    break;

        // -- sensors with 3 measurements
        case HUZZAH_ALPHASENSE_OPCN2:
        case ARDUINO_WINSENZH03_PM:
        case ARDUINO_CITYTECH_NO2:
        case ARDUINO_CITYTECH_O3:
        case ARDUINO_MEMBRAPOR_NO2_C1:  
        case ARDUINO_MEMBRAPOR_NO2_C20:  
        case ARDUINO_MEMBRAPOR_O3_C5:  
                                    sds->num_data_fields=3;
                                    sscanf(buff,"%[^;];%d;%f;%f;%f", sds->SensorId, &sds->N, 
                                                                  &sds->meas_data[0], &sds->meas_data[1],
                                                                  &sds->meas_data[2]);
                                    break;
        // -- sensors with 4 measurements
        case HUZZAH_BME280_SENSOR:
        case ARDUINO_ALPHASENSE_OX: 
        case ARDUINO_ALPHASENSE_NO2: 
                                    sds->num_data_fields=4;
                                    sscanf(buff,"%[^;];%d;%f;%f;%f;%f", sds->SensorId, &sds->N, 
                                                                    &sds->meas_data[0], &sds->meas_data[1],
                                                                    &sds->meas_data[2], &sds->meas_data[3]);
                                  
                                    break;
        // -- sensors with 12 measurements
        case ARDUINO_PLANTOWER7003_PM: 
                                    sds->num_data_fields=12;
                                    sscanf(buff,"%[^;];%d;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f", sds->SensorId, &sds->N, 
                                                                    &sds->meas_data[0], &sds->meas_data[1],
                                                                    &sds->meas_data[2], &sds->meas_data[3],
                                                                    &sds->meas_data[4], &sds->meas_data[5],
                                                                    &sds->meas_data[6], &sds->meas_data[7],
                                                                    &sds->meas_data[8], &sds->meas_data[9],
                                                                    &sds->meas_data[10], &sds->meas_data[11]);
                                  
                                    break;
        default:                 sds->num_data_fields=2;
                                 sscanf(buff,"%[^;];%d;%f;%f", sds->SensorId, &sds->N, &sds->meas_data[0], &sds->meas_data[1]);

    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if valid sensor data (i.e. num_data_fields > 0) and not all name
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
boolean IsValidSerialSensorData(SensorDataStruct *sds){

    int num_NAN = 0;
    boolean IsValidData = true;

    for (int i=0; i < sds->num_data_fields; i++){
        if  (sds->meas_data[i] != sds->meas_data[i]) // way to check for NAN, Inf, ...
           num_NAN++; 
    }

    if (num_NAN == sds->num_data_fields)
        IsValidData=false;

    return(IsValidData);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int sensorIdFromSerialSensorData(String Sensor_data){

    int sensorId = NO_SENSOR;

    if (Sensor_data.length() > 0 && char(Sensor_data[1]) == 'V' && char(Sensor_data[2]) == 'Q'){
        switch (char(Sensor_data[3])){

            case 'A':   sensorId = ARDUINO_SDS011_PM; //  _SIMULATOR;
                        break;
            case 'B':   sensorId = ARDUINO_PPD42JN_PM;
                        break;
            case 'C':   sensorId = ARDUINO_PPD60PV_PM;
                        break;
            case 'D':   sensorId = ARDUINO_HPMA_PM;
                        break;
            case 'E':   sensorId = ARDUINO_PLANTOWER7003_PM;
                        break;
            case 'F':   sensorId = ARDUINO_WINSENZH03_PM;
                        break;
            case 'G':   sensorId = ARDUINO_ALPHASENSE_OX;
                        break;
            case 'H':   sensorId = ARDUINO_ALPHASENSE_NO2;
                        break;
            case 'I':   sensorId = ARDUINO_CITYTECH_NO2;
                        break;
            case 'J':   sensorId = ARDUINO_CITYTECH_O3;
                        break;
            case 'K':   sensorId = ARDUINO_MEMBRAPOR_NO2_C1;
                        break;
            case 'L':   sensorId = ARDUINO_MEMBRAPOR_NO2_C20;
                        break;
            case 'M':   sensorId = ARDUINO_MEMBRAPOR_O3_C5;
                        break;
            case 'N':   sensorId = ARDUINO_DYLOS_PM;
                        break;
            case 'O':   sensorId = ARDUINO_AEROQUAL_O3;
                        break;
            default:    sensorId = UNKNOWN_SENSOR;
        }
    }

    return(sensorId);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// parseSensorDataString()
//
// Check the length and CRC of the received sensor data string, then proceed to process it further into 
// a SensorDataStruct that can be used to build and InfluxDB compatible write statement with
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SensorDataStruct parseSensorDataString(int ConnectedSensorType, String Sensor_data){

    int     strlen = Sensor_data.length();
    char    CRC, calcCRC;
    SensorDataStruct sds;

    //Sensor_data[strlen-1]=char(int(random(0, 255))); // corrupt CRC -- test

    sds.CRC_OK=0;
    sds.N=0;
    sds.num_data_fields=0;
    strncpy(sds.SensorId, "UnknownId", 256);

    if (strlen > 0){
            if (Sensor_data[0] == MAGIC_STX && Sensor_data[strlen-2] == MAGIC_ETX){
                    CRC=Sensor_data[strlen-1];
                    calcCRC=GetCheckSum(Sensor_data);   // last char = CRC and should result in 0
                                                        // but the GetCheckSum() replaces 0 by 1, so we'll test for 1
                    if (calcCRC == 1){
                        // Serial.println("CRC OK!");
                        chopSensorDataString(ConnectedSensorType, Sensor_data, &sds);
                        //printSensorDataStruct(&sds);
                    }
                    else{
                        Serial.print("CRC doesn't match "+String(CRC, HEX));
                        Serial.println(" calc:"+String(GetCheckSum(Sensor_data.substring(0, strlen-1)), HEX)+" "+String(calcCRC, HEX));
                    }
            }
            else{
                Serial.println("Sensor data has no magic? ["+Sensor_data+"]");
                Serial.println("  Found:"+String(Sensor_data[0], HEX)+" "+String(Sensor_data[strlen-2], HEX));
            }

    }
    else {
        Serial.println("Empty sensor data string?");
    }

    return(sds);

}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Ratamahatta()
//
// Function to make sure Chaos AD stays under control ;)
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Ratamahatta(){

    Serial.println("ROOTS BLOODY ROOTS!!");

}
