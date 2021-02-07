#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "Items.h"
#include "Maze.h"

class MazeMaker {
public:
	static MazeMaker* getInstance() {
		if (instance == NULL) {
			instance = new MazeMaker();
			instance->setMazeDimensions();	// Sane defaults
		}
		return instance;
	}

	void setMazeDimensions(uint8_t roomsX = 4, uint8_t roomsY = 4, uint8_t roomWidth = 5, uint8_t roomHeight = 5);

	// Pass in a NULL pointer for maze. It will be instantiated and populated with a valid maze.
	void createMaze(Maze*& maze);

private:
	// Singleton
	MazeMaker() {}
	static MazeMaker* instance;

	// The number of rooms in the maze
	uint8_t ROOMS_X, ROOMS_Y;
	uint8_t NUM_ROOMS;
	// The size of a room in paces
	uint8_t ROOM_W, ROOM_H;
	
	// Constants for door index
	uint16_t NUM_DOORS_E,	// The number of east/east doors
		NUM_DOORS;			// The total number of interior doors

	uint8_t MAZE_W, MAZE_H;

	/*
	* Each byte in the rooms array:
	* 0bShhhEwww
	* S: 1 = no walls, or doorway to south, 0 = wall to south.
	* hhh: horizontal wall height
	* E: 1 = no walls, or doorway to east, 0 = wall to east.
	* www: vertical wall width
	*/
	static const uint8_t
		S_MASK = 0b10000000,
		H_MASK = 0b01110000,
		E_MASK = 0b00001000,
		W_MASK = 0b00000111;
	static const uint8_t
		H_SHIFT = 4;

	static const uint8_t MAZE_BORDER = 1;

	// Region-Level Helpers
	bool replaceRegion(uint8_t** mazeRegions, uint8_t* mazeRegionSizes,
		const uint8_t oldRegion, const uint8_t newRegion) const;
	uint8_t countRegions(const uint8_t* mazeRegionSizes) const;

	// Room-Level Helpers
	static uint8_t getHWallHeight(uint8_t** rooms, const uint8_t x, const uint8_t y);
	static uint8_t getVWallWidth(uint8_t** rooms, const uint8_t x, const uint8_t y);
	static bool isConnectedSouth(uint8_t** rooms, const uint8_t x, const uint8_t y);
	static bool isConnectedEast(uint8_t** rooms, const uint8_t x, const uint8_t y);

	bool isRoomOnOutsideWall(const uint8_t x, const uint8_t y) const;

	// Helpers for choosing a random door
	void randomDoor(uint8_t& x, uint8_t& y, bool& east) const;
	// Chooses a random room that connects two different regions. This is guaranteed to succeed unless there is only one region.
	void randomDoorBetweenDifferentRegions(uint8_t& x, uint8_t& y, bool& east, uint8_t** mazeRegions) const;
	// Chooses a random room that connects to exactly one of the given regions.
	void randomDoorInOneOfTwoRegions(uint8_t& x, uint8_t& y, bool& east, uint8_t** mazeRegions, uint8_t region1, uint8_t region2) const;

	// Advances the selected door sequentially.
	// Repeated calls to this function will pass through every interior door
	// in the maze once before returning to the starting door.
	void nextDoor(uint8_t& x, uint8_t& y, bool& east) const;
	uint16_t doorToIndex(const uint8_t x, const uint8_t y, const bool east) const;
	void indexToDoor(uint8_t& x, uint8_t& y, bool& east, uint16_t index) const;

	// Helpers for choosing a random room
	void randomRoom(uint8_t& x, uint8_t& y) const;
	// Chooses a random room in the given region. This is guaranteed to succeed unless the region doesn't exist.
	void randomRoomInRegion(uint8_t& x, uint8_t& y, uint8_t** mazeRegions, uint8_t region) const;

	// Advances the selected room sequentially.
	// Repeated calls to this function will pass through every room
	// in the maze once before returning to the starting room.
	// The implementation is such that the new room will typically
	// not be adjacent to the old room, but this is not guaranteed.
	void nextRoom(uint8_t& x, uint8_t& y) const;

	// Valid for n < 32749.
	static void nextPseudorandom(uint16_t& k, const uint16_t n);

	// Pace-Level Helpers
	void doorOnWall(uint8_t** rooms, const uint8_t x, const uint8_t y) {
		// TODO
	}
	// DEBUG Converts the x,y coordinates of a room to coordinates of the center of the room in paces.
	void roomToPaces(uint8_t& x, uint8_t& y) {
		x = roomXToPaces(x);
		y = roomYToPaces(y);
	}
	// Gets the coordinates of the lower right corner of a room in paces.
	uint8_t roomXToPaces(const uint8_t x) {
		return x * ROOM_W + ROOM_W / 2 + MAZE_BORDER;
	}
	uint8_t roomYToPaces(const uint8_t y) {
		return y * ROOM_H + ROOM_H / 2 + MAZE_BORDER;
	}

	// Debug Printing
	void printMazeWallsAndRegions(uint8_t** mazeWalls, uint8_t** mazeRegions);
	void printRegionSizes(uint8_t* mazeRegionSizes);
};
