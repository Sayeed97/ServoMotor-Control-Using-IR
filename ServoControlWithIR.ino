#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Servo.h> // Servo library  

// All known motor command states
#define UNKNOWN_MOTOR_CMD   -1
#define MIN_MOTOR_POSITION  0
#define MAX_MOTOR_POSITION  180
#define MOTOR_ROTATE_RIGHT  90
#define MOTOR_ROTATE_LEFT   -90

// An IR detector/demodulator is connected to GPIO pin 5 (D1 on a NodeMCU board).
// Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
#define IR_RECEIVER_PIN 5

IRrecv irrecv(IR_RECEIVER_PIN);

decode_results results;

Servo servo;

// Servo Motor position
unsigned int currentServoPosition = 0;

// A simple no operation function that does nothing
void noop() {}

void changeServoMotorPositionToMinimumPosition(void) {
  currentServoPosition = MIN_MOTOR_POSITION;
}

// Increment the motor position by 90
void servoMotorPositionIncrement(void) {
  // Do nothing if the servo is already at max position
  if (currentServoPosition == MAX_MOTOR_POSITION)
    return;
  currentServoPosition = (currentServoPosition + 90 <= MAX_MOTOR_POSITION) ? currentServoPosition + 90 : MAX_MOTOR_POSITION;
}

// Decrement the motor position by 90
void servoMotorPositionDecrement(void) {
  // Do nothing if the servo is already at min position
  if (currentServoPosition == MIN_MOTOR_POSITION) 
    return;
  currentServoPosition = (currentServoPosition - 90 > MIN_MOTOR_POSITION) ? currentServoPosition - 90 : MIN_MOTOR_POSITION;
}

// Force servo position to zero; if servo is already at position zero then do nothing (noop)
void resetServoMotorPositionToMinimumPosition(void) {
  (currentServoPosition != MIN_MOTOR_POSITION) ? changeServoMotorPositionToMinimumPosition() : noop();
}

// Peforms servo motor action based on the servo command received
void servoMotorCmd(int cmd) {
  // Servo turns right or left direction; Also checks if servo is forced back to zero position 
  // Servo performs no operation for unknown commands
  (cmd != UNKNOWN_MOTOR_CMD) ? ((cmd != MOTOR_ROTATE_LEFT) ? ((cmd != MIN_MOTOR_POSITION) ? servoMotorPositionIncrement() : resetServoMotorPositionToMinimumPosition()) : servoMotorPositionDecrement()) : noop();
  // Drive the servo to the given position
  servo.write(currentServoPosition);
}

// Drives the servo to a particular position based on the IR received command
void moveServoMotorByCommand(void) {
  // Perform servo motor commands based on the IR message value
  if (irrecv.decode(&results)) {
    // results.value is uint64_t; print() & println() can't handle printing long longs (uint64_t)
    // Based on the received value the program performs a specific servo motor operation
    switch(results.value) {
      case 0xFFA857:
        servoMotorCmd(MOTOR_ROTATE_RIGHT);
        break;
      case 0xFFC23D:
        servoMotorCmd(MOTOR_ROTATE_LEFT);
        break;  
      case 0xFF6897:
        servoMotorCmd(MIN_MOTOR_POSITION);
        break;               
      default:
        servoMotorCmd(UNKNOWN_MOTOR_CMD);
        break;     
    }
    Serial.println("Current Servo Position: " + String(currentServoPosition)); // for debugging purposes
    irrecv.resume();  // Receive the next value
  }
  delay(100); // some wait time before the next servo command
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(50); } // Wait for the serial connection to be establised.
  servo.attach(14);  // servo attach D3 pin of arduino  
  irrecv.enableIRIn();  // Start the receiver
  servo.write(MIN_MOTOR_POSITION); // Force the servo to start at zero position when the micro-controller restarts
}

void loop() {
  moveServoMotorByCommand();
}