// Pin definitions
#define LED1 2
#define LED2 3
#define LED3 4
#define SOUND_PIN A0

// Sampling settings
#define SAMPLE_INTERVAL 10000 // 10 seconds in ms
#define SAMPLING_DURATION 86400000UL // 24 hours in ms
#define MAX_SAMPLES 8640 // 24h * 60min * 6 samples/min (every 10s)

unsigned int samples[MAX_SAMPLES];
unsigned int sampleCount = 0;
unsigned long startTime;
bool samplingDone = false;

int minLevel = 0, medianLevel = 0, maxLevel = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  startTime = millis();
}

void loop() {
  if (!samplingDone) {
    // Sampling phase
    static unsigned long lastSampleTime = 0;
    if (millis() - lastSampleTime >= SAMPLE_INTERVAL && 
        millis() - startTime < SAMPLING_DURATION && 
        sampleCount < MAX_SAMPLES) {
      lastSampleTime = millis();
      int soundValue = analogRead(SOUND_PIN);
      samples[sampleCount++] = soundValue;
      Serial.print("Sample "); Serial.print(sampleCount); Serial.print(": "); Serial.println(soundValue);
    }
    // After sampling period, calculate min, median, max
    if ((millis() - startTime >= SAMPLING_DURATION) || (sampleCount >= MAX_SAMPLES)) {
      calculateThresholds();
      samplingDone = true;
      Serial.print("Sampling done. Min: "); Serial.print(minLevel);
      Serial.print(", Median: "); Serial.print(medianLevel);
      Serial.print(", Max: "); Serial.println(maxLevel);
      delay(2000);
    }
  } else {
    // LED control phase
    int soundValue = analogRead(SOUND_PIN);
    if (soundValue <= medianLevel) {
      // Low sound: blink LED1
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, LOW);
    } else if (soundValue <= maxLevel) {
      // Medium sound: blink LED1 & LED2
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, HIGH);
      digitalWrite(LED3, LOW);
    } else {
      // High sound: blink all three LEDs
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, HIGH);
      digitalWrite(LED3, HIGH);
    }
    delay(500); // Blink interval
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    delay(500);
  }
}

// Helper to calculate min, median, max from samples
void calculateThresholds() {
  // Min and Max
  minLevel = samples[0];
  maxLevel = samples[0];
  for (unsigned int i = 1; i < sampleCount; i++) {
    if (samples[i] < minLevel) minLevel = samples[i];
    if (samples[i] > maxLevel) maxLevel = samples[i];
  }
  // Median
  sortSamples();
  if (sampleCount % 2 == 0) {
    medianLevel = (samples[sampleCount / 2 - 1] + samples[sampleCount / 2]) / 2;
  } else {
    medianLevel = samples[sampleCount / 2];
  }
}

// Simple insertion sort for small arrays
void sortSamples() {
  for (unsigned int i = 1; i < sampleCount; i++) {
    unsigned int temp = samples[i];
    int j = i - 1;
    while (j >= 0 && samples[j] > temp) {
      samples[j + 1] = samples[j];
      j--;
    }
    samples[j + 1] = temp;
  }
}
