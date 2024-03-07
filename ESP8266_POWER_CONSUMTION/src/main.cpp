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
#include <Fonts/FreeMono12pt7b.h>

/* ==== LINE ==== */
#define LINEURL "https://notify-api.line.me/api/notify"
#define LINETOKEN "Yours line token"
char lineTokenChar[50] = "Yours line token";
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
void oledDisplayFormat(String head, String val);
void drawCentreString(String text, int x, int y);
void influxdbSendFunc();
void lineNotify();
/* ==== Func. prototype ==== */
/* ==== State Machine ==== */
#define WIFICONNECTIONSTATE 0
#define INFLUXCONNECTIONSTATE 1
#define OLEDDISPLAY 2
#define PZEM004STATE 3
#define LINENOTISTATE 4
#define INFLUXDBSENDSTATE 5
#define INFLUXRECONSTATE 6

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
    display.setRotation(2);
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
            (oledDisplayStep + 1 >= 6) ? oledDisplayStep = 0 : oledDisplayStep++;
            millisOledStep = millis();
        }
        if(!digitalRead(BTN)){
            unsigned long millisBTN = millis();
            while(!digitalRead(BTN)){
                if(millis() - millisBTN > 10000){
                    if(millis() - millisOled > OLEDREFRESH){
                        display.clearDisplay();
                        display.setFont(&FreeMono9pt7b);
                        display.setTextSize(1);
                        display.setTextColor(WHITE);
                        display.setCursor(0, 10);
                        display.print("Reset");
                        display.setCursor(0, 30);
                        display.print("Act. Energy");
                        display.display();
                        pzem.resetEnergy();
                        millisOled = millis();
                    }
                }
                yield();
            }
            (oledDisplayStep + 1 >= 6) ? oledDisplayStep = 0 : oledDisplayStep++;
            Serial.println(oledDisplayStep);
            millisOledStep = millis();
        }
        if(millis() - millisPzem > PZEMDELAY)
            state = PZEM004STATE;
    }
    else if(state == PZEM004STATE){
        pzem004StateFunc();
        if(WiFi.status() == WL_CONNECTED)
            state = LINENOTISTATE;
        else
            state = INFLUXCONNECTIONSTATE;
        millisPzem = millis();
    }
    else if(state == LINENOTISTATE){
        if(millis() - millisLineTemp >= millisLine){
            lineNotify();
            millisLineTemp = millis();
        }
        if(client.validateConnection())
            state = INFLUXDBSENDSTATE;
        else
            state = INFLUXRECONSTATE;
    }
    else if(state == INFLUXDBSENDSTATE){
        influxdbSendFunc();
        state = OLEDDISPLAY;
    }
    else if(state == INFLUXRECONSTATE){
        if(!client.validateConnection()){
            if(millis() - millisOled > OLEDREFRESH){
                display.clearDisplay();
                display.setFont(&FreeMono9pt7b);
                display.setTextSize(1);
                display.setTextColor(WHITE);
                display.setCursor(0, 10);
                display.print("WiFi/IFDB");
                display.setCursor(0, 30);
                display.print("Lost Reconn.");
                display.display();
                millisOled = millis();
            }
        }
        else{
            std::unique_ptr<BearSSL::WiFiClientSecure>clientLine(new BearSSL::WiFiClientSecure);
            clientLine->setInsecure();
            HTTPClient https;
            https.begin(*clientLine, LINEURL);
            https.addHeader("Authorization","Bearer "+String(lineTokenChar));
            https.addHeader("Content-Type", "application/x-www-form-urlencoded");
            String httpData = "message=ESP8266%20PZEM004T%20Online";
            int resCode = https.POST(httpData);
            Serial.println("Line noty Res. code: "+String(resCode));
            state = OLEDDISPLAY;
        }
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
        oledDisplayFormat("Voltage", String(volt)+"V");
    }
    else if(oledDisplayStep == 1){
        oledDisplayFormat("Current", String(curr)+"A");
    }
    else if(oledDisplayStep == 2){
        oledDisplayFormat("Power", String(powe)+"W");
    }
    else if(oledDisplayStep == 3){
        oledDisplayFormat("Energy", String(ener)+"kWh");
    }
    else if(oledDisplayStep == 4){
        oledDisplayFormat("Frequency", String(freq)+"Hz");
    }
    else if(oledDisplayStep == 5){
        oledDisplayFormat("PowerFactor", String(pf));
    }
}

void oledDisplayFormat(String head, String val){
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(5, 10);
    display.setFont(&FreeMono9pt7b);
    display.print(head);
    display.setFont(&FreeMono12pt7b);
    drawCentreString(val, 64,30);
    display.display();
}

void drawCentreString(String text, int x, int y){
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(text.c_str(), 0, y, &x1, &y1, &w, &h); //calc width of new string
    display.setCursor(x - w / 2, y);
    display.print(text);
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
    std::unique_ptr<BearSSL::WiFiClientSecure>clientLine(new BearSSL::WiFiClientSecure);
    clientLine->setInsecure();
    HTTPClient https;
    https.begin(*clientLine, LINEURL);
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
    EEPROM.begin(512);
    WiFiManager wifiManager;
    bool reset = false;
    display.setFont(&FreeMono9pt7b);
    if(!digitalRead(BTN)){
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 10);
        display.print("Reset WiFi");
        display.display();
        while(!digitalRead(BTN))
            yield();
        wifiManager.resetSettings();
        reset = true;
    }
    else{
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 10);
        display.print("Connecting WiFi");
        display.display();
    }
    if(!wifiManager.autoConnect("AutoConnectAP", "ronin")){
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 10);
        display.print("Setup WiFi");
        display.display();
        reset = true;
    }
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
    millisLine = lineMin*60000;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print("Connecting");
    display.setCursor(0, 30);
    display.print("InfluxDB");
    display.display();
    Serial.println("URL: "+String(url));
    Serial.println("Token: "+String(token));
    Serial.println("ORG: "+String(org));
    Serial.println("Bucket: "+String(bucket));
    Serial.println("Line Token: "+String(lineTokenChar));
    Serial.println("Line notyify min.: "+String(lineMin));
    Serial.println("milisLine: "+String(millisLine));
    std::unique_ptr<BearSSL::WiFiClientSecure>clientLine(new BearSSL::WiFiClientSecure);
    clientLine->setInsecure();
    HTTPClient https;
    https.begin(*clientLine, LINEURL);
    https.addHeader("Authorization","Bearer "+String(lineTokenChar));
    https.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpData = "message=ESP8266%20PZEM004T%20Online";
    Serial.println(httpData);
    int resCode = https.POST(httpData);
    Serial.println("Line noty Res. code: "+String(resCode));

    client.setConnectionParams(String(url), String(org), String(bucket), String(token));
    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
    if (client.validateConnection()) {
      Serial.print("Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());
      sensor.addTag("device", DEVICENAME);
      sensor.addTag("SSID", WiFi.SSID());
      EEPROM.end();
    } else {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
    }
}
