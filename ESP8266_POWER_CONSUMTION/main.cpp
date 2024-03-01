#include <Arduino.h>

#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h>

/* ==== LINE ==== */
#define LINEURL "https://notify-api.line.me/api/notify"
#define LINETOKEN "OAjta4ZQbqKeaENdEWiT6rScDMjNSiVWTMcJAOWGA71"
char lineTokenChar[50] = "OAjta4ZQbqKeaENdEWiT6rScDMjNSiVWTMcJAOWGA71";
int lineMin = 0;
unsigned long millisLine = 0, millisLineTemp = 0;
/* ==== LINE ==== */
/* ==== INFLUXDB ==== */
#define INFLUXDB_URL "http://192.168.199.66:8086"
#define INFLUXDB_TOKEN "R5tnIiYABNRmxup-bXJTnggb1Edoik3U3gkokfZajTL9vSJ2DK-miKv83Ze2-Kd_C2mGjOnf1SE96yQ7GHUvFQ=="
#define INFLUXDB_ORG "37451d768e2f6f4e"
#define INFLUXDB_BUCKET "pzem400t"
#define TZ_INFO "UTC7"
InfluxDBClient client;
Point sensor("pzem004t");
#define DEVICENAME "ESP8266"
bool isConnectInflux = false;
/* ==== INFLUXDB ==== */
/* ==== PZEM ==== */
#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN D6
#define PZEM_TX_PIN D5
#endif

#define PZEMDELAY 1000

SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
PZEM004Tv30 pzem(pzemSWSerial);

#define OLEDREFRESH 250
#define OLEDSTEPDURATION 5000

byte pzemAddre = 0;
float volt = 0.00, curr = 0.00, powe = 0.00, ener = 0.00, freq = 0.00, pf = 0.00;
/* ==== PZEM ==== */
/* ==== OLED ==== */
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(128, 32, &Wire, -1);
byte oledDisplayStep = 0;
/* ==== OLED ==== */
/* ==== Func. prototype ==== */
void wifiConnection();
void influxConnection();
void pzem004StateFunc();
void oledDisplay();
void oledDisplayFormat(String line1, String line2);
void influxdbSendFunc();
void lineNotify();
void connectStatus();
/* ==== Func. prototype ==== */
/* ==== State Machine ==== */
#define WIFICONNECTIONSTATE 0
#define INFLUXCONNECTIONSTATE 1
#define SERVERCONNEcTION 2
#define OLEDDISPLAY 3
#define PZEM004STATE 4
#define LINENOTISTATE 5
#define INFLUXDBSENDSTATE 6

byte state;
/* ==== State Machine ==== */

#define BTN D7

unsigned long millisPzem = 0, millisOled = 0, millisOledStep = 0;

void setup() {
    Serial.begin(115200);
    pinMode(BTN, INPUT_PULLUP);
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.clearDisplay();
    display.setFont(&FreeMono9pt7b);
    state = WIFICONNECTIONSTATE;
    millisPzem = millis();
    millisOled = millis();
    millisOledStep = millis();
}

void loop() {
    if(state == WIFICONNECTIONSTATE){
        wifiConnection();
        state = INFLUXCONNECTIONSTATE;
    }
    else if(state == INFLUXCONNECTIONSTATE){
        influxConnection();
        state = OLEDDISPLAY;
    }
    else if(state == OLEDDISPLAY){
        if(millis() - millisOled > OLEDREFRESH){
            display.clearDisplay();
            oledDisplay();
            millisOled = millis();
        }
        if(millis() - millisOledStep > OLEDSTEPDURATION){
            oledDisplayStep = (oledDisplayStep+1 > 2) ? 0 : oledDisplayStep+1;
            millisOledStep = millis();
        }
        state = PZEM004STATE;
    }
    else if(state == PZEM004STATE){
        if(millis() - millisPzem > PZEMDELAY){
            pzem004StateFunc();
            state = LINENOTISTATE;
        }
        else
            state = OLEDDISPLAY;
    }
    else if(state == LINENOTISTATE){
        if(millis() - millisLineTemp >= millisLine){
            lineNotify();
            millisLineTemp = millis();
        }
        if(isConnectInflux)
            state = INFLUXDBSENDSTATE;
        else
            state = INFLUXCONNECTIONSTATE;
    }
    else if(state == INFLUXDBSENDSTATE){
        influxdbSendFunc();
        state = OLEDDISPLAY;
        millisPzem = millis();
    }
}

void influxdbSendFunc(){
    sensor.clearFields();
    sensor.addField("voltage", volt);
    sensor.addField("current", curr);
    sensor.addField("power", powe);
    sensor.addField("energy", ener);
    sensor.addField("frequency", freq);
    sensor.addField("pf", pf);
    if (!client.writePoint(sensor)) {
        Serial.print("InfluxDB write failed: ");
        Serial.println(client.getLastErrorMessage());
    }
}

void oledDisplay(){
    if(oledDisplayStep == 0){
        oledDisplayFormat("Volt "+String(volt,1)+"V", "Curr "+String(curr)+"A");
    }
    else if(oledDisplayStep == 1){
        oledDisplayFormat("Pow. "+String(powe)+"W" ,"Ene. "+String(ener)+"kWh");
    }
    else if(oledDisplayStep == 2){
        oledDisplayFormat("Fre. "+String(freq,1)+"Hz", "PF   "+String(pf));   
    }
    else{
        oledDisplayStep = 0;
    }
}
void oledDisplayFormat(String line1, String line2){
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print(line1);
    display.setCursor(0,25);
    display.print(line2);
    display.display();
}

void pzem004StateFunc(){
    pzemAddre = pzem.readAddress();
    volt = (pzemAddre != 0) ? pzem.voltage() : 0;
    curr = (pzemAddre != 0) ? pzem.current() : 0;
    powe = (pzemAddre != 0) ? pzem.power() : 0;
    ener = (pzemAddre != 0) ? pzem.energy() : 0;
    freq = (pzemAddre != 0) ? pzem.frequency() : 0;
    pf = (pzemAddre != 0) ? pzem.pf() : 0;
}

void lineNotify(){
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient https;
    https.begin(*client, LINEURL);
    https.addHeader("Authorization","Bearer "+String(lineTokenChar));
    https.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String msg = "%0AVoltage:%20"+String(volt)+"%20V";
    msg += "%0ACurrent:%20"+String(curr)+"%20"+"A";
    msg += "%0APower:%20"+String(powe)+"%20W";
    msg += "%0AEnergy:%20"+String(ener)+"%20Wh";
    msg += "%0AFrequency:%20"+String(freq)+"%20Hz";
    msg += "%0APF:%20"+String(pf);
    String httpData = "message="+msg;
    Serial.println(httpData);
    int resCode = https.POST(httpData);
    Serial.println("Line noty Res. code: "+String(resCode));
}

void wifiConnection(){
    EEPROM.begin(1024);
    WiFiManager wifiManager;
    bool reset = false;
    if(!digitalRead(BTN)){
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0, 10);
        display.print("Setup WiFi");
        display.display();
        while(!digitalRead(BTN))
            yield();
        wifiManager.resetSettings();
        reset = true;
    }
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print("Connecting WiFi");
    display.display();
    wifiManager.autoConnect("AutoConnectAP", "ronin");
    WiFiManagerParameter influxDBURL("influxurl", "InfluxDB URL", INFLUXDB_URL, 30);
    WiFiManagerParameter influxDBToken("influxtoken", "InfluxDB Token", INFLUXDB_TOKEN, 100);
    WiFiManagerParameter influxDBOrg("influxorg", "InfluxDB ORG", INFLUXDB_ORG, 30);
    WiFiManagerParameter influxDBBucket("influxbucket", "InfluxDB Bucket", INFLUXDB_BUCKET, 30);
    WiFiManagerParameter lineToken("lineToken", "Line Token", LINETOKEN, 50);
    WiFiManagerParameter lineNoti("lineNoti","Line Interval","15",3);
    wifiManager.addParameter(&influxDBURL);
    wifiManager.addParameter(&influxDBToken);
    wifiManager.addParameter(&influxDBOrg);
    wifiManager.addParameter(&influxDBBucket);
    wifiManager.addParameter(&lineToken);
    wifiManager.addParameter(&lineNoti);
    wifiManager.autoConnect("IoT_G8");
    if(reset){
        char temp[100];
        strcpy(temp, influxDBURL.getValue());
        EEPROM.put(0, temp);
        strcpy(temp, influxDBToken.getValue());
        EEPROM.put(31, temp);
        strcpy(temp, influxDBOrg.getValue());
        EEPROM.put(132, temp);
        strcpy(temp, influxDBBucket.getValue());
        EEPROM.put(200, temp);
        strcpy(temp, lineToken.getValue());
        EEPROM.put(250, temp);
        strcpy(temp, lineNoti.getValue());
        int num = atoi(temp);
        EEPROM.put(301, num);
        delay(10);
        EEPROM.commit();
    }
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient https;
    https.begin(*client, LINEURL);
    https.addHeader("Authorization","Bearer "+String(lineTokenChar));
    https.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpData = "message=ESP8266%20PZEM004T%20Online";
    Serial.println(httpData);
    int resCode = https.POST(httpData);
    Serial.println("Line noty Res. code: "+String(resCode));
}

void influxConnection(){
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
    millisLine = lineMin*60000;
    Serial.println("URL: "+String(url));
    Serial.println("Token: "+String(token));
    Serial.println("ORG: "+String(org));
    Serial.println("Bucket: "+String(bucket));
    Serial.println("Line Token: "+String(lineTokenChar));
    Serial.println("Line notyify min.: "+String(lineMin));
    Serial.println("milisLine: "+String(millisLine));
    client.setConnectionParams(String(url), String(org), String(bucket), String(token));
    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
    if (client.validateConnection()) {
      Serial.print("Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());
      sensor.addTag("device", DEVICENAME);
      sensor.addTag("SSID", WiFi.SSID());
      isConnectInflux = true;
    } else {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
    }
}
