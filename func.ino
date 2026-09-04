// gLEDBufferに色情報を入れて0~127の座標を各壁に割りつけてコピー
void drawToEachWall(uint16_t i) {
  //  leds[(uint16_t)(NUM_LEDS * i / 128)] = gLEDBuffer;
  leds[(uint16_t)(NUM_LEDS_A * i / 128)] = gLEDBuffer;
  leds[(uint16_t)(NUM_LEDS_A + NUM_LEDS_B / 2 - 1 - NUM_LEDS_B * i / 256 )] = gLEDBuffer;
  leds[(uint16_t)(NUM_LEDS_A + NUM_LEDS_B / 2     + NUM_LEDS_B * i / 256 )] = gLEDBuffer;
  leds[(uint16_t)(NUM_LEDS_A * 2 + NUM_LEDS_B - 1 - NUM_LEDS_A * i / 128)] = gLEDBuffer;
  leds[(uint16_t)(NUM_LEDS_A * 2 + NUM_LEDS_B     + NUM_LEDS_C * i / 128)] = gLEDBuffer;
  leds[(uint16_t)(NUM_LEDS - NUM_LEDS_C - NUM_LEDS_B / 2 - 1 - NUM_LEDS_B * i / 256 )] = gLEDBuffer;
  leds[(uint16_t)(NUM_LEDS - NUM_LEDS_C - NUM_LEDS_B / 2     + NUM_LEDS_B * i / 256 )] = gLEDBuffer;
  leds[(uint16_t)(NUM_LEDS - 1 - NUM_LEDS_C * i / 128 )] = gLEDBuffer;
}


// gLEDBufferに色情報を入れて0~127の座標を各壁に半分ずつコピー
void drawToEachHalfWall(uint8_t i) {
  //    leds[(uint8_t)(32 * i / 128)] = gLEDBuffer;
  leds[(uint16_t)(NUM_LEDS_A * i / 256)] = gLEDBuffer;
  leds[(uint16_t)(NUM_LEDS_A - NUM_LEDS_A * i / 256 - 1)] = gLEDBuffer;

  leds[(uint16_t)(NUM_LEDS_A + NUM_LEDS_B * i / 256)] = gLEDBuffer;
  leds[(uint16_t)(NUM_LEDS_A + NUM_LEDS_B - NUM_LEDS_B * i / 256 - 1)] = gLEDBuffer;

  leds[(uint16_t)(NUM_LEDS_A     + NUM_LEDS_B + NUM_LEDS_A * i / 256)] = gLEDBuffer;
  leds[(uint16_t)(NUM_LEDS_A * 2 + NUM_LEDS_B - NUM_LEDS_A * i / 256 - 1)] = gLEDBuffer;

  leds[(uint16_t)(NUM_LEDS_A * 2 + NUM_LEDS_B + NUM_LEDS_C * i / 256)] = gLEDBuffer;
  leds[(uint16_t)(NUM_LEDS_A * 2 + NUM_LEDS_B + NUM_LEDS_C  - NUM_LEDS_C * i / 256 - 1)] = gLEDBuffer;

  leds[(uint16_t)(NUM_LEDS_A * 2 + NUM_LEDS_B + NUM_LEDS_C + NUM_LEDS_B * i / 256 )] = gLEDBuffer;
  leds[(uint16_t)(NUM_LEDS_A * 2 + NUM_LEDS_B + NUM_LEDS_C + NUM_LEDS_B - NUM_LEDS_B * i / 256 - 1)] = gLEDBuffer;

  leds[(uint16_t)(NUM_LEDS - NUM_LEDS_C + NUM_LEDS_C * i / 256 )] = gLEDBuffer;
  leds[(uint16_t)(NUM_LEDS - NUM_LEDS_C * i / 256 - 1)] = gLEDBuffer;
}


// rgb2hsv_approximateにバグがあるので代替
CHSV rgb2hsv_rainbow(CRGB input_color) {
  CHSV output_color;
  uint8_t maximum = max(input_color.r, max(input_color.g, input_color.b));
  uint8_t minimum = min(input_color.r, min(input_color.g, input_color.b));
  output_color.v = maximum;
  uint8_t delta = maximum - minimum;
  output_color.s = (maximum == 0) ? 0 : (255 * delta / maximum);
  if (maximum == minimum) {
    output_color.h = 0;
  }
  else {
    if (maximum == input_color.r) {
      if (input_color.g >= input_color.b) {
        if ((input_color.r - input_color.g) <= (delta / 3)) {
          output_color.h = map((input_color.r - input_color.g), delta / 3, 0, 32, 64);
        }
        else {
          output_color.h = map((input_color.r - input_color.g), delta, delta / 3, 0, 32);
        }
      }
      else {
        output_color.h = map((input_color.r - input_color.b), 0, delta, 208, 255);
      }
    }
    else if (maximum == input_color.g) {
      if (input_color.r >= input_color.b) {
        output_color.h = map((input_color.g - input_color.r), 0, delta, 64, 96);
      }
      else {
        if (input_color.g - input_color.b <= delta / 3) {
          output_color.h = map((input_color.g - input_color.b), delta, delta / 3, 96, 128);
        }
        else {
          output_color.h = map((input_color.g - input_color.b), delta / 3, delta, 128, 160);
        }
      }
    }
    else if (maximum == input_color.b) {
      if (input_color.g > input_color.r) {
        output_color.h = map((input_color.b - input_color.g), -delta / 3, delta, 128, 160);
      }
      else {
        output_color.h = map((input_color.b - input_color.r), delta, 0, 160, 208);
      }
    }
  }
  return output_color;
}
