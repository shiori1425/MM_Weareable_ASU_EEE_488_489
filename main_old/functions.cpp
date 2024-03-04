#include "DummyTouchSubscriber.h"
#include "TouchSubscriberInterface.h"

#define TOUCH_MODULES_CST_SELF        //Use CST816 by default

#include "functions.h"

#define PIXEL_WIDTH 320
#define PIXEL_HEIGHT 170

/* Setup I2C bus connections*/
TwoWire SensorsI2C = TwoWire(1); // Create another I2C bus instance for sensor reads

int32_t width, height, left_center, right_center;

TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);
TFT_eSprite spr_base = TFT_eSprite(&tft);

/* Temp/Humidity Sensor Setup*/
Adafruit_HTU31D htu_ext = Adafruit_HTU31D();
Adafruit_HTU31D htu_int = Adafruit_HTU31D();
bool heaterEnabled = false;
sensors_event_t humidity, temp;

CST816Touch oTouch;

unsigned long longPressDuration = 3000;  // 3 seconds
unsigned long touchStartTime = 0;
bool isTouching = false;
bool isLongPress = false;

float bat_volt = 3.7;
uint16_t* iconData;  // Cast to const uint16_t* 
unsigned long update_bat_interval = 600000; // 2 minutes in milliseconds

bool DEBUG_MODE = true;  // Change this to false to disable debug outputs

void debugPrint(const char* message) {
    if (DEBUG_MODE) {
        Serial.println(message);
    }
}

float calcSkinRes(){
    // implement code later to read skin res
    float rand = random(800000,1200000);
    return(rand);
}

float calcSweatRate(){
    // implement code later to read skin res
    return(random(0.9,8.7));
}

const char* formatResistance(float resistance, char* buffer) {
    char format[10]; // Buffer for the format string
    int decimalPlaces;

    if (resistance >= 1e9) {
        decimalPlaces = (resistance >= 10e9) ? 1 : 2; // Adjust for 10+ Giga
        sprintf(format, "%%.%dfG", decimalPlaces);
        sprintf(buffer, format, resistance / 1e9);
    } else if (resistance >= 1e6) {
        decimalPlaces = (resistance >= 10e6) ? 1 : (resistance >= 1e6) ? 2 : 3; // Adjust for Mega
        sprintf(format, "%%.%dfM", decimalPlaces);
        sprintf(buffer, format, resistance / 1e6);
    } else if (resistance >= 1e3) {
        decimalPlaces = (resistance >= 10e3) ? 1 : (resistance >= 1e3) ? 2 : 3; // Adjust for Kilo
        sprintf(format, "%%.%dfK", decimalPlaces);
        sprintf(buffer, format, resistance / 1e3);
    } else {
        decimalPlaces = (resistance >= 100) ? 1 : (resistance >= 10) ? 2 : 3; // For below 1K
        sprintf(format, "%%.%df", decimalPlaces);
        sprintf(buffer, format, resistance);
    }

    return buffer;
}

float change_to_f(float temp_c){
  return (temp_c * 9.0/5.0) + 32.0;
}

touchEvent getTouch() {

    touchEvent result;
    result.x = -1;
    result.y = -1;
    result.gesture = CST816Touch::GESTURE_NONE; 
    result.gestureEvent = oTouch.hadGesture();

    bool printTouchDebug = true;

    if (oTouch.hadTouch()) {
      oTouch.getLastTouchPosition(result.x,result.y);
      result.y = tft.height() - result.y; // Flip y axis on touch to align with screen rotation
    } else if (result.gestureEvent) {
        oTouch.getLastGesture(result.gesture, result.x, result.y);
        result.y = tft.height() - result.y; // Flip y axis on touch to align with screen rotation
        if(printTouchDebug){
          Serial.println("- - - - - - - - - - - - - - - - ");
          Serial.println("Gesture Event:");
          Serial.print("Motion: ");
          Serial.println(oTouch.gestureIdToString(result.gesture));
          Serial.print("At location: ");
          Serial.printf("x: %d, y: %d \n", result.x, result.y);
        }
    }
    return result;
}


SensorReadings readExternalSensors() {
    // Implement your I2C reading logic here.
    Serial.println("Attempting to read External Sensor!");
    htu_ext.getEvent(&humidity, &temp);
    Serial.println("Sensor read successful!");
    return {temp.temperature, humidity.relative_humidity};
}

SensorReadings readInternalSensors() {
    // Implement your I2C reading logic here.
    Serial.println("Attempting to read Internal Sensor!");
    htu_int.getEvent(&humidity, &temp);
    Serial.println("Sensor read successful!");
    return {temp.temperature, humidity.relative_humidity};
}

void initializeTFT() {
  /*setup screen params*/
  width = PIXEL_WIDTH;
  height = PIXEL_HEIGHT;
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(5,5);
  tft.drawString("Initializing.",5,5);

  left_center = width*0.25;
  right_center = width*0.75;

  ledcSetup(0, 2000, 8);
  ledcAttachPin(PIN_LCD_BL, 0);
  ledcWrite(0, 255);


  oTouch.begin(Wire, nullptr, PIN_TOUCH_INT);
  oTouch.resetChip(true);
  tft.drawString("Initializing..",5,5);
  oTouch.setAutoSleep(false);
}

void initializeTempSensors() {
  if (!htu_ext.begin(0x40, &Wire)) {
    delay(3);
    if (!htu_ext.begin(0x40, &Wire)) {
      Serial.println("Failed to initialize external HTU31D sensor!");
    }
  } else{
    Serial.println("external temp sensor intiialized");
  }
  if (!htu_int.begin(0x41, &Wire)) {
    delay(3);
    if (!htu_ext.begin(0x40, &Wire)) {
      Serial.println("Failed to initialize internal HTU31D sensor!");
    }
  } else{
    Serial.println("internal temp sensor intiialized");
  }
}

void resetDisplay(){
    tft.fillScreen(TFT_WHITE);
    delay(1);
    tft.fillScreen(TFT_BLACK);
    delay(1);
}

void initMCU(){
  // Configure Pins
  gpio_hold_dis((gpio_num_t)PIN_TOUCH_RES);
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);
  pinMode(PIN_TOUCH_RES, OUTPUT);
  digitalWrite(PIN_TOUCH_RES, LOW);
  delay(500);
  digitalWrite(PIN_TOUCH_RES, HIGH);
  pinMode(PIN_BAT_VOLT, INPUT); 
  pinMode(PIN_GPIO_02, INPUT);
  pinMode(PIN_GPIO_03, INPUT);

  // Start I2C
  SensorsI2C.begin(PIN_THIC_SDA, PIN_THIC_SCL); // Initialize Sensor I2C BUS
  SensorsI2C.setClock(1000000);  // max clock speed for HTU31D
  delay(1);
  Wire.begin(PIN_IIC_SDA, PIN_IIC_SCL);
  Wire.setClock(400000);   // Mack clockspeed for Touch Controller
  delay(1);

  // Init TFT 
  initializeTFT();

  tft.drawString("Initializing...",5,5);
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    tft.drawString("Connecting to WiFi...",5,25);
    Serial.println("Connecting to WiFi...");
  }
  tft.drawString("Connected to WiFi",5,45);
  Serial.println("Connected to WiFi");

  // Initialize NTP
  tft.drawString("Configuring RTC",5,65);
  initializeNTP();


  // Create base sprite
  tft.drawString("Setup Complete",5,85);
  delay(500);
  initBaseSprite();
}

void initializeNTP() {
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");  // Use any NTP server you prefer
  sntp_init();

  // Wait till NTP sets the time
  time_t now = 0;
  struct tm timeinfo = { 0 };
  int retry = 0;
  const int retry_count = 10;
  while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
    delay(1000);
    time(&now);
    localtime_r(&now, &timeinfo);
  }

  // Set the time for TimeLib.h
  if (timeinfo.tm_year >= (2016 - 1900)) {
    setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  }

  // read offset from Flash on setup in future....
  const long EST = -4 * 3600;  // 4 hours * 3600 seconds/hour
  adjustTime(EST);

}

void initBaseSprite(){
  spr_base.createSprite(TFT_HEIGHT, TFT_WIDTH);
  spr_base.fillSprite(TFT_BLACK);  // Clear sprite
  spr_base.setTextSize(1);
  spr_base.setTextColor(TFT_WHITE, TFT_BLACK);
  updateBatterySprite();
  spr_base.pushSprite(0, 0);
}

void updateBatterySprite(){
  int sensorValue = analogRead(PIN_BAT_VOLT);
  bat_volt = (sensorValue / 1023.0) * 3.6;

  if (bat_volt >= 3.6) {
    iconData = (uint16_t*)gImage_bat_3_3; 
  } else if (bat_volt >= 3.4) {  // (3.6 + 3.2) / 2 = 3.4
    iconData = (uint16_t*)gImage_bat_2_3; 
  } else if (bat_volt >= 3.0) {  // (3.2 + 2.8) / 2 = 3.0
    iconData = (uint16_t*)gImage_bat_1_3; 
  } else {
    iconData = (uint16_t*)gImage_bat_0_3; 
  }  
  spr_base.pushImage(220,0,50,50,iconData,1);
}

void logSensorDataToNVM() {
    // Replace with your actual logging code
    // This could be writing to EEPROM, SPIFFS, or using the NVS library for ESP32
    // Log external data
      ///htu_ext.getEvent(&humidity, &temp);
    //log internal data
      ///htu_int.getEvent(&humidity, &temp);
    // log skin res

    Serial.println("Logged Data");
}


