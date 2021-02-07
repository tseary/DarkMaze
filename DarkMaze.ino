/*
	Name:       DarkMaze.ino
	Created:	12/15/2019 9:24:32 PM
	Author:     SCROLLSAW\Thomas
*/

#include <Arduino.h>
#include <stdint.h>

#include <Adafruit_DotStar.h>
#include <Adafruit_DRV2605.h>
#include <SPI.h>
#include <TrackBall.h>
#include <Wire.h>

#include "Items.h"
#include "Maze.h"
#include "MazeMaker.h"

// Haptic setup
Adafruit_DRV2605* haptic = NULL;

const uint8_t NONE = 255;

// [0] - middle of room
// [1] - near 1 wall
// [2] - near 2 walls
// etc.
uint8_t EFFECT_FOOTSTEPS[] = {74, 8, 7};		// best yet
//uint8_t EFFECT_FOOTSTEPS[] = {72, 74, 33};	// echoey
const uint8_t NUM_FOOTSTEPS = sizeof(EFFECT_FOOTSTEPS);

// [0] = closest
uint8_t EFFECT_HOMING[] = {64, 65, 67, 68, 69};
const uint8_t NUM_HOMINGS = sizeof(EFFECT_HOMING);

uint8_t homingDistance;
uint32_t homingStartTime;

uint8_t
EFFECT_BUMP = 12;	// DEBUG make const
const uint8_t
EFFECT_START = 82,
EFFECT_CLOCK = 25,
EFFECT_KEY_NEARBY = 0,
EFFECT_FIND_KEY1 = 89,	// Both find key effects are played in sequence (ramp up and double click)
EFFECT_FIND_KEY2 = 34,
EFFECT_LOCKED_DOOR = 52,
EFFECT_UNLOCK_DOOR = 56;

// DEBUG
uint8_t newEffect = 0;

// Trackball setup
const uint8_t TRACKBALL_INT_PIN = 4;
TrackBall* trackball = NULL;

// Trackball input
const uint8_t TICKS_PER_STEP = 5;
bool switchWasPressed = false;
bool switchClick = false;

// Maze data
Maze* maze = NULL;

// Game state
bool haveMidKey, haveExitKey;

// Cardinal directions
const uint8_t N = 1, S = 2, E = 3, W = 4;

void setup() {
	Serial.begin(115200);
	Serial.println("Welcome to Dark Maze");

	Wire.begin();
	initializeTrackBall();
	initializeHaptic();
	initializeDotstar();

	// Startup effect
	queueHapticEffect(EFFECT_START, 0);
	playHapticEffects(1);
	delay(500);

	// Initialize the RNG by clicking the trackball
	Serial.println("Click the trackball to begin.");
	do {
		trackball->read();
	} while (!trackball->getSwitchState());
	uint32_t seed = micros() << 8;
	do {
		trackball->read();
	} while (trackball->getSwitchState());
	seed ^= micros();
	//seed = 496320634;	// DEBUG
	randomSeed(seed);
	Serial.print("Random seed: ");
	Serial.println(seed);

	// Some special cases:
	// Random seed: 496320634	one big room with no walls
	// Random seed: 534960517	BROKEN: Player starts in a one-room region

	// Make a new maze
	newGame();
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
			move = x > 0 ? W : E;
		} else if (abs(y) >= TICKS_PER_STEP) {
			move = y > 0 ? S : N;
		}
	}

	//
	// Move the player
	//

	bool requestEffectBump = false,
		requestEffectKnock = false,
		requestEffectUnlock = false,
		requestEffectJingle = false,
		requestEffectTwinkle = false;
	uint8_t footstep = NONE;
	if (move) {
		trackball->resetOrigin();
		homingStartTime += 200;	// Delay the homing effect

		// Get the player's current location
		uint8_t xPlayerDest, yPlayerDest;
		maze->items->getPlayer(xPlayerDest, yPlayerDest);

		// Get attempted move destination
		if (move == N) {
			yPlayerDest++;
		} else if (move == S) {
			yPlayerDest--;
		} else if (move == E) {
			xPlayerDest--;
		} else if (move == W) {
			xPlayerDest++;
		}

		// Attempt to move
		if (maze->isWall(xPlayerDest, yPlayerDest)) {
			// Hit a wall
			requestEffectBump = true;
			move = 0;	// Move failed
		} else if (maze->items->isMidDoor(xPlayerDest, yPlayerDest)) {
			if (haveMidKey) {
				// Unlock the door
				requestEffectUnlock = true;
				maze->items->setPlayer(xPlayerDest, yPlayerDest);
				maze->items->clearMidDoor();
			} else {
				// Knock on the door
				requestEffectKnock = true;
				move = 0;	// Move failed
			}
		} else if (maze->items->isExitDoor(xPlayerDest, yPlayerDest)) {
			if (haveExitKey) {
				// Unlock the door
				requestEffectUnlock = true;
				maze->items->setPlayer(xPlayerDest, yPlayerDest);
			} else {
				// Knock on the door
				requestEffectKnock = true;
				move = 0;	// Move failed
			}
		} else {
			if (maze->items->isMidKey(xPlayerDest, yPlayerDest)) {
				// Pick up the middle door key
				haveMidKey = true;
				maze->items->clearMidKey();
				requestEffectJingle = true;
			} else if (maze->items->isExitKey(xPlayerDest, yPlayerDest)) {
				// Pick up the exit door key
				haveExitKey = true;
				maze->items->clearExitKey();
				requestEffectJingle = true;
			}

			// Open space
			maze->items->setPlayer(xPlayerDest, yPlayerDest);
		}

		// DEBUG
		maze->printMaze();
	}

	// Update stuff only if the attempted move succeeded
	if (move) {
		// Get the player's current location
		uint8_t xPlayerDest, yPlayerDest;
		maze->items->getPlayer(xPlayerDest, yPlayerDest);

		// Count the number of 4-neighbour walls.
		// This number will be between 0 and 3
		footstep = 0;
		if (maze->isWall(xPlayerDest + 1, yPlayerDest)) footstep++;
		if (maze->isWall(xPlayerDest - 1, yPlayerDest)) footstep++;
		if (maze->isWall(xPlayerDest, yPlayerDest + 1)) footstep++;
		if (maze->isWall(xPlayerDest, yPlayerDest - 1)) footstep++;

		footstep = min(footstep, NUM_FOOTSTEPS - 1);

		// Determine if a key is nearby and update the homing signal
		const int KEY_HOMING_DISTANCE_SQR = 49;
		int distanceSqrToKey = maze->items->distanceSqrToNearestKey(xPlayerDest, yPlayerDest);
		if (distanceSqrToKey <= KEY_HOMING_DISTANCE_SQR) {
			// Reset the homing timer when homing begins
			if (homingDistance == NONE) {
				homingStartTime = millis();
			}

			homingDistance = (uint8_t)sqrt(distanceSqrToKey);
			Serial.print("homing dist =\t");
			Serial.println(homingDistance);
		} else {
			// We are not near anything, clear the homing
			homingDistance = NONE;
		}
	}

	//
	// Queue haptic effects
	//
	uint8_t slot = 0;

	// Footsteps
	if (footstep != NONE) {
		queueHapticEffect(EFFECT_FOOTSTEPS[footstep], slot++);
	}

	// Homing
	const uint32_t HOMING_MILLIS = 1500;
	if (homingDistance != NONE && millis() >= (homingStartTime + homingPeriod())) {
		queueHapticEffect(homingEffect(), slot++);
		homingStartTime += HOMING_MILLIS;
	}

	// Hitting the wall
	if (requestEffectBump) {
		queueHapticEffect(EFFECT_BUMP, slot++);
	}

	// Locked door
	if (requestEffectKnock) {
		queueHapticEffect(EFFECT_LOCKED_DOOR, slot++);
	}

	// Unlocking a door
	if (requestEffectUnlock) {
		queueHapticEffect(EFFECT_UNLOCK_DOOR, slot++);
	}

	// Picking up a key
	if (requestEffectJingle) {
		queueHapticEffect(EFFECT_FIND_KEY1, slot++);
		queueHapticEffect(EFFECT_FIND_KEY2, slot++);
	}

	// User click
	if (switchClick) {
		queueHapticEffect(EFFECT_LOCKED_DOOR, slot++);
	}

	//
	// Play haptic effects
	//
	if (slot) {
		haptic->stop();	// Stop playback
		playHapticEffects(slot);
	}

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
			EFFECT_BUMP = newEffect;
			newEffect = 0;
			Serial.print("new footstep effect: ");
			Serial.println(EFFECT_BUMP);
		}
	}

	delay(50);
}

void newGame() {
	// Create a new maze
	MazeMaker* maker = MazeMaker::getInstance();
	uint8_t roomsX = random(3) + 3,	// 3-5
		roomsY = random(2) + 3,		// 3-4
		roomW = random(3) + 4,		// 4-6
		roomH = random(4) + 4;		// 4-7
	maker->setMazeDimensions(roomsX, roomsY, roomW, roomH);
	maker->createMaze(maze);

	// Clear the keys
	haveMidKey = false;
	haveExitKey = false;

	// Clear homing
	homingDistance = NONE;
	homingStartTime = millis();
}

void initializeTrackBall() {
	trackball = new TrackBall(TrackBall::I2C_ADDRESS, TRACKBALL_INT_PIN);
	//trackball->setRGBW(0, 0, 0, 32);
	trackball->setRGBW(16, 16, 0, 32);
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

void initializeDotstar() {
	// Turn off the internal light (it is hidden inside the enclosure)
	const uint8_t DATAPIN = 7, CLOCKPIN = 8;
	Adafruit_DotStar* dot = new Adafruit_DotStar(1, DATAPIN, CLOCKPIN);
	dot->begin();
	dot->show();
	delete dot;
	SPI.end();
}

void queueHapticEffect(uint8_t effect, uint8_t slot) {
	haptic->setWaveform(slot, effect);
	Serial.print("queue haptic effect: ");
	Serial.println(effect);
}

void playHapticEffects(uint8_t queueLength) {
	haptic->setWaveform(queueLength, 0);   // end waveform
	haptic->go();
}

// Homing helpers

uint8_t homingEffect() {
	return EFFECT_HOMING[min(homingDistance, NUM_HOMINGS) - 1];
}

uint32_t homingPeriod() {
	return (homingDistance + 1) * 333;
}
