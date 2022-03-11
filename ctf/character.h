#ifndef CHARACTER_H

#define CHARACTER_H

#include <Arduino.h>
#include <LiquidCrystal.h>

byte beginBarre[8] = {
  0b00001,
  0b00001,
  0b00001,
  0b00001,
  0b00001,
  0b00001,
  0b00001,
};

byte middleBarre0[8] = {
  0b11111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111,
};

byte middleBarre1[8] = {
  0b11111,
  0b00000,
  0b10000,
  0b10000,
  0b10000,
  0b00000,
  0b11111,
};

byte middleBarre2[8] = {
  0b11111,
  0b00000,
  0b11000,
  0b11000,
  0b11000,
  0b00000,
  0b11111,
};

byte middleBarre3[8] = {
  0b11111,
  0b00000,
  0b11100,
  0b11100,
  0b11100,
  0b00000,
  0b11111,
};

byte middleBarre4[8] = {
  0b11111,
  0b00000,
  0b11110,
  0b11110,
  0b11110,
  0b00000,
  0b11111,
};

byte middleBarre5[8] = {
  0b11111,
  0b00000,
  0b11111,
  0b11111,
  0b11111,
  0b00000,
  0b11111,
};

byte endBarre[8] = {
  0b10000,
  0b10000,
  0b10000,
  0b10000,
  0b10000,
  0b10000,
  0b10000,
};

byte * character[8] = {beginBarre, middleBarre0, middleBarre1, middleBarre2, middleBarre3, middleBarre4, middleBarre5, endBarre};

void loadCharacter(LiquidCrystal lcd) {
  for (size_t i=0; i<8; i++)
    lcd.createChar(i, character[i]);
}

#endif
