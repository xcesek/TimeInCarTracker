/* ==============================================
*  Car Travel Time Tracker
*  April 2018
*  Pavol Cesek
*  See http://www.circuitbasics.com/arduino-7-segment-display-tutorial/
*  ============================================== */

#include <EEPROM.h>
#include <Time.h>
#include <TimeAlarms.h>

/* ========================================================== Global variables ========================================================== */
#define debug 1

//ABCDEFG,dp
// 2d array of segments for individual numbers
const int numeral[10] = {
  B11111100, //0
  B01100000, //1
  B11011010, //2
  B11110010, //3
  B01100110, //4
  B10110110, //5
  B10111110, //6
  B11100000, //7
  B11111110, //8
  B11100110, //9
};

//pins for decimal point and each segment
//dp, G, F, E, D, C, B, A
const int segmentPins[] = { 5, 3, 11, 7, 6, 4, 8, 12 };

//digits 1, 2, 3, 4
const int numberofDigits = 4;
const int digitPins[numberofDigits] = { 13, 10, 9, 2};

byte cur_hour;
byte cur_minute;
byte cur_second;
byte write_pos_index;
byte meta_index_updated;

/* ========================================================== Program setup ========================================================== */
void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println(F("====   Setup procedure started    ====\n"));

  initIOPins();

  randomSeed(analogRead(2));
  doCalculationsJob();

  initMemoryContext();
  initTimer();

  Serial.println(F("\n====   Setup procedure finished    ====\n"));
}

void initIOPins() {
  for (int i = 0; i < 8; i++) {
    pinMode(segmentPins[i], OUTPUT); //set segment and DP pins to output
  }
  for (int i = 0; i < numberofDigits; i++) {
    pinMode(digitPins[i], OUTPUT);
  }

  if (debug) Serial.print("IO pins initiated\n");
}
void initMemoryContext() {
  meta_index_updated = 0;
  write_pos_index = EEPROM.read(0) + 1;

  Serial.print("Last index is ");
  Serial.println(EEPROM.read(0));
  Serial.print("New index is ");
  Serial.println(write_pos_index);

  if (debug) Serial.print("EEPROM initiated\n");
  showMemory();
  //clearMemory();
}

void initTimer() {
  setTime(0, 0, 0, 1, 1, 11);
  Alarm.timerRepeat(0, 0, 1, doUpdateClockJob);   // every second
  Alarm.timerRepeat(0, 1, 0, doUpdateMemoryJob); // every minute
  Alarm.timerRepeat(0, 5, 2, doCalculationsJob); // every 5 minutes

  if (debug) Serial.print("Timers initiated\n");
}

/* ========================================================== Program main loop ========================================================== */
void loop() {
  showTime();
  processInputs();
  Alarm.delay(0);
}

/* ========================================================== Callback Functions ========================================================== */

void doUpdateClockJob() {
  cur_hour = hour();
  cur_minute = minute();
  cur_second = second();
}

// write current minute to memory
void doUpdateMemoryJob() {
  if (cur_minute > 9) {
    if (meta_index_updated == 0) { // update index only once
      EEPROM.write(0, write_pos_index);
      if (debug) Serial.print("Writing index info to EEPROM on position 0, value: ");
      if (debug) Serial.println (write_pos_index);
      meta_index_updated = 1;
    }

    // write to update position index
    EEPROM.write(write_pos_index, cur_minute);
    if (debug) Serial.print("Writing to EEPROM value");
    if (debug) Serial.print( cur_minute);
    if (debug) Serial.print(" on position ");
    if (debug) Serial.println(write_pos_index);
  }
}

// just a fancy animation
// no special meaning
void doCalculationsJob() {
  if (debug) Serial.println(F("calculations started!"));
  for (int d = 0; d < 4; d++) {
    for (int i = 0; i < 5; i++) {
      showDigitAnimated(random(1, 9), d);
      digitalWrite(digitPins[d], LOW);
    }
  }
}

/* ========================================================== Logic Functions ========================================================== */
void showTime() {
  showHour(cur_minute);
  showMinute(cur_second);
}

void showHour(int number) {
  showDigit(number % 10, 1);
  showDigit((number / 10) % 10, 0);
}

void showMinute(int number) {
  showDigit(number % 10, 3);
  showDigit((number / 10) % 10, 2);
}


void showNumber(int number) {
  if (number == 0) {
    showDigit(0, numberofDigits - 1); //display 0 in the rightmost digit
  }
  else {
    for (int digit = numberofDigits - 1; digit >= 0; digit--) {
      if (number > 0) {
        showDigit(number % 10, digit);
        number = number / 10;
      }
    }
  }
}

//Displays given number on a 7-segment display at the given digit position
void showDigit(int number, int digit) {
  digitalWrite(digitPins[digit], HIGH);
  for (int segment = 0; segment < 8; segment++) {
    boolean isBitSet = bitRead(numeral[number], segment);
    isBitSet = !isBitSet; //remove this line if common cathode display
    digitalWrite(segmentPins[segment], isBitSet);

    // blink
    if (segment == 0 && (cur_second % 2 == 0) && (digit == 1)) {
      digitalWrite(segmentPins[segment], LOW);
    }
  }
  delay(5);
  digitalWrite(digitPins[digit], LOW);
}

void showDigitAnimated(int number, int digit) {
  digitalWrite(digitPins[digit], HIGH);
  for (int segment = 0; segment < 8; segment++) {
    boolean isBitSet = bitRead(numeral[number], segment);
    isBitSet = !isBitSet; //remove this line if common cathode display
    digitalWrite(segmentPins[segment], isBitSet);
    delay(100);
  }
  delay(100);
}


void showMemory() {
  Serial.println("### MEMORY DUMP START");
  for (int i = 0; i < write_pos_index; i++) {
    Serial.print("Ind: ");
    Serial.print(i);
    Serial.print(", value: ");
    Serial.println(EEPROM.read(i));
  }
  Serial.println("### MEMORY DUMP END");
}

void clearMemory() {
  EEPROM.write(0, 0);
  for (int i = 1; i < write_pos_index; i++) {
    EEPROM.write(i, 0);
  }

}

void processInputs() {
  // todo
}
