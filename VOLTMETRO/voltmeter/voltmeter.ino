#include <SevSeg.h>
#define analogInput A0

int quattro = 2;
int G = 3;
int C = 4;
int punto = 5;
int D = 6;
int E = 7;
int B = 8;
int tre = 9;
int due = 10;
int F = 11;
int A = 12;
int uno = 13;

SevSeg sevseg;

float R1 = 100000.00;
float R2 = 10000.00;
float voltage = 0.0;

int cont = 1000;

void setup() {
  pinMode(analogInput, INPUT);
  Serial.begin(115200);
  analogReference(DEFAULT);
  byte numDigits = 4;
  byte digitPins[] = {uno, due, tre, quattro};
  byte segmentPins[] = {A, B, C, D, E, F, G, punto};
  bool resistorOnSegments = false;
  byte hardwareConfig = COMMON_CATHODE;
  bool updateWithDelays = false;
  bool leadingZeros = false;
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins,
               resistorOnSegments, updateWithDelays, leadingZeros);
  sevseg.setBrightness(90);
}

float readVoltage(int pin) {
  float sum = 0;
  for (int i = 0; i < 100; i++)
    sum += analogRead(pin);
  float val = sum / 100;
  float Vout = (val * 4.55) / 1024.00;
  float Vin = Vout / (R2 / (R1 + R2)) * 0.8;
  if (Vin < 0.09)
    Vin = 0.00;
  return Vin;
}

void loop() {
  if (cont == 1000) {
    voltage = readVoltage(analogInput);
    cont = 0;
  }

  if (voltage < 10)
    sevseg.setNumber(voltage * 1000, 3);
  if (voltage > 9 && voltage < 100)
    sevseg.setNumber(voltage * 100, 2);
  if (voltage > 99 && voltage < 1000)
    sevseg.setNumber(voltage * 10, 1);
  sevseg.refreshDisplay();
  Serial.println(voltage);
  cont++;
}
