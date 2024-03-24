#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

Adafruit_NeoPixel pixels(4, rgbpin , NEO_GRB + NEO_KHZ800);

void beep(int level, int len)
{
  tone(beeperpin, level * 100);
  delay(len);

  noTone(beeperpin);
}
int beeptype = 10;
long lastbeep = 0;

int ledstat = 0;
/*

   Dirty LED code:
   Namco LED has tons of mode that difficult to determine
   so only coded modes that games use frequently.

  0->off
  1->green to blue
  2->blue breath
  3->RGB breath
  4->red falling
  7->red to blue
  8->green blue green blue~
  10->RYloop
  11->blue keep
*/

long leddelay = 0, ledlasttim = 0, ledlev = 0, ledext = 0;

void ledsvc()
{
  if (!ledstat && !ledext)
  {
    pixels.fill(pixels.Color(0, 0, 0));
    pixels.show(); // Send the updated pixel colors to the hardware.
    ledext = 1;
    return;
  }
  if (ledstat == 11 && !ledext) {
    pixels.fill(pixels.Color(0, 0, 200));
    pixels.show(); // Send the updated pixel colors to the hardware.
    ledext = 1;
  }
  if (ledstat == 8)
  {
    // 绿蓝闪烁
    // delay bypass,tim means a col 's last time,lev bypass ,ext is color
    if (millis() - ledlasttim >= 150)
    {
      ledext = !ledext;
      ledlasttim = millis();
    }
    if (ledext)
    {
      pixels.fill(pixels.Color(0, 0, 255));
      pixels.show(); // Send the updated pixel colors to the hardware.
    }
    else
    {
      pixels.fill(pixels.Color(0, 255, 000));
      pixels.show(); // Send the updated pixel colors to the hardware.
    }
  }

  if (ledstat == 10)
  {
    // RY闪烁
    // delay bypass,tim means a col 's last time,lev bypass ,ext is color
    if (millis() - ledlasttim >= 500)
    {
      ledext = !ledext;
      ledlasttim = millis();
    }
    if (ledext)
    {
      pixels.fill(pixels.Color(255, 0, 0));
      pixels.show(); // Send the updated pixel colors to the hardware.
    }
    else
    {
      pixels.fill(pixels.Color(255, 255, 000));
      pixels.show(); // Send the updated pixel colors to the hardware.
    }
  }

  if (ledstat == 4)
  {
    if (ledlev <= 0)
    {
      pixels.fill(pixels.Color(0, 0, 0));
      pixels.show(); // Send the updated pixel colors to the hardware.
      ledstat = 0;
      return;
    }
    if (millis() - ledlasttim >= leddelay)
    {
      pixels.fill(pixels.Color(ledlev, 0, 0));
      pixels.show(); // Send the updated pixel colors to the hardware.
      ledlev -= 5;
      ledlasttim = millis();
    }
  }
  if (ledstat == 1)
  {
    if (ledlev <= 0)
    {
      return;
    }

    if (ledext == 0)
    {
      pixels.fill(pixels.Color(0, 255, 0));
      pixels.show(); // Send the updated pixel colors to the hardware.
      ledext = 1;
    }
    if (millis() - ledlasttim >= leddelay)
    {
      ledext = 2;
      leddelay = 3;
    }

    if (ledext == 2 && millis() - ledlasttim >= leddelay)
    {
      pixels.fill(pixels.Color(0, ledlev, 255 - ledlev));
      pixels.show(); // Send the updated pixel colors to the hardware.
      ledlev -= 25;
      ledlasttim = millis();
    }
  }

  if (ledstat == 7)
  {
    if (ledlev <= 0)
    {
      return;
    }

    if (ledext == 0)
    {
      pixels.fill(pixels.Color(255, 0, 0));
      pixels.show(); // Send the updated pixel colors to the hardware.
      ledext = 1;
    }
    if (millis() - ledlasttim >= leddelay)
    {
      ledext = 2;
      leddelay = 3;
    }

    if (ledext == 2 && millis() - ledlasttim >= leddelay)
    {
      pixels.fill(pixels.Color(ledlev, 0, 255 - ledlev));
      pixels.show(); // Send the updated pixel colors to the hardware.
      ledlev -= 25;
      ledlasttim = millis();
    }
  }

  if (ledstat == 3)
  {

    if (ledext == 0)
    {
      // red
      if (millis() - ledlasttim >= 1000)
      {
        ledext = 1;
        ledlasttim = millis();
        return;
      }
      pixels.fill(pixels.Color((int)(((double)1 - (double)fabs(((((double)millis() - (double)ledlasttim) / (double)1000) * (double)2) - (double)1)) * (double)255), 0, 0));
      pixels.show(); // Send the updated pixel colors to the hardware.
    }
    if (ledext == 1)
    {
      // green
      if (millis() - ledlasttim >= 1000)
      {
        ledext = 2;
        ledlasttim = millis();
        return;
      }
      pixels.fill(pixels.Color(0, (int)(((double)1 - (double)fabs(((((double)millis() - (double)ledlasttim) / (double)1000) * (double)2) - (double)1)) * (double)255), 0));
      pixels.show(); // Send the updated pixel colors to the hardware.
    }
    if (ledext == 2)
    {
      // blue
      if (millis() - ledlasttim >= 1000)
      {
        ledext = 0;
        ledlasttim = millis();
        return;
      }

      pixels.fill(pixels.Color(0, 0, (int)(((double)1 - (double)fabs(((((double)millis() - (double)ledlasttim) / (double)1000) * (double)2) - (double)1)) * (double)255)));
      pixels.show(); // Send the updated pixel colors to the hardware.
    }
  }
  if (ledstat == 2)
  {
    if (millis() - ledlasttim >= 500)
    {
      ledlasttim = millis();
      ledext = !ledext;
    }
    if (!ledext)
    {
      // ___---^^^
      pixels.fill(pixels.Color(0, 0, uint8_t((double(millis() - ledlasttim) / (double)500) * 255)));
      pixels.show(); // Send the updated pixel colors to the hardware.
    }
    else if (ledext)
    {
      // ^^^---___
      pixels.fill(pixels.Color(0, 0, uint8_t(255 - (double(millis() - ledlasttim) / (double)500) * 255)));
      pixels.show(); // Send the updated pixel colors to the hardware.
    }
  }
}

void beep_1() {
  beep(10, 50);

}

void beep_3() {
  beep(10, 50);
  delay(20);

  beep(10, 50);
  delay(20);

  beep(10, 50);


}
void beep_end() {
  beeptype = 10;

}
void beep_test() {
  beeptype = 0;

}

void beep8svc()
{
  if (beeptype == 10)
    return;
  if (millis() - lastbeep >= 200 && beeptype < 9)
  {
    tone(beeperpin, (beeptype + 5) * 100);
    beeptype++;
    lastbeep = millis();
  }
  if (beeptype >= 9)
  {
    noTone(beeperpin);
    beeptype++;
  }
}

void set_led_blue_breath() {
  ledstat = 2;
  ledlasttim = millis();
  ledext = 0;

}
void set_led_rgb_breath() {
  ledstat = 3;
  ledlasttim = millis();
  ledext = 0;

}
void set_led_red_yellow_loop() {
  ledstat = 10;
  ledlev = 255;
  leddelay = 600;
  ledlasttim = millis();
  ledext = 0;

}

void set_led_off() {
  ledstat = 0;
  ledext = 0;
}
void set_led_blue_keep() {
  ledstat = 11;
  ledext = 0;

}
void set_led_green_blue_loop() {
  ledstat = 8;
  ledlev = 255;
  leddelay = 600;
  ledlasttim = millis();
  ledext = 0;

}

void set_led_green_to_blue_keep() {
  ledstat = 1;
  ledlev = 255;
  leddelay = 600;
  ledlasttim = millis();
  ledext = 0;
}

void set_led_red_to_blue_keep() {
  ledstat = 7;
  ledlev = 255;
  leddelay = 600;
  ledlasttim = millis();
  ledext = 0;
}
