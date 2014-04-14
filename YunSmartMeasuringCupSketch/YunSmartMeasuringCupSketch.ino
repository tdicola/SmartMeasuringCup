// Arduino Yun Smart Measuring Cup Sketch

// Use a liquid sensor to show the volume in a measuring cup on a website in real time.

// Created by Tony DiCola
// Released under an MIT license: http://opensource.org/licenses/MIT

#include <Bridge.h>
#include <YunClient.h>
#include <YunServer.h>

// Configuration values:
#define SERIES_RESISTOR     560    // Value of the series resistor in ohms.    
#define SENSOR_PIN          0      // Analog pin which is connected to the sensor. 

// Calibration values measured from the LiquidSensor sketch. 
#define ZERO_VOLUME_RESISTANCE    0.00    // Resistance value (in ohms) when no liquid is present.
#define CALIBRATION_RESISTANCE    0.00    // Resistance value (in ohms) when liquid is at max line.
#define CALIBRATION_VOLUME        0.00    // Volume (in any units) when liquid is at max line.

#define YUN_SERVER_PORT  5678  // Internal port the YunServer instance will use for communication with Flask.
                                 // Don't change this unless you know what you're doing!

YunServer server(YUN_SERVER_PORT);
YunClient client;

void setup() {
  Serial.begin(115200);
  Bridge.begin();
  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  if (client.connected()) {
    // Handle a connected client.
    float resistance = readResistance(SENSOR_PIN, SERIES_RESISTOR);
    float volume = resistanceToVolume(resistance, ZERO_VOLUME_RESISTANCE, CALIBRATION_RESISTANCE, CALIBRATION_VOLUME);
    client.println(volume, 5);
  }
  else {
    // Look for a new connection.
    client = server.accept();
    if (client) {
      Serial.println("User is connected!");
    }
  }
  delay(200);
}

float readResistance(int pin, int seriesResistance) {
  // Get ADC value.
  float resistance = analogRead(pin);
  // Convert ADC reading to resistance.
  resistance = (1023.0 / resistance) - 1.0;
  resistance = seriesResistance / resistance;
  return resistance;
}

float resistanceToVolume(float resistance, float zeroResistance, float calResistance, float calVolume) {
  if (resistance > zeroResistance || (zeroResistance - calResistance) == 0.0) {
    // Stop if the value is above the zero threshold, or no max resistance is set (would be divide by zero).
    return 0.0;
  }
  // Compute scale factor by mapping resistance to 0...1.0+ range relative to maxResistance value.
  float scale = (zeroResistance - resistance) / (zeroResistance - calResistance);
  // Scale maxVolume based on computed scale factor.
  return calVolume * scale;
}
