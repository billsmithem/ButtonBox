//BUTTON BOX
//USE w ProMicro
//Tested in WIN10 + Assetto Corsa
//AMSTUDIO
//20.8.17
// YouTube video: https://www.youtube.com/watch?v=Z7Sc4MJ8RPM

// 10/06/2019
// Bill Smithem
// Added additional button
// 4/4/2020
// Bill Smithem
// set debounce time to 50 (default was 10)
// 01/21/2021
// Bill Smtihem
// add "4 func" rotaries - different code sent if button pressed while turning. Tracked state of buttons 21-24
//  and used that to "shift" the rotary encoders. Note: we're still sending the button presses for the rotary
//  encoders, so mind that when assigning buttons in game.
// Number of "buttons" goes from 33 to 41

#include <Arduino.h>
#include <Keypad.h>
#include <Joystick.h>

#define ENABLE_PULLUPS
#define NUMROTARIES 4
#define NUMBUTTONS 25
#define NUMROWS 5
#define NUMCOLS 5

// Odd map is so buttons come out in numberical order, not scanning order. i.e. top left button is one, next is
//  two, etc.
byte buttons[NUMROWS][NUMCOLS] = {
  {5, 6, 17, 23, 12},
  {3, 4, 16, 22, 11},
  {9, 10, 19, 20, 14},
  {1, 2, 15, 21, 0},
  {7, 8, 18, 24, 13},
};

struct rotariesdef {
  byte pin1;
  byte pin2;
  int ccwchar;      // send this for CCW
  int cwchar;       //  this for CW
  int ccwPchar;     //  this for CCW with button pressed
  int cwPchar;      //  and this for CW with button pressed
  volatile unsigned char state;
};

rotariesdef rotaries[NUMROTARIES] {
  {0, 1, 32, 31, 40, 39, 0},
  {2, 3, 29, 30, 38, 37, 0},
  {4, 5, 27, 28, 36, 35, 0},
  {6, 7, 25, 26, 34, 33, 0},
};

#define DIR_CCW 0x10
#define DIR_CW 0x20
#define R_START 0x0

#ifdef HALF_STEP
#define R_CCW_BEGIN 0x1
#define R_CW_BEGIN 0x2
#define R_START_M 0x3
#define R_CW_BEGIN_M 0x4
#define R_CCW_BEGIN_M 0x5
const unsigned char ttable[6][4] = {
  // R_START (00)
  {R_START_M,            R_CW_BEGIN,     R_CCW_BEGIN,  R_START},
  // R_CCW_BEGIN
  {R_START_M | DIR_CCW, R_START,        R_CCW_BEGIN,  R_START},
  // R_CW_BEGIN
  {R_START_M | DIR_CW,  R_CW_BEGIN,     R_START,      R_START},
  // R_START_M (11)
  {R_START_M,            R_CCW_BEGIN_M,  R_CW_BEGIN_M, R_START},
  // R_CW_BEGIN_M
  {R_START_M,            R_START_M,      R_CW_BEGIN_M, R_START | DIR_CW},
  // R_CCW_BEGIN_M
  {R_START_M,            R_CCW_BEGIN_M,  R_START_M,    R_START | DIR_CCW},
};
#else
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6

const unsigned char ttable[7][4] = {
  // R_START
  {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
  // R_CW_FINAL
  {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
  // R_CW_BEGIN
  {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
  // R_CW_NEXT
  {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
  // R_CCW_BEGIN
  {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
  // R_CCW_FINAL
  {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
  // R_CCW_NEXT
  {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};
#endif

byte rowPins[NUMROWS] = {21, 20, 19, 18, 15};
byte colPins[NUMCOLS] = {14, 16, 10, 9, 8};

Keypad buttbx = Keypad( makeKeymap(buttons), rowPins, colPins, NUMROWS, NUMCOLS);

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                   JOYSTICK_TYPE_JOYSTICK, 41, 0,
                   false, false, false, false, false, false,
                   false, false, false, false, false);

void setup() {
  void rotary_init();

  Joystick.begin();
  rotary_init();
  buttbx.setDebounceTime(50);
}

void loop() {
  void CheckAllEncoders();
  void CheckAllButtons();

  CheckAllEncoders();

  CheckAllButtons();

}

// keep state of last four buttons (ones attached to rotaries)
KeyState rbState[] = { RELEASED, RELEASED, RELEASED, RELEASED };

void CheckAllButtons(void) {
  int kc;

  if (buttbx.getKeys())
  {
    for (int i = 0; i < LIST_MAX; i++)
    {
      if ( buttbx.key[i].stateChanged )
      {
        kc = buttbx.key[i].kchar - 21;        // track state of buttons 21-24 (switches on rotaries)
        switch (buttbx.key[i].kstate) {
        case PRESSED:
        case HOLD:
          Joystick.setButton(buttbx.key[i].kchar, 1);
          if (kc >= 0)
            rbState[kc] = PRESSED;
          break;
        case RELEASED:
        case IDLE:
          Joystick.setButton(buttbx.key[i].kchar, 0);
          if (kc >= 0)
            rbState[kc] = RELEASED;
          break;
        }
      }
    }
  }
}


void rotary_init() {
  for (int i = 0; i < NUMROTARIES; i++) {
    pinMode(rotaries[i].pin1, INPUT);
    pinMode(rotaries[i].pin2, INPUT);
#ifdef ENABLE_PULLUPS
    digitalWrite(rotaries[i].pin1, HIGH);
    digitalWrite(rotaries[i].pin2, HIGH);
#endif
  }
}


unsigned char rotary_process(int _i) {
  unsigned char pinstate = (digitalRead(rotaries[_i].pin2) << 1) | digitalRead(rotaries[_i].pin1);
  rotaries[_i].state = ttable[rotaries[_i].state & 0xf][pinstate];
  return (rotaries[_i].state & 0x30);
}

void CheckAllEncoders(void) {
  for (int i = 0; i < NUMROTARIES; i++) {
    unsigned char result = rotary_process(i);
    if (result == DIR_CCW) {
      // check if button pressed
      if (rbState[3-i] == PRESSED) {
        Joystick.setButton(rotaries[i].ccwPchar, 1); delay(50); Joystick.setButton(rotaries[i].ccwPchar, 0);
      } else {
        Joystick.setButton(rotaries[i].ccwchar, 1); delay(50); Joystick.setButton(rotaries[i].ccwchar, 0);
      }
    }
    if (result == DIR_CW) {
      if (rbState[3-i] == PRESSED) {
        Joystick.setButton(rotaries[i].cwPchar, 1); delay(50); Joystick.setButton(rotaries[i].cwPchar, 0);
      } else {
        Joystick.setButton(rotaries[i].cwchar, 1); delay(50); Joystick.setButton(rotaries[i].cwchar, 0);
      }
    }
  }
}

