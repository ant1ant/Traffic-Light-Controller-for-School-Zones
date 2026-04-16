// ==========================================
// SMART SCHOOL ZONE TRAFFIC CONTROL SYSTEM
// FINAL COMPLETE CODE
// ESP32 + 2 IR Sensors + 1 Ultrasonic Sensor
// Optional Traffic Light Output Pins Included
// ==========================================

// ---------------- PIN DEFINITIONS ----------------
#define IR1_PIN     14   // Pedestrian Sensor S1
#define IR2_PIN     27   // Pedestrian Sensor S2

#define TRIG_PIN    26   // Ultrasonic Trigger
#define ECHO_PIN    25   // Ultrasonic Echo

//traffic light output pins
#define RED_PIN     18
#define YELLOW_PIN  19
#define GREEN_PIN   21

// ---------------- SETTINGS ----------------
const float SAFE_DISTANCE = 20.0;   // cm (good for demo; increase if needed)
const int YELLOW_TIME = 2000;       // 2 seconds
const int CROSS_TIME = 10000;       // 10 seconds
const int EXTEND_TIME = 5000;       // 5 seconds
const int DETECTION_CONFIRM = 300;  // ms debounce

bool crossingActive = false;

// ==========================================
// FUNCTION: Read distance from ultrasonic
// ==========================================
float getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout

  if (duration == 0) {
    return 999.0; // No object detected
  }

  float distance = duration * 0.0343 / 2.0;
  return distance;
}

// ==========================================
// FUNCTION: Check if pedestrian is waiting
// IR modules usually output LOW when detecting
// ==========================================
bool pedestrianDetected() {
  int s1 = digitalRead(IR1_PIN);
  int s2 = digitalRead(IR2_PIN);

  return (s1 == LOW || s2 == LOW);
}

// ==========================================
// FUNCTION: Set traffic light states
// (works if LEDs/module are later connected)
// ==========================================
void setTrafficLight(bool redState, bool yellowState, bool greenState) {
  digitalWrite(RED_PIN, redState);
  digitalWrite(YELLOW_PIN, yellowState);
  digitalWrite(GREEN_PIN, greenState);
}

// ==========================================
// FUNCTION: Change traffic light to RED
// ==========================================
void changeToRed() {
  Serial.println("TRAFFIC LIGHT: GREEN -> YELLOW -> RED");

  // Green ON
  setTrafficLight(false, false, true);
  delay(500);

  // Yellow ON
  setTrafficLight(false, true, false);
  delay(YELLOW_TIME);

  // Red ON
  setTrafficLight(true, false, false);

  Serial.println("RED light ON - Pedestrians can cross");
}

// ==========================================
// FUNCTION: Change traffic light to GREEN
// ==========================================
void changeToGreen() {
  Serial.println("TRAFFIC LIGHT: RED -> YELLOW -> GREEN");

  // Yellow briefly before green
  setTrafficLight(false, true, false);
  delay(1000);

  // Green ON
  setTrafficLight(false, false, true);

  Serial.println("GREEN light ON - Vehicles can move");
}

// ==========================================
// SETUP
// ==========================================
void setup() {
  Serial.begin(115200);

  // Sensor pins
  pinMode(IR1_PIN, INPUT);
  pinMode(IR2_PIN, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Optional traffic light output pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);

  // Default traffic state: GREEN
  setTrafficLight(false, false, true);

  Serial.println("======================================");
  Serial.println("SMART SCHOOL ZONE TRAFFIC SYSTEM");
  Serial.println("System Started Successfully");
  Serial.println("======================================");
}

// ==========================================
// MAIN LOOP
// ==========================================
void loop() {

  // Normal traffic flow unless crossing is active
  if (!crossingActive) {

    float distance = getDistanceCM();

    Serial.print("S1: ");
    Serial.print(digitalRead(IR1_PIN));
    Serial.print(" | S2: ");
    Serial.print(digitalRead(IR2_PIN));
    Serial.print(" | Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // Step 1: Check if pedestrian is waiting
    if (pedestrianDetected()) {
      Serial.println("Pedestrian detected. Confirming...");
      delay(DETECTION_CONFIRM);

      // Confirm again to avoid false trigger
      if (pedestrianDetected()) {

        distance = getDistanceCM();

        Serial.print("Vehicle Distance: ");
        Serial.print(distance);
        Serial.println(" cm");

        // Step 2: If road is safe, allow crossing
        if (distance > SAFE_DISTANCE) {
          crossingActive = true;

          Serial.println("SAFE CONDITION DETECTED");
          Serial.println("Stopping traffic for pedestrian crossing...");

          changeToRed();

          Serial.println("Crossing time started (10 seconds)...");
          delay(CROSS_TIME);

          // Step 3: Recheck if pedestrians are still present
          if (pedestrianDetected()) {
            Serial.println("Pedestrian still detected.");
            Serial.println("Extending RED signal for 5 more seconds...");
            delay(EXTEND_TIME);
          }

          // Step 4: Return to normal traffic
          changeToGreen();
          crossingActive = false;
        }
        else {
          Serial.println("Vehicle too close. Waiting before stopping traffic...");
        }
      }
    }
    else {
      // No pedestrian request
      setTrafficLight(false, false, true);
      Serial.println("No pedestrian waiting. GREEN remains ON.");
    }
  }

  delay(500);
}