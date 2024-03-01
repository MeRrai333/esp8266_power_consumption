#include <Arduino.h>

#include <EEPROM.h>

char lineTokenChar[50] = "OAjta4ZQbqKeaENdEWiT6rScDMjNSiVWTMcJAOWGA71";
int lineMin = 0;

void setup() {
    Serial.begin(115200);
    EEPROM.begin(512);
    
    char url[30];
    char token[100];
    char org[30];
    char bucket[30];
    EEPROM.get(0,url);
    EEPROM.get(31,token);
    EEPROM.get(132,org);
    EEPROM.get(200,bucket);
    EEPROM.get(250, lineTokenChar);
    EEPROM.get(301, lineMin);
    EEPROM.end();
    Serial.println("URL: "+String(url));
    Serial.println("Token: "+String(token));
    Serial.println("ORG: "+String(org));
    Serial.println("Bucket: "+String(bucket));
    Serial.println("Line Token: "+String(lineTokenChar));
    Serial.println("Line notyify min.: "+String(lineMin));
}

void loop() {

}
