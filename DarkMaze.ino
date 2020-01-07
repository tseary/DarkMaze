/*
	Name:       DarkMaze.ino
	Created:	12/15/2019 9:24:32 PM
	Author:     SCROLLSAW\Thomas
*/

#include <Arduino.h>

#include <Adafruit_DotStar.h>
#include <Adafruit_DRV2605.h>
#include <SPI.h>
#include <TrackBall.h>
#include <Wire.h>

#include "Maze.h"
#include "MazeMaker.h"

// Haptic setup
Adafruit_DRV2605* haptic = NULL;

// [0] - middle of room
// [1] - near 1 wall
// [2] - near 2 walls
// etc.
uint8_t EFFECT_FOOTSTEPS[] = {74, 8, 7};		// best yet
//uint8_t EFFECT_FOOTSTEPS[] = {72, 74, 33};	// echoey
const uint8_t NUM_FOOTSTEPS = sizeof(EFFECT_FOOTSTEPS);

uint8_t
EFFECT_BUMP = 12;	// DEBUG make const
const uint8_t
EFFECT_START = 82,
EFFECT_CLOCK = 25,
EFFECT_LOCKED_DOOR = 52,
EFFECT_DOOR_OPEN = 56;

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
// Player position in the maze
uint8_t xPlayer = 0, yPlayer = 0;

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

	// Create a new maze
	MazeMaker::createMaze(maze, xPlayer, yPlayer);
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

	bool bump = false;
	const uint8_t NONE = 255;
	uint8_t footstep = NONE;
	if (move) {
		trackball->resetOrigin();

		uint8_t xPlayerDest = xPlayer;
		uint8_t yPlayerDest = yPlayer;

		if (move == N) {
			yPlayerDest++;
		} else if (move == S) {
			yPlayerDest--;
		} else if (move == E) {
			xPlayerDest--;
		} else if (move == W) {
			xPlayerDest++;
		}

		if (!maze->isWall(xPlayerDest, yPlayerDest)) {
			xPlayer = xPlayerDest;
			yPlayer = yPlayerDest;

			// Count the number of 4-neighbour walls.
			// This number will be between 0 and 3

			footstep = 0;
			if (maze->isWall(xPlayer + 1, yPlayer)) footstep++;
			if (maze->isWall(xPlayer - 1, yPlayer)) footstep++;
			if (maze->isWall(xPlayer, yPlayer + 1)) footstep++;
			if (maze->isWall(xPlayer, yPlayer - 1)) footstep++;

			footstep = min(footstep, NUM_FOOTSTEPS - 1);
		} else {
			bump = true;
		}

		// DEBUG
		printMaze();
	}

	//
	// Queue haptic effects
	//
	uint8_t slot = 0;
	
	// Footsteps
	if (footstep != NONE) {
		queueHapticEffect(EFFECT_FOOTSTEPS[footstep], slot++);
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

void initializeTrackBall() {
	trackball = new TrackBall(TrackBall::I2C_ADDRESS, TRACKBALL_INT_PIN);
	trackball->setRGBW(0, 0, 0, 32);
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
	const uint8_t DATAPIN = 7, CLOCKPIN = 8;
	Adafruit_DotStar *dot = new Adafruit_DotStar(1, DATAPIN, CLOCKPIN);
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

void printMaze() {
	for (int y = maze->getHeight() - 1; y >= 0; y--) {

		Serial.print("y=\t");
		Serial.print(y);
		Serial.print("\t");

		for (int x = maze->getWidth() - 1; x >= 0; x--) {
			if (maze->isWall(x, y)) {
				// Wall
				Serial.print("[]");
			} else if (x == xPlayer && y == yPlayer) {
				// Player
				Serial.print("P1");
			} else {
				// Space
				Serial.print(".'");
			}
		}

		Serial.println();
	}
	Serial.println();
}
