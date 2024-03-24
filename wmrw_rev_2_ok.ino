#define sysled 2
#define nfccommled 3
#define rgbpin 10
#define beeperpin 6

#define LEDBrightness 255

#include "PeripheralDevice.h"


#define Serialto532 Serial0
#define Serialtogame Serial



class message
{
  public:
    int len = -1;
    int all_len = 0;
    uint8_t data[100];
    bool add_ack = 0;
    void initThis()
    {
      this->len = -1;
      this->all_len = 0;
      this->add_ack = 0;
      memset(data, 0, sizeof(data));
    }
    bool isReady()
    {
      if (len == 0 && all_len == 6)
        return 1;
      return len != -1 && (all_len == (len + 7));
    }
    uint8_t getCommandCode() {
      return this->data[6];
    }

};

message recv, _send;
int pass_s0 = 0;
bool zerohead = 0;
void setup()
{

  pinMode(sysled, OUTPUT);
  pinMode(nfccommled, OUTPUT);

  digitalWrite(sysled, 1);
  digitalWrite(nfccommled, 0);
  // put your setup code here, to run once:
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(LEDBrightness);
  pixels.fill(pixels.Color(100, 0, 0));
  pixels.show(); // Send the updated pixel colors to the hardware.
  beep(6, 10);
  delay(40);
  beep(6, 10);
  pixels.fill(pixels.Color(0, 0, 0));
  pixels.show(); // Send the updated pixel colors to the hardware.

  Serialtogame.begin(38400);
  Serialto532.begin(115200);
  
  pinMode(3, OUTPUT);
  recv.initThis();

  digitalWrite(nfccommled, 1);


  const uint8_t startup532[] = {
    0x55 , 0x55 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0xFF , 0x03 , 0xFD , 0xD4 , 0x14 , 0x01 , 0x17 , 0x00,
    0x00 , 0x00 , 0xff , 0x05 , 0xfb , 0xd4 , 0x14 , 0x01 , 0x14 , 0x01 , 0x02 , 0x00
  };
  while (Serialto532.available())Serialto532.read();
  for (int i = 0 ; i < 24; i++) {
    Serialto532.write(startup532[i]);
  }
  unsigned long cmdTime = millis();
  int toreadlen = 15;
  bool nfc_exist = 0;
  while (toreadlen && (millis() - cmdTime <= 2000))
  {
    if (Serialto532.available()) {
      Serialto532.read();
      toreadlen-- ;
    }
  }
  if (!toreadlen) nfc_exist = 1;
  
  for (int i = 24 ; i < 24 + 12; i++) {
    Serialto532.write(startup532[i]);
  }

  cmdTime = millis();
  toreadlen = 15;
  while (toreadlen && (millis() - cmdTime <= 50))
  {
    if (Serialto532.available()) {
      Serialto532.read();
      toreadlen-- ;
    }
  }
  int tryCnt = 0;
  while (!nfc_exist)
  {
    tryCnt++;
    digitalWrite(nfccommled, 0);
    beep(6, 10);
    delay(40);
    if (tryCnt >= 10)
    {
      ledstat = 10;
      ledlev = 255;
      leddelay = 600;
      ledlasttim = millis();
      ledext = 0;
      while (1)
      {
        ledsvc();
      }
    }
  }
  digitalWrite(nfccommled, 1);

  delay(250);
  beep(11, 80);
  //  delay(40);
  //  beep(11, 80);

  pixels.fill(pixels.Color(255, 0, 0));
  pixels.show(); // Send the updated pixel colors to the hardware.

  delay(700);
  //  ledstat = 4; ledlev = 255; leddelay = 3;
  ledstat = 3;
  ledlasttim = millis();

  _send.initThis();
  // errorHappen();
}

void errorHappen()
{
  recv.initThis();
}


void setLEDMode(uint8_t _mode) {
  //bngRwReqLED?
  if (_mode == 0x11)set_led_green_to_blue_keep();
  else if (_mode == 0x16)set_led_red_to_blue_keep();
  else if (_mode == 0x0c)set_led_green_blue_loop();
  else if (_mode == 0x00)set_led_off();
  else if (_mode == 0x05)set_led_blue_breath();
  else if (_mode == 0x0b) set_led_rgb_breath();
  else if (_mode == 0x08) set_led_red_yellow_loop();
  else if (_mode == 0x1b) set_led_blue_keep();

}
void setBEEPMode(uint8_t _mode) {
  //bngRwReqBeep?
  if (_mode == 0x81)beep_1();
  else if (_mode == 0x82)beep_3();
  else if (_mode == 0x93)beep_test();
  else if (_mode == 0x80)beep_end();

}



void proc()
{
  if (recv.len == 0)
  {
    zerohead = 1;
    recv.initThis();
    return;
  }
  bool to_pn532 = 1 ;
  //do non/dif-pn532 proc:
  //PeripheralDevice
  if (recv.getCommandCode() == 0x0E)
  { //Command Code = 0x0E(Write GPIO) -> BEEP and LED Commands
    if (recv.data[7] == 0x08) // BEEP Command
      setBEEPMode(recv.data[8]);
    if (recv.data[7] == 0x01) // LED Command
      setLEDMode(recv.data[8]);
    //all of them has a same response
    _send.add_ack = 1;
    _send.len = 2;
    _send.all_len = 9;
    uint8_t tempBuf[10] = {0x00 , 0x00 , 0xFF , 0x02 , 0xFE , 0xD5 , 0x0F , 0x1C , 0x00};
    memcpy(_send.data, tempBuf, sizeof(tempBuf));
    to_pn532 = 0;
  }

  //BngRw Commands
  if (recv.getCommandCode() == 0x18)
  { //Command Code = 0x18(Unknown Command 0x18)
    if (recv.data[7] = 0x01) // WakeUp Command
    {
      _send.add_ack = 1;
      _send.len = 2;
      _send.all_len = 9;
      uint8_t tempBuf[10] = {0x00 , 0x00 , 0xFF , 0x02 , 0xFE , 0xD5 , 0x19 , 0x12 , 0x00};
      memcpy(_send.data, tempBuf, sizeof(tempBuf));
    }
    to_pn532 = 0;
  }
  if (recv.getCommandCode() == 0x32)
  { //Command Code == 0x32(RFConfig)
    if (recv.data[7] == 0x81) {
      //(Syntax err: 0x81 is a invild item)
      _send.add_ack = 1;
      _send.len = 2;
      _send.all_len = 9;
      uint8_t tempBuf[10] = {0x00 , 0x00 , 0xFF , 0x02 , 0xFE , 0xD5 , 0x33 , 0xF8 , 0x00};
      memcpy(_send.data, tempBuf, sizeof(tempBuf));
      to_pn532 = 0;
    }
  }
  if (recv.getCommandCode() == 0x06)
  { //Command Code == 0x06(ReadRegister)
    if (recv.len == 0x12) {
      //BngRwSpecialRegisterLen0x12
      _send.add_ack = 1;
      _send.len = 0x0A;
      _send.all_len = 17;
      uint8_t tempBuf[20] = {0x00, 0x00, 0xFF, 0x0A, 0xF6, 0xD5, 0x07, 0xFF, 0x3F, 0x0E, 0xF1, 0xFF, 0x3F, 0x0E, 0xF1, 0xAA, 0x00};
      memcpy(_send.data, tempBuf, sizeof(tempBuf));
      to_pn532 = 0;
    }
    if (recv.len == 0x18) {
      //BngRwSpecialRegisterLen0x18
      _send.add_ack = 1;
      _send.len = 0x0D;
      _send.all_len = 20;
      uint8_t tempBuf[20] = { 0x00, 0x00, 0xFF, 0x0D, 0xF3, 0xD5, 0x07, 0xDC, 0xF4, 0x3F, 0x11, 0x4D, 0x85, 0x61, 0xF1, 0x26, 0x6A, 0x87, 0xC9, 0x00};
      memcpy(_send.data, tempBuf, sizeof(tempBuf));
      to_pn532 = 0;
    }

  }
  if (recv.getCommandCode() == 0x0C)
  { //Command Code == 0x0C(ReadGPIO)
    //BngRwSpecialGPIO
    _send.add_ack = 1;
    _send.len = 0x05;
    _send.all_len = 12;
    uint8_t tempBuf[15] = {0x00 , 0x00 , 0xFF , 0x05 , 0xFB , 0xD5 , 0x0D , 0x20 , 0x06 , 0x00 , 0xF8 , 0x00};
    memcpy(_send.data, tempBuf, sizeof(tempBuf));
    to_pn532 = 0;
  }
  if (recv.getCommandCode() == 0x12)
  { //Command Code == 0x12(SetParameters)
    //BngRwSpecialParam
    _send.add_ack = 1;
    _send.len = 0x02;
    _send.all_len = 9;
    uint8_t tempBuf[15] = {0x00 , 0x00 , 0xFF , 0x02 , 0xFE , 0xD5 , 0x13 , 0x18 , 0x00};
    memcpy(_send.data, tempBuf, sizeof(tempBuf));
    to_pn532 = 0;
  }
  if (recv.getCommandCode() == 0x08)
  { //Command Code == 0x08(Write Register)
    if (recv.data[7] == 0xFF) {
      //(Syntax err: Unknown SFR address)
      _send.add_ack = 1;
      _send.len = 3;
      _send.all_len = 10;
      uint8_t tempBuf[10] = {0x00 , 0x00 , 0xFF , 0x03 , 0xFD , 0xD5 , 0x09 , 0x00 , 0x22 , 0x00 };
      memcpy(_send.data, tempBuf, sizeof(tempBuf));
      to_pn532 = 0;
    }

  }
  if (recv.getCommandCode() == 0x08)
  { //Command Code == 0x08(Write Register)
    if (recv.data[7] == 0xFF) {
      //(Syntax err: Unknown SFR address)
      _send.add_ack = 1;
      _send.len = 3;
      _send.all_len = 10;
      uint8_t tempBuf[10] = {0x00 , 0x00 , 0xFF , 0x03 , 0xFD , 0xD5 , 0x09 , 0x00 , 0x22 , 0x00 };
      memcpy(_send.data, tempBuf, sizeof(tempBuf));
      to_pn532 = 0;
    }
    if (recv.data[7] == 0x63) {
      //clear cardreading service

      _send.add_ack = 1;
      _send.len = 3;
      _send.all_len = 10;
      uint8_t tempBuf[10] = {0x00 , 0x00 , 0xFF , 0x03 , 0xFD , 0xD5 , 0x09 , 0x00 , 0x22 , 0x00 };
      memcpy(_send.data, tempBuf, sizeof(tempBuf));
      pass_s0 = 9 + 6;
    }
  }
  if (recv.getCommandCode() == 0x52)
  { //Command Code == 0x52(InRelease)

    for (int i = 0 ; i < recv.all_len; i++) {
      Serialto532.write(recv.data[i]);
    }
    uint8_t temp_532_data[10 + 6];
    int to_recv_532_len = 10 + 6;
    int recved_532_len = 0;
    while (to_recv_532_len) {
      while (to_recv_532_len && Serialto532.available()) {
        temp_532_data[recved_532_len++] = Serialto532.read();
        to_recv_532_len --;
      }
    }
    _send.add_ack = 1;
    _send.len = 4;
    _send.all_len = 11;
    if (temp_532_data[13] == 0x00) {
      uint8_t tempBuf[11] = {0x00 , 0x00 , 0xFF , 0x04 , 0xFC , 0xD5 , 0x53 , 0x01 , 0x00 , 0xD7 , 0x00 };
      memcpy(_send.data, tempBuf, sizeof(tempBuf));
    }
    else if (temp_532_data[13] == 0x27) {
      uint8_t tempBuf[11] = {0x00 , 0x00 , 0xFF , 0x04 , 0xFC , 0xD5 , 0x53 , 0x01 , 0x27 , 0xB0 , 0x00 };
      memcpy(_send.data, tempBuf, sizeof(tempBuf));
    }
    to_pn532 = 0;
  }

  if (recv.getCommandCode() == 0x44)
  { //Command Code == 0x44(InDeselect)

    for (int i = 0 ; i < recv.all_len; i++) {
      Serialto532.write(recv.data[i]);
    }
    uint8_t temp_532_data[10 + 6];
    int to_recv_532_len = 10 + 6;
    int recved_532_len = 0;
    while (to_recv_532_len) {
      while (to_recv_532_len && Serialto532.available()) {
        temp_532_data[recved_532_len++] = Serialto532.read();
        to_recv_532_len --;
      }
    }
    _send.add_ack = 1;
    _send.len = 4;
    _send.all_len = 11;
    if (temp_532_data[13] == 0x00) {
      uint8_t tempBuf[11] = {0x00 , 0x00 , 0xFF , 0x04 , 0xFC , 0xD5 , 0x45 , 0x01 , 0x00 , 0xE5 , 0x00 };
      memcpy(_send.data, tempBuf, sizeof(tempBuf));
    }
    else  if (temp_532_data[13] == 0x27) {
      uint8_t tempBuf[11] = {0x00 , 0x00 , 0xFF , 0x04 , 0xFC , 0xD5 , 0x45 , 0x01 , 0x27 , 0xBF , 0x00 };
      memcpy(_send.data, tempBuf, sizeof(tempBuf));
    }
    to_pn532 = 0;
  }


  if (recv.getCommandCode() == 0xA0)
  {
    //Command Code == 0xA0 ReadFelicaCardbySonyRCS620s?
    //00 00 ff %Len=15% %lensum=EB% D4 [40] %Tg:01% [Raw Data:[Data len(cmdlen+1)][cmd]] %checksum% 00
    //send to 532
    uint8_t to_532_data[] = {0x00 , 0x00 , 0xFF , 0x15 , 0xEB , 0xD4 , 0x40 , 0x01,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0xCC , 0x00
                            };
    uint8_t sum = 0;
    for (int i = 0 ; i < 18; i++)to_532_data[8 + i] = recv.data[9 + i]; //copy felica len+cmd raw data
    for (int i = 0; i < 26; i++) sum += to_532_data[i];// calc checksum step 1
    to_532_data[26] = (uint16_t)0xFF - (uint16_t)sum; // calc checksum step 2
    for (int i = 0; i < 28; i++) {
      Serialto532.write(to_532_data[i]); // send to 532}
      //      Serial.write(to_532_data[i]);debug
    }

    uint8_t temp_532_data[0x30 + 7 + 6];
    int to_recv_532_len = 0x30 + 7 + 6;
    bool card_lost = 0 ;
    unsigned long begin_time = millis();
    int recved_532_len = 0;
    while (to_recv_532_len) {
      ledsvc();
      if (millis() - begin_time >= 250) {
        card_lost = 1;
        break;
      }
      while (to_recv_532_len && Serialto532.available()) {
        temp_532_data[recved_532_len++] = Serialto532.read();
        if (recved_532_len == 10 && temp_532_data[9] == 0x03) {
          card_lost = 1;
          to_recv_532_len -= 0x30 - 0x03;
        }
        to_recv_532_len --;
      }
    }
    if (card_lost) {
      _send.add_ack = 1;
      _send.len = 3;
      _send.all_len = 10;
      uint8_t tempBuf[11] = {0x00 , 0x00 , 0xFF , 0x03 , 0xFD , 0xD5 , 0xA1 , 0x01 , 0x89 , 0x00 };
      memcpy(_send.data, tempBuf, sizeof(tempBuf));
    }
    else {
      temp_532_data[12] += 0x60;
      temp_532_data[0x030 + 7 + 6 - 2] -= 0x60;
      _send.add_ack = 0;
      _send.len = 0x30 + 6;
      _send.all_len = 0x30 + 7 + 6;

      memcpy(_send.data, temp_532_data, sizeof(temp_532_data));
    }
    to_pn532 = 0;
  }
  if (to_pn532) {
    for (int i = 0 ; i < recv.all_len; i++) {
      Serialto532.write(recv.data[i]);
    }
  }


  recv.initThis();

}

void doRecv() {

  if (Serialtogame.available())
  {
    uint8_t recvByte = Serialtogame.read();

    if (recv.all_len == 0 && recvByte != 0x00)
    {
      recv.initThis();
    }
    else if (recv.all_len < 3)
    { //package head
      recv.data[recv.all_len++] = recvByte;
    }
    else if (recv.all_len == 3)
    { //package length
      recv.data[recv.all_len++] = recvByte;
      recv.len = recv.data[3];
      if (!(recv.data[0] == 0 && recv.data[1] == 0 && recv.data[2] == 0xFF))
      {
        errorHappen();
      }
    }
    else
    { //normal data
      recv.data[recv.all_len++] = recvByte;
      if (recv.isReady())
      {
        proc();
      }
    }
  }


}

void doSend() {
  if (_send.isReady()) {
    if (_send.add_ack) {
      uint8_t str[10] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
      for (int i = 0; i < 6; i++)
        Serialtogame.write((byte)str[i]);
    }
    for (int i = 0 ; i < _send.all_len; i++)
      Serialtogame.write(_send.data[i]);
  }
  _send.initThis();
}

void loop()
{
  ledsvc();
  beep8svc();
  doRecv();
  doSend();
  if (pass_s0) {
    while (Serialto532.available() && pass_s0) {
      Serialto532.read();
      pass_s0--;
    }
  }
  else {
    while (Serialto532.available())Serial.write(Serialto532.read());
  }
}
