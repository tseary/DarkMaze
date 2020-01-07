
#pragma once

#include <Arduino.h>
//#include <stdint.h>

/*
* This class holds the data that defines a maze:
* - walls
* - doors
* It does not include items in the maze, e.g.:
* - player
* - keys
*
* This class also is not responsible for creating the maze or ensuring that it
* is solvable. The maze is populated by the MazeMaker class.
*/

typedef uint32_t maze_t;

class Maze {
public:
	Maze() {}

	~Maze() {
		delete[] _walls;
	}

	bool isWall(uint8_t x, uint8_t y);
	void setWall(uint8_t x, uint8_t y, bool wall);

	void setAllWalls(const maze_t *walls, const uint8_t width, const uint8_t height);

	uint8_t getWidth();
	uint8_t getHeight();

private:
	// Walls (rendered at the pace level)
	// Each bit is one tile. The LSB is the easternmost wall,
	// and walls[0] is the southernmost.
	maze_t* _walls = NULL;

	// Maze size
	uint8_t _mazeWidth, _mazeHeight;
};
