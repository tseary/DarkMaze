/*
	Name:       DarkMaze.ino
	Created:	12/15/2019 9:24:32 PM
	Author:     SCROLLSAW\Thomas
*/

#include <Wire.h>
#include <Adafruit_DRV2605.h>
#include <TrackBall.h>

// Haptic setup
Adafruit_DRV2605* haptic = NULL;

uint8_t EFFECT_FOOTSTEP = 5;
uint8_t EFFECT_BUMP = 12;
uint8_t EFFECT_CLOCK = 25;

uint8_t newEffect = 0;

// Trackball setup
const uint8_t TRACKBALL_INT_PIN = 4;
TrackBall* trackball = NULL;

// Trackball input
const uint8_t TICKS_PER_STEP = 6;

bool switchWasPressed = false;
bool switchClick = false;

// Maze grid
// Mze size
const uint8_t MAZE_WIDTH = 4,
MAZE_HEIGHT = 3;
// Player position in the maze
uint8_t xMaze = 0, yMaze = 0;
uint8_t xTrackballOrigin = 0, yTrackballOrigin = 0;

const uint8_t N = 1, S = 2, E = 3, W = 4;

void setup() {
	Serial.begin(115200);
	Serial.println("Welcome to Dark Maze");

	Wire.begin();
	initializeTrackBall();
	initializeHaptic();

	// Startup effect
	queueHapticEffect(5, 0);
	queueHapticEffect(80, 1);
	queueHapticEffect(25, 2);
	playHapticEffects(3);
	delay(500);
}

void loop() {

	// Check for trackball movement
	uint8_t move = 0;
	if (trackball->isInterrupt()) {
		switchWasPressed = trackball->getSwitchState();

		trackball->read();

		switchClick = trackball->getSwitchState() && !switchWasPressed;

		int x = trackball->getX();
		int y = trackball->getY();

		if (abs(x) >= TICKS_PER_STEP) {
			move = x > 0 ? E : W;
		} else if (abs(y) >= TICKS_PER_STEP) {
			move = y > 0 ? N : S;
		}
	}

	//
	// Move the player
	//

	bool bump = false;
	bool footstep = false;
	if (move) {
		trackball->resetOrigin();
		if (move == N) {
			if (yMaze < MAZE_HEIGHT - 1) {
				yMaze++;
				footstep = true;
			} else {
				bump = true;
			}
		} else if (move == S) {
			if (yMaze > 0) {
				yMaze--;
				footstep = true;
			} else {
				bump = true;
			}
		} else if (move == E) {
			if (xMaze < MAZE_WIDTH - 1) {
				xMaze++;
				footstep = true;
			} else {
				bump = true;
			}
		} else if (move == W) {
			if (xMaze > 0) {
				xMaze--;
				footstep = true;
			} else {
				bump = true;
			}
		}
	}

	//
	// Queue haptic effects
	//
	bool requestFootstep = false,
		requestClick = false;
	uint8_t slot = 0;

	// Footsteps
	if (footstep) {
		queueHapticEffect(EFFECT_FOOTSTEP, slot++);
	}

	// Hitting the wall
	if (bump) {
		queueHapticEffect(EFFECT_BUMP, slot++);
	}

	// User click
	if (switchClick) {
		queueHapticEffect(EFFECT_BUMP, slot++);
	}

	//
	// Play haptic effects
	//
	playHapticEffects(slot);

	//
	// DEBUG get new effect
	//
	while (Serial.available()) {
		char c = Serial.read();
		if (isDigit(c)) {
			// Append a digit to the new effect number
			newEffect = newEffect * 10 + c - '0';
		} else if (c == '\n') {
			// Accept the new effect
			EFFECT_FOOTSTEP = newEffect;
			newEffect = 0;
			Serial.print("new footstep effect: ");
			Serial.println(EFFECT_FOOTSTEP);
		}
	}

	delay(50);
}

void initializeTrackBall() {
	trackball = new TrackBall(TrackBall::I2C_ADDRESS, TRACKBALL_INT_PIN);

}

void initializeHaptic() {
	haptic = new Adafruit_DRV2605();

	haptic->begin();
	haptic->selectLibrary(1);

	// I2C trigger by sending 'go' command
	// default, internal trigger when sending GO command
	haptic->setMode(DRV2605_MODE_INTTRIG);

	haptic->useLRA();
}

void queueHapticEffect(uint8_t effect, uint8_t slot) {
	haptic->setWaveform(slot, effect);
}

void playHapticEffects(uint8_t queueLength) {
	haptic->setWaveform(queueLength, 0);   // end waveform
	haptic->go();
}
