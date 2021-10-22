/**************************************************************************************
 * vmm_helpers_influx.h 
 * 
 * Header file containing all relevant data for vmm_helpers_influx.cpp 
 * in order to improve maximum re-usability of the code.
 * 
 * Last updated on 12-sep-2018 by DRoVMM
 *
 **************************************************************************************/

#ifndef __VMM_INFLUX_H__
#define __VMM_INFLUX_H__

InfluxData  SensorData_to_Influx(int connectedSensor, SensorDataStruct *sds);

#endif