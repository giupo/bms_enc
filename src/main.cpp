#include <Arduino.h>

// -----
// AcceleratedRotator.ino - Example for the RotaryEncoder library.
// This class is implemented for use with the Arduino environment.
// Copyright (c) by Matthias Hertel, http://www.mathertel.de
// This work is licensed under a BSD style license. See http://www.mathertel.de/License.aspx
// More information on: http://www.mathertel.de/Arduino
// -----
// 18.01.2014 created by Matthias Hertel
// 13.11.2019 converted to AcceleratedRotator by Damian Philipp
// -----

// This example checks the state of the rotary encoder in the loop() function.
// It then computes an acceleration value and prints the current position when changed.

// Hardware setup:
// Attach a rotary encoder with output pins to A2 and A3.
// The common contact should be attached to ground.

#include <RotaryEncoder.h>

// Setup a RoraryEncoder for pins A2 and A3:
RotaryEncoder encoder(D2, D1);

// Define some constants.
// at 500ms, there should be no acceleration.
constexpr const unsigned long kAccelerationLongCutoffMillis = 100;
// at 4ms, we want to have maximum acceleration
constexpr const unsigned long kAccelerationShortCutffMillis = 4;
// linear acceleration: incline
constexpr static const float m = .04f;
// linear acceleration: y offset
constexpr static const float c = 10.03;

/* To derive these constants, compute as follows:
 * * On an x-y-plane, let x be the time between rotations.
 * * On an x-y-plane, let y be the accelerated number of rotations.
 * * Select a long cuttoff. If two encoder rotations happen longer apart than the cutoff,
 *   they are not accelerated anymore. Without the long cutoff, not moving the encoder for a while
 *   will make the value jump by extreme amounts (possibly in the wrong direction) on the next rotation.
 * * Select a short cutoff. This limits the maximum acceleration. While an infinite acceleration is
 *   not really a problem, it is unrealistic to achieve (how quickly can you tick the encoder?) and
 *   not having it causes headaches when computing the actual acceleration ;)
 *   Pick two points defining your acceleration. At x2=(long cutoff), you want y2=1 (No acceleration happened).
 *   At x1=(short cutoff) you want y1=(maximum accelerated number of ticks).
 *   Then, compute m and c using high school math (or google for "straight line through two points").
 * 
 *   The numbers given in this example are tailored for the following conditions:
 *   * An encoder with 24 increments per 360 degrees
 *   * A useful range of 0..1000 rotations
 *   * Making a 180 degree rotation (12 increments) on the encoder within 50ms will hit
 *     the opposite end of the range.
 * 
 *   Please do not rely on these exact numbers. Recompute them for your usecase and verify
 *   them with a physical test. A 4ms timing is fairly tight for many controllers. Depending on your
 *   controller and application, you might not be able to sample the encoder quickly enough to achieve this.
 */


void setup() {
  Serial.begin(9600);
  Serial.println("AcceleratedRotator example for the RotaryEncoder library.");
} // setup()



bool is_same_direction(RotaryEncoder::Direction currentDirection, RotaryEncoder::Direction lastMovementDirection) {
  return currentDirection == lastMovementDirection && 
        currentDirection != RotaryEncoder::Direction::NOROTATION &&
        lastMovementDirection != RotaryEncoder::Direction::NOROTATION;
}

float compute_acceleration() {
  u_long deltat = encoder.getMillisBetweenRotations();
  if (deltat < kAccelerationLongCutoffMillis) {
    if (deltat < kAccelerationShortCutffMillis) {
      // limit to maximum acceleration
      deltat = kAccelerationShortCutffMillis;
    }
    float fTicksActual = m * deltat + c;
    return max(.0f, fTicksActual);
  }

  return 0.0;
}
      
// Read the current position of the encoder and print out when changed.
void loop() {
  static int pos = 0;
  static RotaryEncoder::Direction lastMovementDirection = RotaryEncoder::Direction::NOROTATION;
   encoder.tick();

  int newPos = encoder.getPosition();
  if (pos != newPos) {
    // compute linear acceleration
    RotaryEncoder::Direction currentDirection = encoder.getDirection();
    if (is_same_direction(currentDirection, lastMovementDirection)) {
      
      int deltaTicks = (int) compute_acceleration();
#ifdef DEBUG
      Serial.print("Accelerated value:");
      Serial.print(deltaTicks);
      Serial.print(" - ");
#endif 
      if (currentDirection == RotaryEncoder::Direction::COUNTERCLOCKWISE) {
        deltaTicks = -(deltaTicks);
      }

      newPos = (newPos + deltaTicks) % 360;
      encoder.setPosition(newPos);
    }

#ifdef DEBUG  
    Serial.print("Pos: ");
    Serial.print(newPos);
    Serial.println();
#endif

    pos = newPos;
    lastMovementDirection = currentDirection;
  } // if
} // loop

// The End