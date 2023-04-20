#include "BMP085.h"
#include <Wire.h>
#include <DHT.h>

// DHT SETUP
#define DHTPIN PB12
#define DHTTYPE DHT21
DHT dht(DHTPIN, DHTTYPE);

float dht_h, dht_t;

// BM180 SETUP
float temperature, pressure, atm, altitude;
BMP085 myBarometer;

// LORA SETUP
#define FREQUENCY 470000000
#define ADDRESS 1
String base_command = "AT+SEND=10,";
String CRLF = "\r\n";
String separator = "#";


// DIRT SENSOR SETUP
#define terrain1 PB0
#define terrain2 PB1

// MQ130 SENSOR SETUP
#define _MQ135 PA2

void setup() {
  Serial1.begin(115200);
  delay(100);

  String settings = "AT+BAND=" + String(FREQUENCY) + CRLF;
  Serial1.write(settings.c_str());
  delay(100);

  settings = "AT+ADDRESS=" + String(ADDRESS) + CRLF;
  Serial1.write(settings.c_str());
  delay(100);

  dht.begin();
  myBarometer.init();

  pinMode(terrain1, INPUT_ANALOG);
  pinMode(terrain2, INPUT_ANALOG);
  pinMode(_MQ135, INPUT_ANALOG);
  analogReadResolution(12);

}

String build_command(String s) {
  return String(base_command + String(s.length()) + "," + s + "\r\n");
}

int calc_int_avg(float value1, float value2) {
  return (int)((value1 + value2) / 2);
}

void loop() {
  String data = "";
  float dht_h = dht.readHumidity();
  float dht_t = dht.readTemperature();

  float bm180_t = myBarometer.bmp085GetTemperature(myBarometer.bmp085ReadUT());
  float bm180_p = myBarometer.bmp085GetPressure(myBarometer.bmp085ReadUP());
  atm = bm180_p / 101325;

  int thum1 = (float)(analogRead(terrain1) / 4095) * 100;
  int thum2 = (float)(analogRead(terrain2) / 4095) * 100;

  float sensor_volt = analogRead(_MQ135);

  //  temperature
  if (!isnan(dht_t)) {
    int temperature = calc_int_avg(bm180_t, dht_t);
    data = data + (int)temperature + separator;
  }
  else
    data = data + "x" + separator;

  //  humidity
  if (!isnan(dht_h))
    data = data + (int)dht_h + separator;
  else
    data = data + "x" + separator;

  // pressure
  data = data + (int)bm180_p + separator;

  // terrain 1 humidity
  data = data + (int)thum1 + separator;

  // terrain 2 humidity
  data = data + (int)thum2 + separator;

  // mq135 gas sensor
  data = data + (int)sensor_volt + separator;


  // send data
  String command = build_command(data);
  Serial1.write(command.c_str());

  delay(1000);
}
