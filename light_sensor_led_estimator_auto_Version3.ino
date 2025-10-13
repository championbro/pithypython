/*
  light_sensor_led_estimator_auto.ino

  Purpose:
  - Observe a light sensor (A0) and automatically determine thresholds (no manual calibration)
    by tracking the day's minimum and maximum readings.
  - Estimate how many of three external LEDs are illuminated (0..3) by dividing the day's
    observed range into 4 bands.
  - Sample every second, keep hourly average for reporting, and beep 0..3 times every hour
    according to the hourly average estimate.
  - Reset the day's min/max once every 24 hours so thresholds adapt day-to-day.
  - Print status to Serial for monitoring.

  Notes:
  - This sketch assumes the other project will produce conditions such that at least once
    per day you will observe the scene when all 3 LEDs are illuminated (so the daily max
    will reflect that). During sleep hours you should observe near the daily min (0 LEDs).
  - No calibration step required; thresholds are computed automatically from daily min/max.
  - Pins:
      LIGHT_SENSOR_PIN -> A0 (analog light sensor)
      BUZZER_PIN       -> D5 (piezo buzzer; use tone() / noTone())
  - Adjust SAMPLE_INTERVAL_MS or HOUR_INTERVAL_MS if you want different cadences.

  Author: adapted for your requirements
*/

#define LIGHT_SENSOR_PIN A0
#define BUZZER_PIN 5

// timing
const unsigned long SAMPLE_INTERVAL_MS = 1000UL;     // sample every 1 second
const unsigned long HOUR_INTERVAL_MS   = 3600000UL;  // 1 hour
const unsigned long DAY_INTERVAL_MS    = 86400000UL; // 24 hours

// running state
unsigned long lastSampleTime = 0;
unsigned long lastHourTime = 0;
unsigned long dayStartTime = 0;

// hourly accumulation
unsigned long hourSum = 0;
unsigned int hourCount = 0;

// daily min/max (learned automatically)
int dayMin = 1023;
int dayMax = 0;

// debug prints: set to true to get Serial output
const bool SERIAL_DEBUG = true;

// ------------------------------------------------------------------
// Setup / loop
// ------------------------------------------------------------------
void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  if (SERIAL_DEBUG) {
    Serial.begin(115200);
    delay(10);
    Serial.println();
    Serial.println("Auto light-based LED estimator starting...");
  }
  lastSampleTime = millis();
  lastHourTime = millis();
  dayStartTime = millis();
  // Initialize dayMin/dayMax to extremes so first readings set them.
  dayMin = 1023;
  dayMax = 0;
}

void loop() {
  unsigned long now = millis();

  // -- Sampling
  if (now - lastSampleTime >= SAMPLE_INTERVAL_MS) {
    lastSampleTime += SAMPLE_INTERVAL_MS; // keep it steady
    int lightValue = analogRead(LIGHT_SENSOR_PIN);

    // update hourly accumulation
    hourSum += (unsigned long)lightValue;
    hourCount++;

    // update day's min/max
    if (lightValue < dayMin) dayMin = lightValue;
    if (lightValue > dayMax) dayMax = lightValue;

    // compute current estimate and optionally print
    int estimate = estimateLEDs(lightValue, dayMin, dayMax);
    if (SERIAL_DEBUG) {
      Serial.print("sample: ");
      Serial.print(lightValue);
      Serial.print(" | est=");
      Serial.print(estimate);
      Serial.print(" | dayMin=");
      Serial.print(dayMin);
      Serial.print(" dayMax=");
      Serial.println(dayMax);
    }
  }

  // -- Hourly reporting & beep
  if (now - lastHourTime >= HOUR_INTERVAL_MS) {
    lastHourTime += HOUR_INTERVAL_MS;
    int hourlyEstimate = 0;
    if (hourCount > 0) {
      int avg = (int)(hourSum / hourCount);
      hourlyEstimate = estimateLEDs(avg, dayMin, dayMax);
      if (SERIAL_DEBUG) {
        Serial.println();
        Serial.print("HOURLY: samples=");
        Serial.print(hourCount);
        Serial.print(", avg=");
        Serial.print(avg);
        Serial.print(", hourly estimate=");
        Serial.println(hourlyEstimate);
      }
    } else {
      if (SERIAL_DEBUG) Serial.println("HOURLY: no samples this hour.");
    }

    // beep 0..3 times according to hourlyEstimate
    beepCount(hourlyEstimate);

    // clear hourly accumulators
    hourSum = 0;
    hourCount = 0;
  }

  // -- Daily reset of learned min/max (every 24 hours)
  if (now - dayStartTime >= DAY_INTERVAL_MS) {
    // Before resetting, print final daily stats
    if (SERIAL_DEBUG) {
      Serial.println();
      Serial.println("=== DAILY RESET ===");
      Serial.print("Day observed min=");
      Serial.print(dayMin);
      Serial.print(", max=");
      Serial.println(dayMax);
      Serial.println("Recomputing thresholds for next day (automatic).");
    }
    // restart daily learning
    dayMin = 1023;
    dayMax = 0;
    dayStartTime += DAY_INTERVAL_MS;
  }

  // keep loop non-blocking aside from tone delays in beepCount
}

// ------------------------------------------------------------------
// Utility functions
// ------------------------------------------------------------------

// Estimate number of LEDs (0..3) by dividing observed day range into 4 bands.
// If dayMax <= dayMin (not enough data yet), fall back to simple fixed bands.
int estimateLEDs(int value, int minv, int maxv) {
  // If we haven't learned a range yet, use fallback equal-sized bands across 0..1023
  if (maxv <= minv + 5) { // small tolerance to avoid division by zero
    // no learning yet: map ADC full-scale into 4 bins
    int band = (value * 4) / 1024; // 0..3
    if (band < 0) band = 0;
    if (band > 3) band = 3;
    return band;
  }

  // compute band thresholds as quartiles of the day's range:
  // [min, t1) -> 0, [t1, t2) ->1, [t2, t3)->2, [t3, max] ->3
  long range = (long)maxv - (long)minv;
  int t1 = minv + (int)(range / 4L);
  int t2 = minv + (int)(range / 2L);
  int t3 = minv + (int)(3L * range / 4L);

  if (value < t1) return 0;
  if (value < t2) return 1;
  if (value < t3) return 2;
  return 3;
}

// Emit up to 3 beeps: 200ms on, 200ms off between beeps.
void beepCount(int count) {
  if (count <= 0) return;
  if (count > 3) count = 3;
  for (int i = 0; i < count; ++i) {
    tone(BUZZER_PIN, 1000); // 1kHz
    delay(200);
    noTone(BUZZER_PIN);
    if (i < count - 1) delay(200);
  }
}