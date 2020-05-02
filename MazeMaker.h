#pragma once

#include "Items.h"
#include "Maze.h"
#include <Arduino.h>

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

	void createMaze(Maze*& maze);

	/**
	Pass in a NULL pointer for maze. It will be instantiated and populated with a valid maze.
	*/
	/*static void createMaze(Maze*& maze, uint8_t width, uint8_t height) {
		if (maze != NULL) {
			delete maze;
		}

		// DEBUG static maze
		maze_t *myWalls = new maze_t[22];

		myWalls[21] = 0xFFFFFFFF;
		myWalls[20] = 0xFFFF07C1;
		myWalls[19] = 0xFFF10001;
		myWalls[18] = 0xFFF00001;
		myWalls[17] = 0xFFF10001;
		myWalls[16] = 0xFFFF06C1;
		myWalls[15] = 0xFFFFDEC1;
		myWalls[14] = 0xFFE08C41;
		myWalls[13] = 0xFFE00001;
		myWalls[12] = 0xFFE08C41;
		myWalls[11] = 0xFFFFDFC1;
		myWalls[10] = 0xFFF18FF7;
		myWalls[9] = 0xFFF18C41;
		myWalls[8] = 0xFFF00001;
		myWalls[7] = 0xFFF18C41;
		myWalls[6] = 0xFFF18EFF;
		myWalls[5] = 0xFFFF06FF;
		myWalls[4] = 0xFFE00023;
		myWalls[3] = 0xFFE00023;
		myWalls[2] = 0xFFE00023;
		myWalls[1] = 0xFFFF07FF;
		myWalls[0] = 0xFFFFFFFF;

		maze = new Maze(22, 22);
		maze->setAllWalls(myWalls, 22, 22);

		delete[] myWalls;

		//for (uint8_t y = 0; y < maze->getHeight(); y++) {
		//	for (uint8_t x = 0; x < maze->getWidth(); x++) {
		//		maze->setWall(x, y, x == 0 || x == (maze->getWidth() - 1) ||
		//			y == 0 || y == (maze->getHeight() - 1));
		//	}
		//}

		maze->items->setPlayer(20, 15);
	}*/

private:
	// Singleton
	MazeMaker() {}
	static MazeMaker* instance;

	// The number of rooms in the maze
	uint8_t ROOMS_X, ROOMS_Y;
	uint8_t ROOMS;
	// The size of a room in paces
	uint8_t ROOM_W, ROOM_H;

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
	static inline uint8_t getHWallHeight(uint8_t** rooms, const uint8_t x, const uint8_t y);
	static inline uint8_t getVWallWidth(uint8_t** rooms, const uint8_t x, const uint8_t y);
	static inline bool isConnectedSouth(uint8_t** rooms, const uint8_t x, const uint8_t y);
	static inline bool isConnectedEast(uint8_t** rooms, const uint8_t x, const uint8_t y);

	// Helper for choosing a random room
	void randomRoom(uint8_t& x, uint8_t& y);

	// Advances the selected room sequentially.
	// Repeated calls to this function will pass through every room
	// in the maze once before returning to the starting room.
	// The implementation is such that the new room will typically
	// not be adjacent to the old room, but this is not guaranteed.
	void nextRoom(uint8_t& x, uint8_t& y);

	// Valid for n < 32749.
	static inline void nextPseudorandom(uint16_t& k, const uint16_t n);

	// Debug Printing
	void printMazeWallsAndRegions(uint8_t** mazeWalls, uint8_t** mazeRegions);
	void printMaze(const Maze* maze);
	void printRegionSizes(uint8_t* mazeRegionSizes);
};
