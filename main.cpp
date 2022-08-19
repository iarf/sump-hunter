#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

int stickX; // left-right
int stickY; // forward-back
int stickZ; // up-down

int stickX_baseline;
int stickY_baseline;
int stickZ_baseline;

float forwardProportion;
float directionalProportion;

int forwardBias;
int directionalBias;
int baseThrust;
int leftAdjustment;
int rightAdjustment;

int leftInput;
int rightInput;

int LEFT_1 = D3;
int LEFT_2 = D4;
int LEFT_ENA = D5;

int RIGHT_1 = D6;
int RIGHT_2 = D7;
int RIGHT_ENA = D8;

int UP_1 = (uint8_t)3U; // rx
int UP_2 = (uint8_t)1U; // tx
int UP_ENA = D0;

void setup()
{
    Serial.begin(115200);

    if (!ads.begin())
    {
        Serial.println("Failed to initialize ADS.");
        while (1)
            ;
    }
    ads.setGain(GAIN_TWOTHIRDS);

    pinMode(LEFT_1, OUTPUT);
    pinMode(LEFT_2, OUTPUT);
    pinMode(LEFT_ENA, OUTPUT);

    digitalWrite(LEFT_1, LOW);
    digitalWrite(LEFT_2, LOW);

    // calibrate joysticks
    stickX_baseline = ads.readADC_SingleEnded(1);
    stickY_baseline = ads.readADC_SingleEnded(0);
    stickZ_baseline = ads.readADC_SingleEnded(2);
}

void loop()
{
    stickX = ads.readADC_SingleEnded(1);
    stickY = ads.readADC_SingleEnded(0);
    stickZ = ads.readADC_SingleEnded(2);

    if (stickY < stickY_baseline + 100 && stickY > stickY_baseline - 100)
    {
        forwardBias = 0;
    }
    else
    {
        forwardBias = (stickY - stickY_baseline);
    }
    forwardProportion = (float)forwardBias / stickY_baseline;
    if (forwardProportion > 1)
    {
        forwardProportion = 1;
    }
    if (forwardProportion < -1)
    {
        forwardProportion = -1;
    }
    baseThrust = 1023 * forwardProportion;

    if (stickX < stickX_baseline + 100 && stickX > stickX_baseline - 100)
    {
        directionalBias = 0;
    }
    else
    {
        directionalBias = stickX - stickX_baseline; // negative is left, positive is right
    }

    directionalProportion = (float)directionalBias / stickX_baseline;

    leftAdjustment = 0 - 1023 * directionalProportion;
    rightAdjustment = 1023 * directionalProportion;

    // express horizontal motor inputs within range -1023 to 1023
    leftInput = baseThrust + leftAdjustment;
    if (leftInput > 1023)
    {
        leftInput = 1023;
    }
    else if (leftInput < -1023)
    {
        leftInput = -1023;
    }
    rightInput = baseThrust + rightAdjustment;
    if (rightInput > 1023)
    {
        rightInput = 1023;
    }
    else if (rightInput < -1023)
    {
        rightInput = -1023;
    }

    // apply input to motors
    setThruster(0, leftInput);
    setThruster(1, rightInput);
}

// power is an int between -1023 and +1023. +-600 is a decent baseline
void setThruster(int8 thruster, int power)
{
    int pin_1;
    int pin_2;
    int pin_ena;

    switch (thruster)
    {
    case 0: // left
        pin_1 = LEFT_1;
        pin_2 = LEFT_2;
        pin_ena = LEFT_ENA;
        break;
    case 1: // right
        pin_1 = RIGHT_1;
        pin_2 = RIGHT_2;
        pin_ena = RIGHT_ENA;
        break;
    case 3: // vertical
        pin_1 = UP_1;
        pin_2 = UP_2;
        pin_ena = UP_ENA;
    default:
        Serial.print("tried to set speed for invalid thruster: ");
        Serial.println(thruster);
        return;
    }

    if (power > 0)
    {
        digitalWrite(pin_1, 0);
        digitalWrite(pin_2, 1);
    }
    else if (power < 0)
    {
        digitalWrite(pin_1, 1);
        digitalWrite(pin_2, 0);
    }
    else
    {
        digitalWrite(pin_1, 0);
        digitalWrite(pin_2, 0);
    }

    int speed = abs(power);
    Serial.println(speed);

    analogWrite(pin_ena, speed);
}