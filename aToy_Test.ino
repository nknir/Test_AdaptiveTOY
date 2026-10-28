// Arduino Uno with DRV8833 motor driver

const int FSR_PIN = A0;

const int MOTOR_IN1 = 5;     // Driver AIN1
const int MOTOR_IN2 = 6;     // Driver AIN2
const int MOTOR_SLEEP = 7;   // Driver SLP
const int FAULT_PIN = 8;     // Driver FLT

const int PRESS_THRESHOLD = 200;
const int RELEASE_THRESHOLD = 100;

int motorSpeed = 175;
const int SPEED_STEP =30;

int toysFed = 0;
bool readyForToy = false;
bool faultStopped = false;

unsigned long lastPrint = 0;


void stopMotor() {
  analogWrite(MOTOR_IN1, 0);
  digitalWrite(MOTOR_SLEEP, LOW);
  motorSpeed = 0;
}


void setup() {
  Serial.begin(115200);

  pinMode(FSR_PIN, INPUT);
  pinMode(FAULT_PIN, INPUT_PULLUP);

  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_SLEEP, OUTPUT);

  analogWrite(MOTOR_IN1, 0);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_SLEEP, LOW);

  Serial.println("FSR AND MOTOR DIAGNOSTICS");
  Serial.println("A0 = FSR");
  Serial.println("D5 = AIN1, D6 = AIN2");
  Serial.println("D7 = SLP, D8 = FLT");
  Serial.println("Press and release the FSR to test.");
  Serial.println("PWM is a command, not measured motor speed.");

  digitalWrite(MOTOR_SLEEP, HIGH);
  delay(5);

  analogWrite(MOTOR_IN1, motorSpeed);
  Serial.println("Starting motor: PWM command = 100");
}


void loop() {
  int pressure = analogRead(FSR_PIN);
  bool faultSignal = digitalRead(FAULT_PIN) == LOW;

  // Stop if the driver reports a fault during operation.
  if (faultSignal && !faultStopped && toysFed < 4) {
    stopMotor();
    faultStopped = true;

    Serial.println(
      "FAULT SIGNAL: motor disabled. Check driver, wiring,"
    );
    Serial.println(
      "motor load and supply. Press Reset after fixing."
    );
  }

  // Run the feeding logic while the game is active.
  if (!faultStopped && toysFed < 4) {
    if (pressure <= RELEASE_THRESHOLD) {
      readyForToy = true;
    }

    if (pressure >= PRESS_THRESHOLD && readyForToy) {
      readyForToy = false;
      toysFed++;

      Serial.print("NEW FSR PRESS: toy count = ");
      Serial.println(toysFed);

      if (toysFed >= 10) {
        stopMotor();
        Serial.println("GAME COMPLETE: motor command OFF.");
        Serial.println("Press Arduino Reset to restart.");
      } else {
        motorSpeed += SPEED_STEP;
        analogWrite(MOTOR_IN1, motorSpeed);

        Serial.print("Motor PWM command increased to ");
        Serial.println(motorSpeed);
      }
    }
  }

  // Keep showing diagnostics even after the motor stops.
  if (millis() - lastPrint >= 500) {
    lastPrint = millis();

    Serial.print("FSR=");
    Serial.print(pressure);

    Serial.print(" | Toys=");
    Serial.print(toysFed);
    Serial.print("/4");

    Serial.print(" | PWM command=");
    Serial.print(motorSpeed);

    Serial.print(" | FLT pin=");
    Serial.print(faultSignal ? "LOW" : "HIGH");

    if (faultStopped) {
      Serial.print(" | STOPPED BY FAULT");
    } else if (toysFed >= 4) {
      Serial.print(" | GAME COMPLETE");
    }

    Serial.println();
  }

  delay(10);
}