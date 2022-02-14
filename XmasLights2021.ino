/**
 * @file XmasLights2021.ino
 * @author Thomas J. Petz, Jr. (tom@tjpetz.com)
 * @brief Christmas tree light effects for WS1218B LEDS using FastLED with BLE configuration.
 * @version 0.1
 * @date 2022-02-13
 * 
 * @copyright Copyright (c) 2022
 * 
 * @note While it might be nice to use the flash storage on the NINA module the
 * ArduinoBLE moduel and WiFiNINA are difficult to use together.  Using the WiFiNINA to
 * access the filesystem disables the BLE functionality.
 */

#include <Arduino.h>
#include <ArduinoBLE.h>
#include <FlashStorage.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <FastLED.h>

/** Global defaults */
#define CANDY_STRIP_WIDTH 5
#define TRAIN_CAR_LENGTH 5
#define BLE_LOCAL_NAME "XmasLights_001"
#define BLE_DEVICE_NAME "XmasLights"
#define NUMBER_OF_LIGHTS 144
#define DATA_PIN 3

/** guid block
f0d754d8-a042-4d39-9cec-5b2243a2de86
ecb47133-a2dc-481f-919b-d878ccf2fce3
6fea786a-3fa5-48e5-bd7b-4ec7a089d7d2
60a94c4b-4406-4c8b-86c9-3cbc69a19067
12ee8384-d7b7-4a5a-9176-0f464507be0b
*/

#define MAX_DEBUG_BUFF 256
#define LOG(...) \
{ \
  char _buff[MAX_DEBUG_BUFF]; \
  snprintf(_buff, MAX_DEBUG_BUFF, __VA_ARGS__); \
  Serial.print(_buff); \
}

/** Save our configuration parameters as BLE characteristics */
BLEService g_BLEService ("81bea2b7-ad1a-493a-bf19-123596b3328b");
BLEBoolCharacteristic g_runLights("3a6d65bb-ed42-4443-a23b-4225e76f10d8", BLERead | BLEWrite);
BLEUnsignedIntCharacteristic g_numberOfLights("a9497a4a-4735-4b50-b10c-da941ac7b51b", BLERead | BLEWrite);
BLEUnsignedIntCharacteristic g_candyStripWidth("672b85ce-5175-485e-a9e2-739ebae601d9", BLERead | BLEWrite);
BLEUnsignedIntCharacteristic g_trainCarLength("9307a368-8d50-48dd-92e8-9f65bb15f98f", BLERead | BLEWrite);

/** TODO: fix this as it's a fixed length array */
CRGB leds[NUMBER_OF_LIGHTS];
const CHSV red = CHSV(0, 255, 72);
const CHSV green = CHSV(85, 255, 72);
const CRGB white = CHSV(0, 0, 50);
const CRGB black = CHSV(0, 0, 0);;

/** OLED Display */
Adafruit_SSD1306 display(128, 64);

/** Configuration storage structure */
#define CONFIG_FILE_VERSION 1
typedef struct {
  int version;
  bool run;
  int nbrOfLeds;
  int candyStripWidth;
  int trainCarLength;
} configData_t;

FlashStorage(myConfigData, configData_t);
configData_t g_configData;


/** When BLE disconnects update the configuration file */
void updateConfiguration(BLEDevice central) {
  
  LOG("BLE Disconnected\n");

  // Write the config out if anything has changed.
  if (g_runLights.value() != g_configData.run ||
      g_numberOfLights.value() != g_configData.nbrOfLeds ||
      g_candyStripWidth.value() != g_configData.candyStripWidth ||
      g_trainCarLength.value() != g_configData.trainCarLength) {
    LOG("Writing Configuration\n");
    g_configData.run = g_runLights.value();
    g_configData.nbrOfLeds = g_numberOfLights.value();
    g_configData.candyStripWidth = g_candyStripWidth.value();
    g_configData.trainCarLength = g_trainCarLength.value();

    myConfigData.write(g_configData);
  }
}


/** Configure the BLE Service and it's characteristics and descriptors */
void configureBLEService() {

  LOG("configureBLEService\n");

  g_configData = myConfigData.read();

  if (g_configData.version != CONFIG_FILE_VERSION) {
    // Use the defaults if the version is wrong or not present
    LOG("Using the default configuration\n");
    g_configData.version = CONFIG_FILE_VERSION;
    g_configData.run = true;
    g_configData.nbrOfLeds = NUMBER_OF_LIGHTS;
    g_configData.candyStripWidth = CANDY_STRIP_WIDTH;
    g_configData.trainCarLength = TRAIN_CAR_LENGTH;
  }

  g_runLights.writeValue(g_configData.run);
  g_numberOfLights.writeValue(g_configData.nbrOfLeds);
  g_candyStripWidth.writeValue(g_configData.candyStripWidth);
  g_trainCarLength.writeValue(g_configData.trainCarLength);

  // Add the characteristics
  g_BLEService.addCharacteristic(g_runLights);
  g_BLEService.addCharacteristic(g_numberOfLights);
  g_BLEService.addCharacteristic(g_candyStripWidth);
  g_BLEService.addCharacteristic(g_trainCarLength);

  // Setup the service
  BLE.addService(g_BLEService);
  BLE.setEventHandler(BLEDisconnected, updateConfiguration);

  LOG("BLE setup complete\n");
}


/** Green and Red Train */
void train(unsigned int trainLength, unsigned int nbrLEDS) {

  static int offset = 0;      // train position, advanced each call

  FastLED.clear();
  for (int j = 0; j < trainLength; j++) {
    if ((j + offset) < nbrLEDS) {
      leds[j + offset] = red;
    }
    if ((j + trainLength + offset) < nbrLEDS) {
      leds[j + trainLength + offset] = green;
    }
  }
  
  FastLED.show();
  offset = (offset + 1) % nbrLEDS;
}


/** Rotating candy cane - move the candy cane one step everytime we're called */
void candyCane(unsigned int stripWidth, unsigned int nbrLEDS) {

  static int offset = 0;

  // Fill all with background color - white
  for (int i = 0; i < nbrLEDS; i++) {
    leds[i] = white;
  }
  
  // Draw the red stripes - compute the number of strips
  for (int i = 0; i < (nbrLEDS / stripWidth); i++) {
    // Draw each strip 2 strip widths apart.
    for (int j = i * stripWidth * 2; j < ((i * stripWidth * 2) + stripWidth); j++) {
      leds[(j + offset) % nbrLEDS] = red;
    }
   }

  offset = (offset + 1) % nbrLEDS;

  FastLED.show(); 
}


/** random green and red */
void randomGreenAndRed(unsigned int nbrLEDS) {

  for (int i = 0; i < nbrLEDS; i++) {
    leds[i] = random(10) > 5 ? red : green;
  }
  FastLED.show();

}


void updateDisplay(float fps) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  display.setFont(NULL);  // use the default font
  display.setCursor(0,0);
  display.print("FPS: "); display.print(fps);
  display.setCursor(0, 10);
  display.print("Required Pwr: "); 
  display.print(calculate_unscaled_power_mW(leds, NUMBER_OF_LIGHTS)); 
  display.print(" mW");
  if (BLE.connected()) {
    display.setCursor(0, 20);
    display.print("BLE Connected");
    display.setCursor(10, 30);
    display.print(BLE.central().address());
  }
  display.display();
  
}


void setup() {

  Serial.begin(115200);
  delay(3000);

  pinMode(DATA_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Failed to initialize the display!");
    while(1);
  }
  display.clearDisplay();

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUMBER_OF_LIGHTS);
  FastLED.setMaxPowerInMilliWatts(250);
  set_max_power_indicator_LED(LED_BUILTIN);

  if (!BLE.begin()) {
    Serial.println("Error initializing BLE!");
  } else {
    configureBLEService();
    BLE.setAdvertisedService(g_BLEService);
    BLE.setLocalName(BLE_LOCAL_NAME);
    BLE.setDeviceName(BLE_DEVICE_NAME);
    BLE.advertise();
  }
}

float g_fps = 0.0;
int currentEffect = 0;
const int maxEffects = 3;

void loop() {

  int startMillis = millis();

  if (!BLE.connected()) {         // Only run the effects if NOT connected to BLE
    if (g_runLights.value()) {

      // Run the current effect
      EVERY_N_MILLISECONDS(500) {
        switch (currentEffect) {
          case 0:
            candyCane(g_candyStripWidth.value(), g_numberOfLights.value());
            break;
          case 1:
            randomGreenAndRed(g_numberOfLights.value());
            break;
          case 2:
            train(g_trainCarLength.value(), g_numberOfLights.value());
            break;
        }
      }

      // Switch to the next effect
      EVERY_N_SECONDS(15) {
        currentEffect = (currentEffect + 1) % maxEffects;
      }
   } else {
      FastLED.clear(true);
    }
  }

  int duration = millis() - startMillis;
  g_fps = 0.9f * g_fps + 0.1f * (1000.0f / duration);
  updateDisplay(g_fps);

  BLE.poll(50);
}