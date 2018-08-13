#include <SPI.h>

/**
 * Micra K11 Tachometer
 * 
 * 500 to 4000 rpm = 7 green leds
 * 4500 to 5500 rpm = 3 orange leds
 * 6000 to 7000 rpm = 3 red leds
 * 7500 to 8000 = unused
 * 
 * shift light = 1 blue led
 */

#define DATA_PIN 3
#define INTERRUPT_PIN 1
#define LED_SHIFT_LIGHT_PIN 2
#define LATCH_PIN 10
#define CLEAR_PIN 8


/*----------------------------------------*/


const byte SHIFT_REGISTERS_COUNT = 2;
const byte LEDS_COUNT = SHIFT_REGISTERS_COUNT * 8;

const int LED_ILLUMINATE_RPM[LEDS_COUNT] = {500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000};

const int REV_LIMITER_RPM = 6500; //ecu limiter 6650rpm
const int SHIFT_LIGHT_RPM = 4000;

const byte NUMBER_OF_CYLINDERS = 4;
const byte LED_UPDATE_INTERVAL = 80;

/**
 * The Micra has a 4 stroke engine firing twice every rev.
 * This snippet, with thanks to https://github.com/deepsyx/arduino-tachometer
 * calculates the number of fires per revolution for any number of cylinders.
 */
const int FIRES_PER_REV = (360 / (720 / NUMBER_OF_CYLINDERS));

SPISettings tpic6c595(10000000UL, MSBFIRST, SPI_MODE0);


/*----------------------------------------*/

unsigned long lastUpdateTime = 0;

int sparkCount = 0;

int lastRpmValue = 0;

bool revLimiterBlink = false;

int ledPattern = 0b0000000000000000;

/*----------------------------------------*/


void setLedState(int rpm, bool limit = true, bool shift = true);

/**
 * Interrupt increment spark count.
 */
void incrementSparkCount() {
    sparkCount++;
}

void transferLedPattern() {

    SPI.beginTransaction(tpic6c595);
    digitalWrite(LATCH_PIN, LOW);

    SPI.transfer(highByte(ledPattern));
    SPI.transfer(lowByte(ledPattern));

    digitalWrite(LATCH_PIN, HIGH);
    SPI.endTransaction();

    Serial.print("LED pattern: ");
    Serial.println(String(ledPattern, BIN));
}

/**
 * Switch all leds on or off.
 */
void setGlobalState(bool state) {

    setShiftLightState(state);

    for (int i = 0; i < LEDS_COUNT; i++) {
        bitWrite(ledPattern, i, state);
    }

    Serial.print("\n");
    Serial.print("Global state: ");
    Serial.println(state);

    transferLedPattern();
}

/**
 * Switch on or off the shift light.
 */
void setShiftLightState(bool state) {
    digitalWrite(LED_SHIFT_LIGHT_PIN, state);
}

/**
 * Cycle the led state.
 */
void cycleLedState() {

    Serial.print("\n");
    Serial.println("Ramping up...");
    Serial.print("\n");

    for (int i = 0; i < LEDS_COUNT; i++) {
        setLedState(LED_ILLUMINATE_RPM[i], false, false);
        delay(LED_UPDATE_INTERVAL);
    }

    Serial.print("\n");
    Serial.println("Hitting limiter...");

    for (int i = 0; i <= LEDS_COUNT; i++) {
        setLedState(REV_LIMITER_RPM, true, false);
        delay(LED_UPDATE_INTERVAL);
    }

    Serial.print("\n");
    Serial.println("Ramping down...");
    Serial.print("\n");

    for (int i = LEDS_COUNT - 1; i >= 0; i--) {
        setLedState(LED_ILLUMINATE_RPM[i], false, false);
        delay(LED_UPDATE_INTERVAL);
    }

    setGlobalState(LOW);

    Serial.print("\n-----------\n\n");
}

/**
 * Illuminate leds based on RPM.
 */
void setLedState(int rpm, bool limit, bool shift) {

    //blink when rev limiter hit
    if (limit && rpm >= REV_LIMITER_RPM) {
        setGlobalState(revLimiterBlink);
        revLimiterBlink = !revLimiterBlink;
        return;
    }

    //illuminate shift light at set rpm
    setShiftLightState(shift && rpm >= SHIFT_LIGHT_RPM);

    //set led pattern based on rpm
    for (int i = 0; i < LEDS_COUNT; i++) {
        if (rpm >= LED_ILLUMINATE_RPM[i]) {
            bitWrite(ledPattern, i, 1);
        } else {
            bitWrite(ledPattern, i, 0);
        }
    }

    transferLedPattern();
}

/**
 * Setup and run through startup sequence.
 */
void setup() {

    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLEAR_PIN, OUTPUT);

    digitalWrite(LATCH_PIN, HIGH);
    digitalWrite(CLEAR_PIN, LOW);
    digitalWrite(CLEAR_PIN, HIGH);

    pinMode(LED_SHIFT_LIGHT_PIN, OUTPUT);

    //initialise serial
    Serial.begin(9600);

    //initialise spi
    SPI.begin();

    //run though startup sequence
    setShiftLightState(HIGH);
    delay(100);
    setShiftLightState(LOW);
    delay(100);
    setShiftLightState(HIGH);
    delay(100);
    setShiftLightState(LOW);
    delay(200);

    cycleLedState();

    //ready for data
    pinMode(DATA_PIN, INPUT_PULLUP);
    attachInterrupt(INTERRUPT_PIN, incrementSparkCount, FALLING);
}

/**
 * Update leds at configured interval based on rpm.
 */
void loop() {

    if ((millis() - lastUpdateTime) >= LED_UPDATE_INTERVAL) {

        //calculate the current rpm by multiplying the number of sparks in one update interval
        //by the number of intervals per second - finding the number of sparks in one second
        int sparksPerSecond = sparkCount * (1000 / LED_UPDATE_INTERVAL);

        //then multiply by 60 to find sparks in one minute
        int sparksPerMinute = sparksPerSecond * 60;

        //finally, divide by the number of fires per revolution to find the current rpm.
        int currentRpm = sparksPerMinute / FIRES_PER_REV;

        //find an average between the current and last rpm to provide a little smoothing
        int smoothedRpm = (currentRpm + lastRpmValue) / 2;

        setLedState(smoothedRpm);

        Serial.print("Smoothed RPM: ");
        Serial.println(smoothedRpm);
        Serial.println("\n");

        lastRpmValue = currentRpm;

        sparkCount = 0;
        lastUpdateTime = millis();
    }
}
