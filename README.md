# banapass_reader_pn532
code to use pn532 to make a namco card-if device

# Hardware
you need :

1. a mcu with dual hardware UART
2. a pn532 nfc module
3. (Optional) four ws2812 led
4. (Optional) a beeper


default pin connection can be modified in the code:
```cpp
#define sysled 2      //debug led,ignore it
#define nfccommled 3  //debug led,ignore it
#define rgbpin 10     //ws2812 DIN pin
#define beeperpin 6   //beeper pin

#define LEDBrightness 255 

#define Serialto532 Serial0  //uart to pn532
#define Serialtogame Serial  //uart to game
```

# Software

install Adafruit_NeoPixel library in your arduino ide.

# Usage

this code can communicate with original namco bngrw.dll  
make sure no loader hook/emulated it.

for taiko,you can use this TAL:  
https://github.com/gyt4/tal_no_bngrw_hook
