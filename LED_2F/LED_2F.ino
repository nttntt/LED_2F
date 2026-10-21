#define FASTLED_ESP32_I2S true
#include <Arduino.h>
#include <FastLED.h>
#include <BLEDevice.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>


/* Function Prototype */
void doInitialize(void);
void displayData(void);
void bleTask(void * pvParameters);
void getPeakLevel(void);
void chgBrightness(void);
void chgSensitivity(void);
int16_t getDeltaY(int8_t);
void nextPattern(void);
void flash(uint8_t);
void bpm(uint8_t);
void solid(uint8_t);
void confetti(uint8_t);
void blur(uint8_t);
void flickerBySound(uint8_t);
void levelmeter(uint8_t);
void moveByJoystick(uint8_t);
void pacifica_loop(uint8_t);
void fire(uint8_t);
void gradation(uint8_t);
void drawToEachWall(uint16_t);
CHSV rgb2hsv_rainbow(CRGB);
void httpListen(void);
void httpRequestProccess(String*);
void httpSendResponse(WiFiClient*);
/* 基本属性定義  */
#define SPI_SPEED   115200            // SPI通信速度

// Delete the include line and uncomment the two lines below to enter your Wi-Fi details.
#include <credentials.h>
//  #define WIFI_SSID     "InputYourSSID"
//  #define WIFI_PASSWORD "InputYourPassword"

// Multicast DNS名
#define LEDS_NAME "LED_2F"

// Webサーバーオブジェクト
#define HTTP_PORT 80
WebServer server(HTTP_PORT);

/* LED関連 */

#define DATA_PIN    25
#define LED_TYPE    WS2812B
#define COLOR_ORDER RGB
#define NUM_LEDS    680
#define NUM_LEDS_A  114  // 部屋の各辺のLED数
#define NUM_LEDS_B  104
#define NUM_LEDS_C  122
#define BRIGHTNESS  255
#define FRAMES_PER_SECOND  50

CRGB leds[NUM_LEDS];
CRGB gLEDBuffer;

uint8_t gBrightness = BRIGHTNESS;
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0;                  // rotating "base color" used by many of the patterns

typedef void (*SimplePatternList[])(uint8_t);
SimplePatternList gPatterns = { gradation, solid, pacifica_loop, flash, bpm,  confetti, fire, blur, flickerBySound, levelmeter, moveByJoystick};
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

/* BLE関連 */
#define   DEVICE_NAME "LED_2F"         // 対象デバイス名
#define   ManufacturerId 0xffff       // 既定のManufacturer ID
const uint8_t scanning_time = 3;      // スキャン時間（秒）
BLEAdvertisedDevice* targetDevice;    // 目的のBLEデバイス
String gData = "";
uint8_t gPrevSeq = 255;               // 前回のシーケンス番号
int16_t gX = 0;
int16_t gY = 0;
uint8_t gB = 0;

/* マイク関連 */
#define MIC_PIN       32
#define NOISE_LEVEL   10
uint16_t gSoundLevel = 0;   // 計測値（0~4095）
uint16_t gSensitivity = 8;  // 感度

/*****************************< Define Callback >*****************************/
// アドバタイジング受信時コールバック
class advertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (advertisedDevice.getName() == DEVICE_NAME) {
        BLEDevice::getScan()->stop();
        gData = advertisedDevice.getManufacturerData();
        // Serial.println(advertisedDevice.toString().c_str());
      }
    }
};

/*****************************************************************************
                            Predetermined Sequence
 *****************************************************************************/
void setup() {
  // 初期化処理をして
  doInitialize();
  // BLE受信のタスク起動
  xTaskCreatePinnedToCore( bleTask,  "bleTask", 4096, NULL, 10, NULL, 0);
  xTaskCreatePinnedToCore(htmlTask, "htmlTask", 4096, NULL, 10, NULL, 0);
}

void loop() {
  boolean autoRotate = false;
  boolean restorePreset = 0;

  if (0 < gB && gB < ARRAY_SIZE( gPatterns) + 1) {
    gCurrentPatternNumber = gB - 1;
    gB = 0;
  }
  if (gB == 16) {
    restorePreset = 1;
  }

  if (gB == 17) {
    autoRotate = true;
  } else if (gB) {
    autoRotate = false;
  }

  if (autoRotate) {
    EVERY_N_SECONDS( 10 ) {
      nextPattern();  // change patterns periodically
    }
  }

  gPatterns[gCurrentPatternNumber](restorePreset);     // Call the current pattern function once, updating the 'leds' array
  restorePreset = 0;

  FastLED.show();

  FastLED.delay(1000 / FRAMES_PER_SECOND); // insert a delay to keep the framerate modest
  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }
  //  displayData();
}


/*****************************< Other functions >*****************************/
/*  初期化処理  */
void doInitialize() {
  Serial.begin(SPI_SPEED);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(gBrightness);

  Serial.println("LED Controller start ...");
}

/* 次のパターンに変更 */
void nextPattern() {
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

/* 受信データを表示する */
void displayData() {
  Serial.print("No.");      Serial.print(gPrevSeq);
  Serial.print("  X:");     Serial.print(gX);
  Serial.print(",  Y:");    Serial.print(gY);
  Serial.print(",  B:");    Serial.print(gB);
  Serial.print(",  S:");    Serial.print(gSensitivity);
  Serial.print(",  M:");    Serial.println(gCurrentPatternNumber);
}

/****************************< MIC functions >****************************
  #define SAMPLING_RATE 50    // ミリセカンド
  void getPeakLevel() {
  static uint32_t sStartMillis = millis();
  static uint16_t sSignalMax = 1700;
  static uint16_t sSignalMin = 1900;
  static uint16_t micin = 1800;

  if (millis() - sStartMillis < SAMPLING_RATE) {
    micin = analogRead(MIC_PIN);
    if ((0 <= micin) && ( micin <= 4095)) {
      if (micin > sSignalMax) {
        sSignalMax = micin;
      } else if (micin < sSignalMin) {
        sSignalMin = micin;
      }
    }
  } else {
    gSoundLevel = abs(sSignalMax - sSignalMin);
    gSoundLevel = constrain(gSoundLevel - NOISE_LEVEL, 0, 4095);
    sSignalMax = 1700;
    sSignalMin = 1900;
    sStartMillis = millis();
  }
  }*/
void getPeakLevel() {
  static int32_t sAverageLevel = 1800;
  uint16_t micin = analogRead(MIC_PIN);

  sAverageLevel = (sAverageLevel * 19 + micin) / 20;
  //  gSoundLevel = (gSoundLevel + abs(micin - sAverageLevel)) / 2;
  gSoundLevel = abs(micin - sAverageLevel);
  gSoundLevel = constrain(gSoundLevel - NOISE_LEVEL, 0, 4095);
  //  Serial.print("sAverageLevel:");
  //  Serial.print(sAverageLevel - 1500);
  //  Serial.print(" gSoundLevel:");
  //  Serial.print(gSoundLevel);
  //  Serial.print(" micin:");
  //  Serial.println(micin - 1500);

}





/****************************< Joystick functions >****************************/
// JoystickのX軸で明るさ調整
void chgBrightness() {
  if (gX) {
    // -2000~2000を-32~32に変換
    int8_t delta = gX * 32 / 2000;
    gBrightness = constrain(gBrightness + delta, 10, 255);
    FastLED.setBrightness(fl::dim8_lin(gBrightness));
    gX = 0;
  }
}

void chgSensitivity() {
  if (gX < -1000 && gSensitivity > 1) {
    gSensitivity = gSensitivity - 1;
  } else if (gX > 1000 && gSensitivity < 12) {
    gSensitivity = gSensitivity + 1;
  }
  gX = 0;
}

int16_t getDeltaY(int8_t range) {
  int16_t result = 0;
  if (gY < -100 || 100 < gY) {
    result = gY * range / 2000;
  }
  gY = 0;
  return result;
}

/*****************************< BLE Task functions >*****************************/
/* マルチタスクでBLE待ち受け */
void bleTask(void * pvParameters) {

  BLEDevice::init("");                            // BLEデバイスを作成する
  BLEScan* pBLEScan = BLEDevice::getScan();       // Scanオブジェクトを取得して、
  pBLEScan->setAdvertisedDeviceCallbacks(new advertisedDeviceCallbacks()); //コールバックを設定
  pBLEScan->setActiveScan(false);                 // パッシブスキャンに設定する
  while (true) {
    pBLEScan->start(0);                             // スキャン

    //  uint8_t manuCode  = gData[1] << 8 | gData[0];
    uint8_t seqNumber = gData[2];

    if (seqNumber != gPrevSeq) {    // シーケンス番号が更新されていたら受信データを取り出す
      gPrevSeq = seqNumber;
      gX = gData[4] << 8 | gData[3];
      gY = gData[6] << 8 | gData[5];
      gB = gData[7];
    }
    delay(1);
    //displayData();
  }
}

/****************************< HTTP functions >****************************/
/* マルチタスクでHTTP & OTA待ち受け */
void htmlTask(void * pvParameters) {
  connectToWifi();            // Wi-Fiルーターに接続する
  startMDNS();                // Multicast DNS
  startWebServer();           // WebServer
  startOTA();


  while (true) {
    server.handleClient();         // HTTPをリスンする
    ArduinoOTA.handle();
    delay(1);
  }
}

/* Wi-Fiルーターに接続する */
void connectToWifi() {
  Serial.print("\nScanning ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false);

  int n = WiFi.scanNetworks();

  int targetIndex = -1;
  int bestRSSI = -1000;

  for (int i = 0; i < n; i++) {
    int rssi = WiFi.RSSI(i);
    Serial.printf("Found: SSID=%-32s BSSID=%s CH=%2d RSSI=%3d\n", WiFi.SSID(i).c_str(), WiFi.BSSIDstr(i).c_str(), WiFi.channel(i), rssi);
    if (WiFi.SSID(i) == WIFI_SSID) {
      if (rssi > bestRSSI) {
        bestRSSI = rssi;
        targetIndex = i;
      }
    }
  }

  if (targetIndex != -1) {
    uint8_t bssid[6];
    memcpy(bssid, WiFi.BSSID(targetIndex), 6);
    int channel = WiFi.channel(targetIndex);

    WiFi.begin(
      WIFI_SSID,
      WIFI_PASSWORD,
      channel,
      bssid,
      true);

    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected");
    Serial.printf("SSID : %s\n", WiFi.SSID(targetIndex).c_str());
    Serial.printf("BSSID: %s\n", WiFi.BSSIDstr(targetIndex).c_str());
    Serial.printf("CH   : %2d\n", channel);
    Serial.printf("RSSI : %3d\n", WiFi.RSSI(targetIndex));
    Serial.printf("IP Address : %s\n", WiFi.localIP().toString().c_str());

  } else {
    Serial.print(WIFI_SSID);
    Serial.println(" not found");
  }

  WiFi.scanDelete();
}

/* Multicast DNSを開始する */
void startMDNS() {
  Serial.print("mDNS server instancing ");
  while (!MDNS.begin(LEDS_NAME)) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("success!");
}

void startWebServer() {
  server.on("/", handleHtml);
  server.onNotFound(handleNotFound);
  server.begin();             // HTTPサーバ開始
  Serial.println("WebServer Started.");

}

void startOTA() {
  ArduinoOTA
  .setHostname(LEDS_NAME)
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA Started.");
}
