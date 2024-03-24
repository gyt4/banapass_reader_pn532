# banapass_reader_pn532
code to use pn532 to make a namco card-if device
based on captured packets from namco's original card reader by my self.

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

# How I made this

I bought a original namco reader,connect it to game with a RS232 adapter and a 12v power supplier.Then,capture the serial packets.  
I guess the structure of the packets,and try to send every of it to the reader manualy,get the response and respond in leds,rf and beeper.   
Then i found it is similar to PN532 packets(the original reader uses sony rcs620s,whos ).Every commands can be explain with PN532's packet structure,except LEDS ,beeper,and some special command.  
Just send most commands directly to pn532,receive pn532's response to send it to game,translate some sony rcs620s command(the original reader use it),and manually process leds,beeper and some special command... 
and it is done.



