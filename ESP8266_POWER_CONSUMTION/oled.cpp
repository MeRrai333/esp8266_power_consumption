#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
/* ==== OLED ==== */
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(128, 32, &Wire, -1);
byte oledDisplayStep = 0;
/* ==== OLED ==== */
void drawCentreString(String text, int x, int y);
void oledDisplay();
void oledDetail();
void oledDisplayFormat(String head, String val);
unsigned long millisOLED = 0, millisOLEDStep = 0;

#define BTN D7

void setup() {
    Serial.begin(115200);
    pinMode(BTN, INPUT_PULLUP);
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.clearDisplay();

}

void loop() {
    if(millis() - millisOLED > 250){
        display.clearDisplay();
        oledDisplay();
        millisOLED = millis();
    }
    if(millis() - millisOLEDStep > 5000){
        (oledDisplayStep + 1 >= 6) ? oledDisplayStep = 0 : oledDisplayStep++;
        millisOLEDStep = millis();
    }

    if(!digitalRead(BTN)){
        unsigned long millisBTN = millis();
        while(!digitalRead(BTN)){
            if(millis() - millisBTN > 2000){
                display.clearDisplay();
                oledDetail();
            }
            yield();
        }
        if(millis() - millisBTN > 2000)
            oledDisplayStep = 7;
        else
            (oledDisplayStep + 1 >= 6) ? oledDisplayStep = 0 : oledDisplayStep++;
        millisOLEDStep = millis();
    }
}

void oledDetail(){
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setFont(&FreeMono9pt7b);
    display.setCursor(5, 10);
    display.print("WiFi: YES");
    display.setCursor(5, 25);
    display.print("IFDB: NO");
    display.display();
}

void oledDisplay(){
    if(oledDisplayStep == 0){
        oledDisplayFormat("Voltage", String(231.12)+"V");
    }
    else if(oledDisplayStep == 1){
        oledDisplayFormat("Current", String(0.4)+"A");
    }
    else if(oledDisplayStep == 2){
        oledDisplayFormat("Power", String(19.0)+"W");
    }
    else if(oledDisplayStep == 3){
        oledDisplayFormat("Energy", String(0.0)+"Wh");
    }
    else if(oledDisplayStep == 4){
        oledDisplayFormat("Frequency", String(50.0)+"Hz");
    }
    else if(oledDisplayStep == 5){
        oledDisplayFormat("PowerFactor", String(0.8));
    }
    else if(oledDisplayStep == 7){
        oledDetail();
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

/*
    === Connecting WiFI ===
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(15, 10);
    display.print("Connecting");
    display.setCursor(40, 30);
    display.print("WiFi");
    display.display();
}
*/