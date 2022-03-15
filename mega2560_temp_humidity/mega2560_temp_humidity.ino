/*
 * Arduino Mega 2560 with 
 * - Sensor DHT22 
 * - Touchscreen 2.4Zoll 240x320, ILI931, SKUMAR2406
 */
 
#include "DHT.h" 
#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_KBV.h> //Hardware-specific library
 // include the SD library:
#include <SPI.h>
#include <SD.h>
#include <stdlib.h>


#define DHTGND 42
#define DHTPIN 44   // free PWM port bei Mega2560 with inserted Touchscreen hat
#define DHTVCC 46
#define DHTTYPE DHT22    // DHT11 as aternative when that sensor is used instead of DHT22

#define CS_PIN 53 // CS PIN FOR SPI

/* 
 *  Pianos work best and sound best when the temperature and humidity are right. 
 *  Proper ventilation is also important. 
 *  Generally speaking, a relative humidity around 45 percent is ideal for pianos. 
 *  The use of materials such as wood, felt and cloth in piano construction means that many parts are quite delicate.
 *  
 *  When a cold room is warmed suddenly, moisture will condense on the piano strings and other metal parts, 
 *  causing them to rust. Felt parts will absorb moisture, dulling their action and resulting in unclear sound. 
 *  Be especially careful about sudden temperature changes when moving your piano into a room in a cold northern 
 *  climate or into an airtight room in a concrete building. Do not place objects on top of the piano.
 *  
 *  Source: https://ca.yamaha.com/en/support/caring_for_your_piano/index.html
*/
#define MIN_HUMIDITY_KLAVIER 40
#define MAX_HUMIDITY_KLAVIER 60

/*
 * Text size for display.
 * With size 6 in one row can be displayed 6 characters.
 */
#define TEXT_SIZE 5
#define TEXT_SIZE_WARNING 6

DHT dht(DHTPIN, DHTTYPE);

//if the IC model is known or the modules is unreadable,you can use this constructed function
LCDWIKI_KBV mylcd(ILI9341,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset

float prev_humidity = 0.0;
float prev_temperature = 0.0;

// coordinate for info/warning text line.
int rowInfo = 170; 

// variables for SD card log
boolean cardInit = false;
char buff[10];
long passedSeconds = 0;
  
/*
 * Colors for Display 
 */
#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define MAGENTA 0xF81F
#define WHITE   0xFFFF

void updateScreen(float humidity, float temperature) {

  // print temperature value
  int row0 = 10;
    
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_Size(TEXT_SIZE);

  mylcd.Print_Number_Float(temperature, 2, 0, row0, '.', 6, ' ');  // 2: to digits, x, y, dec, length, filler
  mylcd.Draw_Char(190, row0-5,'o', WHITE, BLACK, TEXT_SIZE-2, 0);
  mylcd.Draw_Char(190+15, row0,'C', WHITE, BLACK, TEXT_SIZE, 0); 

  // print humidity warning
  mylcd.Set_Text_Size(TEXT_SIZE_WARNING);
  if (humidity >= MAX_HUMIDITY_KLAVIER) {
    mylcd.Set_Text_colour(RED);
    mylcd.Print_String("ABLAK!", RIGHT, rowInfo); 
  }else if (humidity <= MIN_HUMIDITY_KLAVIER) {
    mylcd.Set_Text_colour(MAGENTA);
    mylcd.Print_String("SZARAZ", RIGHT, rowInfo);
  }else {
    mylcd.Set_Text_colour(GREEN);
    mylcd.Print_String("------", RIGHT, rowInfo); 
  }

  // print humidity value
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_Size(TEXT_SIZE);
  int row1 = row0 + TEXT_SIZE * 12;
  mylcd.Print_Number_Float(humidity, 2, 0, row1, '.', 6, ' ');  // 2: to digits, x, y, dec, length, filler
  mylcd.Draw_Char(190, row1,'%', WHITE, BLACK, TEXT_SIZE, 0); //x,y char, color, backgound, size, mode
}

void logToCard(float humidity, float temperature) {
      if (cardInit) {
        File datafile = SD.open("datalog.txt", FILE_WRITE);
        if (datafile) {
          delay(200); // to see the circle!
          datafile.write(' ');
          dtostrf(humidity, 5, 2, buff);
          datafile.write(buff,5);
          datafile.write(' ');
          dtostrf(temperature, 5, 2, buff);
          datafile.write(buff,5);
          datafile.println();
          datafile.close();
        }
    }
}

void setup() {
  pinMode(DHTGND, OUTPUT);
  digitalWrite(DHTGND, LOW);
  pinMode(DHTVCC, OUTPUT);
  digitalWrite(DHTVCC, HIGH);
    
  dht.begin();

  mylcd.Init_LCD();
  mylcd.Fill_Screen(BLACK);
  mylcd.Set_Text_Mode(0); // 0 - no overlap, 1 - overlap

  // SDCard initialisation
  pinMode(CS_PIN, OUTPUT);
  cardInit = SD.begin(CS_PIN);
  if (cardInit) {
    // show icon for card
    mylcd.Set_Draw_color(0,0,255);
    mylcd.Fill_Round_Rectangle(10,280,30,300,5);
  }
}

void loop() {
  // Wait a 5 seconds between measurements.
  delay(5000);                     // DHT22 needs minimum 2 seconds for measurements.
  passedSeconds = passedSeconds + 5;
                                    
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
 
  if (isnan(humidity) || isnan(temperature)) {       
    mylcd.Set_Text_colour(RED);
    mylcd.Set_Text_Size(TEXT_SIZE_WARNING);
    mylcd.Print_String("*HIBA*", 0, rowInfo);
    return;
  }

  mylcd.Set_Draw_color(0,255,0);
  mylcd.Fill_Circle(20,290,(int)passedSeconds/10);
    
  if (passedSeconds >= 60) {
    passedSeconds = 0;
    mylcd.Set_Draw_color(255,0,0);
    mylcd.Fill_Circle(20,290,6);
    logToCard(humidity, temperature);
    mylcd.Set_Draw_color(0,0,255);
    mylcd.Fill_Circle(20,290,6);
  }

  if (abs(humidity - prev_humidity) >= 0.1 ||
      abs(temperature - prev_temperature) >= 0.1
  ) {
    updateScreen(humidity, temperature);
    prev_humidity = humidity;
    prev_temperature = temperature;
  }
}
