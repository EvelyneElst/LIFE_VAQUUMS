<img src="https://vaquums.eu/@@site-logo/life_vaquums_eu.png" alt="VAQUUMS project website" title="VAQUUMS project website">

# Field campaign dataset

Here are the complete datasets for the entire Life VAQUUMS field campaign of the low-cost sensors. This campaign was done at the official Flanders Environment Agency (VMM) monitoring station R801 in Borgerhout, Antwerp, Belgium (WGS84 coordinates: `51.2097 lat`, `4.4318 lon`).

We divided the data into two distinct, but complimentary, dataset folders:
* `monitor` contains all the official measurements, i.e. the reference (true) values
* `sensors` contains all the sensor measurements in fivefold, along with additional quality control parameters.

All the data is stored in CSV files in wide format.

# Detailed description of the field campaign dataset

Each sensor model was given a unique letter code (UniqueLetter) prefixed by `VQ` and since each sensor was tested in fivefold it is followed by digits 1-5, i.e. the five SDS011 sensors have codes VQA1 to VQA5. 

| UniqueLetter | Sensormodel         | Pollutant  |
|--------------|---------------------|------------|
| A            | Nova Fitness SDS011 | PM         |
| B            | Shinyei PPD42NJ     | PM         |
| C            | Shinyei PPD60PV-T2  | PM         |
| D            | Honeywell HPMA115S0 | PM         |
| E            | Plantower PMS7003   | PM         |
| F            | Winsen ZH03B        | PM         |
| G            | Alphasense OX-B431  | O3+NOx     |
| H            | Alphasense NO2-B43F | NO2        |
| I            | Citytech NO2 3E50   | NO2        |
| J            | Citytech O3 3E1F    | O3         |
| K            | Membrapor NO2-C1    | NO2        |
| L            | Membrapor NO2-C20   | NO2        |
| M            | Membrapor O3-C5     | O3         |
| N            | Dylos DC1700        | PM         |
| O            | Aeroqual O3 0-0.15  | O3         |
| S            | Envea CairClip NO2  | NO2, RH, T |
| T            | Envea CairClip O3   | O3, RH, T  |
| Y            | *Alphasense OPC-N2* | PM         |
| Z            | *BME280 DHT sensor* | RH,T       |

> Due to technical difficulties the Alphasense OPC-N2 measurements were not usable and this dataset is not available. The BME280 dataset is currently under investigation and is likely to be published later.

The sensor measurements were aggregated to 1-minute mean values. In the column prefixed by `N` the number of individual readings (i.e. polling or sampling rate) that make up the mean is shown, i.e. for sensors that send data every 1s the column N will have a value close to 60. Depending on the sensor model the polling frequency can be higher or lower that once every 1s.

The 1-minute timestamps (`time`) are expressed as epochs (s precision) with the origin at 1970-01-01 00:00. They span the entire duration of the field campaign starting at 1550880000 (`2019-02-23 00:00 UTC`) and ending at 1585612800 (`2020-03-31 00:00 UTC`).

Note about the Envea CairClip sensors:
* to save battery life the time resolution was set to 15 minutes
* these sensors were not validated (cf. no `sc` codes), measurement are provided as-is
* these sensors also measure relative humidity (RH in %) and temperature (T in °C)

### *Calibration*

The sensor data is not calibrated in any way. The data represents the raw readings and the concentrations for gaseous pollutants are expressed in ppb while for particulate matter (PM) they are in µg/m³-equivalents. One exception for PM is the Shinyei PPD60PV which reports in internal concentration units that need to be converted to µg/m³ using a suitable (calibration) function. More details on the function we used can be found in the reports mentioned under the [Further reading](#further-reading) section below.

## Status codes

Sensor measurements passed a quality control based on expert judgement (so a human being, not a machine learning or artificial intellegence algorithm) comparing the fivefold measurements among eachother and also to the official measurements.

The sensor measurements were accordingly tagged/labeled in the `sc` column with the following possible values:

| sc code value           | meaning    |
|-------------------------|------------|
| NA (not available) or 0 | OK         |
| -1                      | suspicious |
| -2                      | invalid    |

By default every sensor reading was given an NA tag in the `sc` column. So, when not explicitely tagged as -1 or -2 the measurement can be considered as suitable for further analysis.

## Official measurements

For the official measurements the Flanders Environment Agency (VMM) used the following equipment:

| Instrument              | Pollutant |
|-------------------------|-----------|
| Thermo Scientific 42i   | NOx       |
| API T400                | O3        |
| Palas Fidas 200         | PM        |
| Palas Fidas 200         | RH, T     |

All of the measurements for the gaseous pollutants (NOx, O3) are expressed in ppb while for particulate matter (PM) it is µg/m³.

The relative humidity (RH) and to a lesser extent temperature (T) measurements are included in order to investigate possible correction algorithms.

### *Calibration*

The sensor measurements are not calibrated in any way, they represent the original signal that was aggregated (mean) from the original time resolution (often 1s) to 1 minute.

For the official measurement equipment the gas monitors are frequently calibrated with internal standards. For the PM monitor the PM10 results were compared with the gravimetric reference method and an additive calibration factor of 2,6 µg/m³ was determined. This calibration is already taken into account.

## Time convention

Everything is expressed in UTC and the timestamps indicate the `starting time`, i.e. 2019-02-23 00:00 refers to all 1 minute measurements between 2019-02-23 00:00 and 2019-02-23 00:01.

## Further reading

Detailed reports containing analysis of this datasets can be found at the [VAQUUMS website](https://vaquums.eu/test-results) along with fact sheets and other guides.

## License

This dataset is avalaible under
<a rel="license" href="http://creativecommons.org/licenses/by-nc/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc/4.0/80x15.png" /></a><br /><span xmlns:dct="http://purl.org/dc/terms/" href="http://purl.org/dc/dcmitype/Dataset" property="dct:title" rel="dct:type">LIFE VAQUUMS: Low-cost sensors field campaign dataset</span> by <span xmlns:cc="http://creativecommons.org/ns#" property="cc:attributionName">Flanders Environment Agency (VMM)</span> is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc/4.0/">Creative Commons Attribution-NonCommercial 4.0 International License</a>.<br />Based on a work at <a xmlns:dct="http://purl.org/dc/terms/" href="https://vaquums.eu" rel="dct:source">https://vaquums.eu</a>.

