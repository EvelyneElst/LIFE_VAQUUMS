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
	
	Wrappers to write sensor data to an InfluxDB.
	
	.-----
	Last updated on 11-okt-2018 by DRoVMM
	
 ***************************************************************************/


#include "vmm_helpers.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Adafruit BME280 sensor
/////////////////////////////////////////////////////////////////////////////////////////////////////

InfluxData BME_SensorData_to_Influx(SensorDataStruct *sds){

    InfluxData  row("bme_sensor_data");

    row.addTag("sensor_internal_id", sds->SensorId); //i.e. VQZ1
    row.addTag("sensor_id", "AdafruitBME280");
    row.addValue("CRC_correct", sds->CRC_OK);
    row.addValue("N",   sds->N);
    row.addValue("T",   sds->meas_data[0]);
    row.addValue("p",   sds->meas_data[1]);
    row.addValue("RH",  sds->meas_data[2]);
    row.addValue("alt", sds->meas_data[3]);

    return row;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Alphasense OPC-N2 PM sensor
/////////////////////////////////////////////////////////////////////////////////////////////////////

InfluxData AlphasenseOPCN2_SensorData_to_Influx(SensorDataStruct *sds){

    InfluxData  row("alphasense_sensor_data");

    row.addTag("sensor_internal_id", sds->SensorId); //i.e. VQZ1
    row.addTag("sensor_id", "AlphasenseOPCN2");
    row.addTag("pollutant", "PM");
    row.addValue("CRC_correct", sds->CRC_OK);
    row.addValue("N",   sds->N);
    row.addValue("conc_PM1",   sds->meas_data[0]);
    row.addValue("conc_PM25",  sds->meas_data[1]);
    row.addValue("conc_PM10",  sds->meas_data[2]);

    return row;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Citytech sensors
/////////////////////////////////////////////////////////////////////////////////////////////////////

InfluxData CityTech_SensorData_to_Influx(SensorDataStruct *sds, String sensor_id, String pollutant){

    InfluxData  row("citytech_sensor_data");

    row.addTag("sensor_internal_id", sds->SensorId); //i.e. VQZ1
    row.addTag("sensor_id", sensor_id);
    row.addTag("pollutant", pollutant);
    row.addValue("CRC_correct", sds->CRC_OK);
    row.addValue("N",   sds->N);
    row.addValue("conc_mV",   sds->meas_data[0]);
    row.addValue("conc_ppb",  sds->meas_data[1]);
    row.addValue("conc_mV_stdev",  sds->meas_data[2]);

    return row;

}

InfluxData CityTechO3_SensorData_to_Influx(SensorDataStruct *sds){

    return CityTech_SensorData_to_Influx(sds, "CityTech_O3_3E1F", "O3");

}


InfluxData CityTechNO2_SensorData_to_Influx(SensorDataStruct *sds){

    return CityTech_SensorData_to_Influx(sds, "CityTech_NO2_3E50", "NO2");

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Membrapor sensors
/////////////////////////////////////////////////////////////////////////////////////////////////////

InfluxData Membrapor_SensorData_to_Influx(SensorDataStruct *sds, String sensor_id, String pollutant){

    InfluxData  row("membrapor_sensor_data");

    row.addTag("sensor_internal_id", sds->SensorId); //i.e. VQZ1
    row.addTag("sensor_id", sensor_id);
    row.addTag("pollutant", pollutant);
    row.addValue("CRC_correct", sds->CRC_OK);
    row.addValue("N",   sds->N);
    row.addValue("conc_mV",   sds->meas_data[0]);
    row.addValue("conc_ppb",  sds->meas_data[1]);
    row.addValue("conc_mV_stdev",  sds->meas_data[2]);

    return row;

}

InfluxData MembraporNO2_C1_SensorData_to_Influx(SensorDataStruct *sds){

    return Membrapor_SensorData_to_Influx(sds, "MembraporNO2_C1", "NO2");

}

InfluxData MembraporNO2_C20_SensorData_to_Influx(SensorDataStruct *sds){

    return Membrapor_SensorData_to_Influx(sds, "MembraporNO2_C20", "NO2");

}

InfluxData MembraporO3_C5_SensorData_to_Influx(SensorDataStruct *sds){

    return Membrapor_SensorData_to_Influx(sds, "MembraporO3_C5", "O3");

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Aeroqual sensors
/////////////////////////////////////////////////////////////////////////////////////////////////////

InfluxData Aeroqual_SensorData_to_Influx(SensorDataStruct *sds, String sensor_id, String pollutant){

    InfluxData  row("aeroqual_sensor_data");

    row.addTag("sensor_internal_id", sds->SensorId); //i.e. VQZ1
    row.addTag("sensor_id", sensor_id);
    row.addTag("pollutant", pollutant);
    row.addValue("CRC_correct", sds->CRC_OK);
    row.addValue("N",   sds->N);
    row.addValue("conc_ppb",   sds->meas_data[0]);
    row.addValue("sensor_status",  sds->meas_data[1]);

    return row;

}

InfluxData AeroqualO3_SensorData_to_Influx(SensorDataStruct *sds){

    return Aeroqual_SensorData_to_Influx(sds, "AeroqualO3", "O3");

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Dylos sensors
/////////////////////////////////////////////////////////////////////////////////////////////////////

InfluxData Dylos_SensorData_to_Influx(SensorDataStruct *sds, String sensor_id, String pollutant){

    InfluxData  row("dylos_sensor_data");

    row.addTag("sensor_internal_id", sds->SensorId); //i.e. VQZ1
    row.addTag("sensor_id", sensor_id);
    row.addTag("pollutant", pollutant);
    row.addValue("CRC_correct", sds->CRC_OK);
    row.addValue("N",   sds->N);
    row.addValue("conc_PM25",   sds->meas_data[0]);
    row.addValue("conc_PM10",  sds->meas_data[1]);

    return row;

}

InfluxData DylosPM_SensorData_to_Influx(SensorDataStruct *sds){

    return Dylos_SensorData_to_Influx(sds, "Dylos1700", "PM");

}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// AlphaSense sensors
/////////////////////////////////////////////////////////////////////////////////////////////////////

InfluxData AlphaSense_SensorData_to_Influx(SensorDataStruct *sds, String sensor_id, String pollutant){

    InfluxData  row("alphasense_sensor_data");

    row.addTag("sensor_internal_id", sds->SensorId); //i.e. VQZ1
    row.addTag("sensor_id", sensor_id);
    row.addTag("pollutant", pollutant);
    row.addValue("CRC_correct", sds->CRC_OK);
    row.addValue("N",   sds->N);
    row.addValue("WE_mV",    sds->meas_data[0]);
    row.addValue("Aux_mV",   sds->meas_data[1]);
    row.addValue("WE_ppb",   sds->meas_data[2]);
    row.addValue("Aux_ppb",  sds->meas_data[3]);
    row.addValue("WE_mV_stdev", sds->meas_data[4]);

    return row;

}

InfluxData AlphaSenseO3_SensorData_to_Influx(SensorDataStruct *sds){

    return AlphaSense_SensorData_to_Influx(sds, "AlphaSenseOX", "O3");

}

InfluxData AlphaSenseNO2_SensorData_to_Influx(SensorDataStruct *sds){

    return AlphaSense_SensorData_to_Influx(sds, "AlphaSenseNO2", "NO2");

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// SDS011 sensor
/////////////////////////////////////////////////////////////////////////////////////////////////////

InfluxData SDS_SensorData_to_Influx(SensorDataStruct *sds){

    InfluxData  row("sds_sensor_data");

    row.addTag("sensor_internal_id", sds->SensorId); //i.e. VQZ1
    row.addTag("sensor_id", "SDS011");
    row.addTag("pollutant",  "PM");
    row.addValue("CRC_correct", sds->CRC_OK);
    row.addValue("N",   sds->N);
    row.addValue("PM25",   sds->meas_data[0]);
    row.addValue("PM10",  sds->meas_data[1]);

    return row;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// SDS simulator
/////////////////////////////////////////////////////////////////////////////////////////////////////

InfluxData Simulator_SensorData_to_Influx(SensorDataStruct *sds){

    InfluxData  row("simulated_sensor_data");

    row.addTag("sensor_internal_id", sds->SensorId); //i.e. VQZ1
    row.addTag("sensor_id", "SDSSimulator");
    row.addTag("pollutant",  "PM");
    row.addValue("CRC_correct", sds->CRC_OK);
    row.addValue("N",   sds->N);
    row.addValue("PM25",   sds->meas_data[0]);
    row.addValue("PM10",  sds->meas_data[1]);

    return row;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// PDP42NJ PM sensor
/////////////////////////////////////////////////////////////////////////////////////////////////////
InfluxData PPD42_SensorData_to_Influx(SensorDataStruct *sds){

    InfluxData  row("ppd_sensor_data");

    row.addTag("sensor_internal_id", sds->SensorId); //i.e. VQZ1
    row.addTag("sensor_id", "PPD42NJ");
    row.addTag("pollutant",  "PM");
    row.addValue("CRC_correct", sds->CRC_OK);
    row.addValue("N_P1",      sds->meas_data[0]);
    row.addValue("ratioP1",   sds->meas_data[1]);
    row.addValue("N_P2",      sds->meas_data[2]);
    row.addValue("ratioP2",   sds->meas_data[3]);

    return row;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// PPD60PV PM sensor
/////////////////////////////////////////////////////////////////////////////////////////////////////
InfluxData PPD60_SensorData_to_Influx(SensorDataStruct *sds){

    InfluxData  row("ppd_sensor_data");

    row.addTag("sensor_internal_id", sds->SensorId); //i.e. VQZ1
    row.addTag("sensor_id", "PPD60PV");
    row.addTag("pollutant",  "PM");
    row.addValue("CRC_correct", sds->CRC_OK);
    row.addValue("N_P1",      sds->meas_data[0]);
    row.addValue("ratioP1",   sds->meas_data[1]);

    return row;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// HPMA PM sensor
/////////////////////////////////////////////////////////////////////////////////////////////////////
InfluxData HPMA_SensorData_to_Influx(SensorDataStruct *sds){

    InfluxData  row("hpma_sensor_data");

    row.addTag("sensor_internal_id", sds->SensorId); //i.e. VQZ1
    row.addTag("sensor_id", "HPMA");
    row.addTag("pollutant",  "PM");
    row.addValue("CRC_correct", sds->CRC_OK);
    row.addValue("N",      sds->N); 
    row.addValue("PM25",   sds->meas_data[0]);
    row.addValue("PM10",   sds->meas_data[1]);

    return row;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// PlantowerPMS7003 PM sensor
/////////////////////////////////////////////////////////////////////////////////////////////////////
InfluxData PMS7003_SensorData_to_Influx(SensorDataStruct *sds){

    InfluxData  row("plantower_sensor_data");

    row.addTag("sensor_internal_id", sds->SensorId); //i.e. VQZ1
    row.addTag("sensor_id", "PlantowerPMS7003");
    row.addTag("pollutant",  "PM");
    row.addValue("CRC_correct", sds->CRC_OK);
    row.addValue("N",           sds->N); 
    row.addValue("PM1",         sds->meas_data[0]);
    row.addValue("PM25",        sds->meas_data[1]);
    row.addValue("PM10",        sds->meas_data[2]);
    row.addValue("PM1atm",      sds->meas_data[3]);
    row.addValue("PM25atm",     sds->meas_data[4]);
    row.addValue("PM10atm",     sds->meas_data[5]);
    row.addValue("N_diam03",    sds->meas_data[6]);
    row.addValue("N_diam05",    sds->meas_data[7]);
    row.addValue("N_diam1",     sds->meas_data[8]);
    row.addValue("N_diam25",    sds->meas_data[9]);
    row.addValue("N_diam5",     sds->meas_data[10]);
    row.addValue("N_diam10",    sds->meas_data[11]);

    return row;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Winsen PM sensor
/////////////////////////////////////////////////////////////////////////////////////////////////////
InfluxData WinsenZH03_SensorData_to_Influx(SensorDataStruct *sds){

    InfluxData  row("winsen_sensor_data");

    row.addTag("sensor_internal_id", sds->SensorId); //i.e. VQZ1
    row.addTag("sensor_id", "WinsenZH03");
    row.addTag("pollutant",  "PM");
    row.addValue("CRC_correct", sds->CRC_OK);
    row.addValue("N",           sds->N); 
    row.addValue("PM1",         sds->meas_data[0]);
    row.addValue("PM25",        sds->meas_data[1]);
    row.addValue("PM10",        sds->meas_data[2]);

    return row;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// No sensor dummy
/////////////////////////////////////////////////////////////////////////////////////////////////////

InfluxData NoSensorData_to_Influx(){
    
    InfluxData  row("no_sensor_data");
    return row;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Main function to write data to InfluxDB
/////////////////////////////////////////////////////////////////////////////////////////////////////

InfluxData SensorData_to_Influx(int connectedSensor, SensorDataStruct *sds){

    switch(connectedSensor){
        case HUZZAH_BME280_SENSOR:      return(BME_SensorData_to_Influx(sds));
        case HUZZAH_ALPHASENSE_OPCN2:   return(AlphasenseOPCN2_SensorData_to_Influx(sds));
        //--- GAS
        case ARDUINO_CITYTECH_NO2:      return(CityTechNO2_SensorData_to_Influx(sds));
        case ARDUINO_CITYTECH_O3:       return(CityTechO3_SensorData_to_Influx(sds));
        case ARDUINO_MEMBRAPOR_NO2_C1:  return(MembraporNO2_C1_SensorData_to_Influx(sds));
        case ARDUINO_MEMBRAPOR_NO2_C20: return(MembraporNO2_C20_SensorData_to_Influx(sds));
        case ARDUINO_MEMBRAPOR_O3_C5:   return(MembraporO3_C5_SensorData_to_Influx(sds));
        case ARDUINO_ALPHASENSE_OX:     return(AlphaSenseO3_SensorData_to_Influx(sds));
        case ARDUINO_ALPHASENSE_NO2:    return(AlphaSenseNO2_SensorData_to_Influx(sds));
        case ARDUINO_AEROQUAL_O3:       return(AeroqualO3_SensorData_to_Influx(sds));
        //--- PM
        case ARDUINO_DYLOS_PM:          return(DylosPM_SensorData_to_Influx(sds));
        case ARDUINO_SDS011_PM:         return(SDS_SensorData_to_Influx(sds));
        case ARDUINO_PPD42JN_PM:        return(PPD42_SensorData_to_Influx(sds));
        case ARDUINO_PPD60PV_PM:        return(PPD60_SensorData_to_Influx(sds));
        case ARDUINO_HPMA_PM:           return(HPMA_SensorData_to_Influx(sds));
        case ARDUINO_PLANTOWER7003_PM:  return(PMS7003_SensorData_to_Influx(sds));
        case ARDUINO_WINSENZH03_PM:     return(WinsenZH03_SensorData_to_Influx(sds));

        case ARDUINO_SDS_SIMULATOR:     return(Simulator_SensorData_to_Influx(sds));
        default:                        return(NoSensorData_to_Influx());

    }

    return(NoSensorData_to_Influx());
}


