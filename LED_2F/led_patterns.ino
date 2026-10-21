/*****************************< LED functions >*****************************/

void flash(uint8_t reset) {
  // チカチカ
  static uint8_t sChanceOfFlash = 32;
  if (reset) {
    sChanceOfFlash = 32;
    gBrightness = BRIGHTNESS;
  }
  sChanceOfFlash += getDeltaY(10);
  chgBrightness();
  fadeToBlackBy( leds, NUM_LEDS, 10);
  for (uint16_t i = 0; i < 4; ++i) {
    if (random8() < sChanceOfFlash) {
      leds[ random16(NUM_LEDS) ] += CRGB::White;
    }
  }
}
void bpm(uint8_t reset) {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  static uint8_t sBeatsPerMinute = 62;
  if (reset) {
    sBeatsPerMinute = 62;
    gBrightness = BRIGHTNESS;
  }
  sBeatsPerMinute -= getDeltaY(5);
  chgBrightness();
  CRGBPalette16 currentPalette = PartyColors_p;
  uint8_t beat = beatsin8( sBeatsPerMinute, 64, 255);
  for ( uint16_t i = 0; i < NUM_LEDS; ++i) {
    leds[i] = ColorFromPalette(currentPalette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}
void solid(uint8_t reset) {
  // 普通の光
  static uint8_t sHue = 24;
  if (reset) {
    sHue = 24;
    gBrightness = BRIGHTNESS;
  }
  sHue += getDeltaY(10);
  chgBrightness();
  fill_solid( leds, NUM_LEDS, CHSV( sHue, 255, 255));
}
void confetti(uint8_t reset)
{
  static uint8_t sHue = 24;
  if (reset) {
    sHue = 24;
    gBrightness = BRIGHTNESS;
  }
  sHue += getDeltaY(10);
  chgBrightness();
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 5);
  uint8_t pos = random16(NUM_LEDS / 10);
  uint8_t hue = random8(64);
  for (uint16_t i = 0; i < 10; ++i) {
    leds[pos * 10 + i] += CHSV( sHue + hue, 200, 255);
  }
}

void gradation(uint8_t reset) {
  static uint8_t sPattern = 4;
  static uint8_t sDeltaHue = 7;
  if (reset) {
    sPattern = 4;
    sDeltaHue = 7;
  }
  if (gX || gY || gB) {
    sPattern = (sPattern + getDeltaY(1) + 8) % 8; // 0~7(Rotateあり）
    chgBrightness();
  }
  CRGBPalette16 currentPalette;
  switch (sPattern) {
    case 0:   currentPalette = CloudColors_p;         break;  // blue and white
    case 1:   currentPalette = LavaColors_p;          break;  // orange, red, black and yellow),
    case 2:   currentPalette = OceanColors_p;         break;  // blue, cyan and white
    case 3:   currentPalette = ForestColors_p;        break;  // greens and blues
    case 4:   currentPalette = RainbowColors_p;       break;  // standard rainbow animation
    case 5:   currentPalette = RainbowStripeColors_p; break;  // single colour, black space, next colour and so forth
    case 6:   currentPalette = PartyColors_p;         break;  // red, yellow, orange, purple and blue
    case 7:   currentPalette = HeatColors_p;          break;  // red, orange, yellow and white
  }
  for (uint16_t i = 0; i < NUM_LEDS; ++i)  {
    leds[i] = ColorFromPalette(currentPalette, gHue + i * sDeltaHue, 255);
  }
}

void fire(uint8_t reset) {
  static uint8_t sHue = 0;
  if (reset) {
    sHue = 0;
    gBrightness = BRIGHTNESS;
  }
  sHue = sHue + getDeltaY(10);
  chgBrightness();

  CRGBPalette16 currentPalette = HeatColors_p;
  int16_t a = millis();
  // 128の仮想LEDで絵作りして部屋の各壁に投影
  for (uint16_t i = 0; i < 128; ++i) {
    uint8_t noise = inoise8 (0 , i * 60 - a , a / 3);
    uint8_t index = qsub8 (noise, i * 2);
    // Set the LED color from the palette
    gLEDBuffer = ColorFromPalette (currentPalette, index, 255);
    //  白の時は変換しない
    if ((gLEDBuffer.r != gLEDBuffer.g) || ( gLEDBuffer.r != gLEDBuffer.b)) {
      CHSV tempColor = rgb2hsv_rainbow(gLEDBuffer);
      tempColor.hue = (tempColor.hue + sHue ) % 256;
      gLEDBuffer = tempColor;
    }
    drawToEachHalfWall(i);
  }
}

void blur(uint8_t reset) {
  if (reset) {
    //    sHue = 0;
    gBrightness = BRIGHTNESS;
  }
  chgBrightness();

  uint8_t sinBeat   = beatsin8(20, 0, 127, 0, 0);
  uint8_t sinBeat2  = beatsin8(30, 0, 127, 0, 85);
  uint8_t sinBeat3  = beatsin8(30, 0, 127, 0, 170);
  gLEDBuffer   = CRGB::Green;
  drawToEachHalfWall(sinBeat);
  gLEDBuffer  = CRGB::Blue;
  drawToEachHalfWall(sinBeat2);
  gLEDBuffer  = CRGB::Red;
  drawToEachWall(sinBeat3);

  EVERY_N_MILLISECONDS(1) {
    for (int i = 0; i < 4; i++) {
      blur1d(leds, NUM_LEDS, 50);
    }
  }
}



void flickerBySound(uint8_t reset) {
  //音で揺らぐ光
  static uint8_t sHue = 24;
  if (reset) {
    sHue = 24;
    gSensitivity = 8;
  }
  sHue += getDeltaY(10);
  chgSensitivity();

  uint16_t t;
  int16_t  noise;
  uint16_t brightness;

  getPeakLevel();
  brightness = constrain(gSoundLevel * 8 / gSensitivity, 0, 63);  // マイク入力と感度から0~63くらいになる数を算出
  //Serial.println(brightness);
  t = millis() / 5;
  for (uint16_t i = 0; i < NUM_LEDS; ++i) {
    noise = inoise8(i * 10, t) / 4 - 32;     // -32~+31のばらつきを持たせて
    leds[i].setHSV(sHue + noise, 200, brightness + noise + 160);
  }
}
void levelmeter(uint8_t reset) {
  // レベルメーター
  static uint8_t sHue = 96;
  static uint8_t sMax = 0;
  static uint32_t previousTime = 0;

  if (reset) {
    sHue = 96;
    gSensitivity = 8;
  }
  sHue += getDeltaY(10);
  chgSensitivity();


  // マイク入力と感度から0~127になる数を算出
  getPeakLevel();
  uint16_t numTurnOn = constrain(gSoundLevel * 8 / gSensitivity, 0, 127);
  // ピークマーカー
  if (sMax < numTurnOn) {
    sMax = numTurnOn;
  } else if (sMax > 0) {
    sMax--;
  }

  // 描画
  // 100msに1回更新
  if ((millis() - previousTime) < 100) {
    return;
  }
  previousTime = millis();

  uint8_t hue = sHue;

  for (uint16_t i = 0; i < 127; ++i) {
    if (i == 72) hue -= 96;   // よく見るレベルメーターのように60%以上から色を変える
    if (i < numTurnOn) {      // 点灯
      gLEDBuffer.setHSV(hue, 255, 255);
    } else {
      gLEDBuffer.setHSV(hue, 0, 0);
    }
    if ((i % 8) > 4) {        // 8個につき3個消灯して黒線を入れる
      gLEDBuffer.setHSV(hue, 0, 0);
    }
    if (sMax < i && i < (sMax + 5)) {  // ピークマーカー
      gLEDBuffer.setHSV(hue, 0, 255);
    }
    drawToEachHalfWall(i);
  }
}


void moveByJoystick(uint8_t reset) {
  // Joystickで色と光を動かす
  static int16_t lastLED = 0;
  int8_t  color = 10;
  fadeToBlackBy( leds, NUM_LEDS, 50);
  // ある程度ジョイスティックを動かしたら
  if ( gX < -1000 || 1000 < gX || gY < -1000 || 1000 < gY) {
    // 部屋のLEDの配置に合わせてジョイスティックの入力値から移動先LEDを算出
    int16_t destinationLED = (atan2( -gY,  -gX) / 6.2832 + 0.5) * NUM_LEDS;
    // 相対距離に変換して（最短方向になるように考慮）
    int16_t relativeLED = ((int16_t)(destinationLED - lastLED + NUM_LEDS * 1.5) % NUM_LEDS) - NUM_LEDS / 2;
    // 相対距離の1/10を1回の移動量に（小数点以下は切り上げ・負の場合小数点以下は切り下げと同じになるよう）
    relativeLED = ((0 < relativeLED) - (relativeLED < 0)) * ceil(abs(relativeLED) / 10);
    // 移動中は色を変える
    color = 255;
    // オーバーフローやマイナスにならないように考慮して移動量を加算
    lastLED = (lastLED + relativeLED + NUM_LEDS) % NUM_LEDS;
  }

  int16_t workLED = lastLED;
  for (int i = 0; i < (NUM_LEDS / 16); ++i) {
    leds[workLED].setHSV(gHue, color, 255);
    workLED++;
    if (workLED == NUM_LEDS) {
      workLED = 0;
    }
  }
}

CRGBPalette16 pacifica_palette_1 =
{ 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
  0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50
};
CRGBPalette16 pacifica_palette_2 =
{ 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
  0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F
};
CRGBPalette16 pacifica_palette_3 =
{ 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33,
  0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF
};
void pacifica_loop(uint8_t p) {
  if (p) {
    //    sHue = 0;
    gBrightness = BRIGHTNESS;
  }
  chgBrightness();
  
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011, 10, 13));
  sCIStart2 -= (deltams21 * beatsin88(777, 8, 11));
  sCIStart3 -= (deltams1 * beatsin88(501, 5, 7));
  sCIStart4 -= (deltams2 * beatsin88(257, 4, 6));

  // Clear out the LED array to a dim background blue-green
  fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0 - beat16( 301) );
  pacifica_one_layer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
  pacifica_one_layer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10, 38), 0 - beat16(503));
  pacifica_one_layer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10, 28), beat16(601));

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps();

  // Deepen the blues and greens a bit
  pacifica_deepen_colors();
}
// Add one layer of waves into the led array
void pacifica_one_layer( CRGBPalette16 & p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for ( uint16_t i = 0; i < NUM_LEDS; ++i) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}
// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );

  for ( uint16_t i = 0; i < NUM_LEDS; ++i) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if ( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}
// Deepen the blues and greens
void pacifica_deepen_colors()
{
  for ( uint16_t i = 0; i < NUM_LEDS; ++i) {
    leds[i].blue = scale8( leds[i].blue,  145);
    leds[i].green = scale8( leds[i].green, 200);
    leds[i] |= CRGB( 2, 5, 7);
  }
}
