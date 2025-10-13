#define LIGHT_SENSOR_PIN A0
#define BUZZER_PIN 5

// Thresholds for guessing number of LEDs illuminated (calibrate for your room and LED brightness)
int ledThreshold1 = 300; // Minimum for 1 LED illuminated
int ledThreshold2 = 600; // Minimum for 2 LEDs illuminated
int ledThreshold3 = 900; // Minimum for 3 LEDs illuminated

// Timing
#define SAMPLE_INTERVAL 1000      // 1 second in ms
#define HOUR_INTERVAL 3600000UL   // 1 hour in ms
#define MAX_SAMPLES 3600          // 3600 samples per hour (every second)

unsigned int lightSamples[MAX_SAMPLES];
unsigned int sampleCount = 0;
unsigned long lastSampleTime = 0;
unsigned long lastHourTime = 0;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(9600);
  lastSampleTime = millis();
  lastHourTime = millis();
}

void loop() {
  unsigned long now = millis();

  // Sample light every second and store in array
  if (now - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = now;
    int lightValue = analogRead(LIGHT_SENSOR_PIN);
    if (sampleCount < MAX_SAMPLES) {
      lightSamples[sampleCount++] = lightValue;
    }
    // Print estimated number of illuminated LEDs to Serial
    int nLEDs = estimateLEDs(lightValue);
    Serial.print("Light value: ");
    Serial.print(lightValue);
    Serial.print(" | Estimated LEDs ON: ");
    Serial.println(nLEDs);
  }

  // Every hour, beep 0-3 times based on average light level
  if (now - lastHourTime >= HOUR_INTERVAL) {
    lastHourTime = now;
    int avg = 0;
    if (sampleCount > 0) {
      for (unsigned int i = 0; i < sampleCount; i++) {
        avg += lightSamples[i];
      }
      avg /= sampleCount;
    }
    int beepCount = estimateLEDs(avg);

    // Serial report
    Serial.print("AVERAGE light value for last hour: ");
    Serial.print(avg);
    Serial.print(" | Beeping ");
    Serial.print(beepCount);
    Serial.println(" times.");

    // Beep commensurate times
    for (int i = 0; i < beepCount; i++) {
      tone(BUZZER_PIN, 1000); // 1kHz beep
      delay(200);
      noTone(BUZZER_PIN);
      delay(200);
    }
    // Reset samples for next hour
    sampleCount = 0;
  }
}

// Estimate number of LEDs illuminated based on thresholds
int estimateLEDs(int value) {
  if (value < ledThreshold1) return 0;
  if (value < ledThreshold2) return 1;
  if (value < ledThreshold3) return 2;
  return 3;
}
