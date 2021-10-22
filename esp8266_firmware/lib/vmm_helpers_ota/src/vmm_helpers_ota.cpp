#include "vmm_helpers_ota.h"

void setUp_httpUpdate(){

    t_httpUpdate_return ret = ESPhttpUpdate.update(OTA_HTTP_IP, OTA_HTTP_PORT,
                                                   OTA_HTTP_URL, OTA_HTTP_CURRENTFW);
    
    switch(ret) {
        case HTTP_UPDATE_FAILED:
            Serial.println("[OTA http] Update failed.");
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("[OTA http] Update no Update.");
            break;
        case HTTP_UPDATE_OK:
            Serial.println("[OTA http] Update ok."); // may not called we reboot the ESP
            break;
    }
}

void setUp_OTA(){

  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  ArduinoOTA.setPassword("password");

  // Password can be set with it's md5 value as well
  //ArduinoOTA.setPasswordHash(VQ_OTA_PWD_MD5HASH);

  ArduinoOTA.onStart([]() {
 
    String type;
 
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("[OTA wifi] Start updating " + type);
  
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
 
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA wifi] Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
 
  ArduinoOTA.begin();

}