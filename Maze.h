
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
	Maze(uint8_t width, uint8_t height) {
		// Define the size of the maze
		_mazeWidth = min(width, 8 * sizeof(maze_t));
		_mazeHeight = height;

		// Allocate the walls
		walls = new maze_t[_mazeHeight];
	}

	inline bool isWall(uint8_t x, uint8_t y) {
		return walls[y] & ((maze_t)1 << x);
	}

	inline void setWall(uint8_t x, uint8_t y, bool wall) {
		const maze_t WALL_MASK = (maze_t)1 << x;
		if (wall) {
			walls[y] |= WALL_MASK;
		} else {
			walls[y] &= ~WALL_MASK;
		}
	}

	inline uint8_t getWidth() {
		return _mazeWidth;
	}

	inline uint8_t getHeight() {
		return _mazeHeight;
	}

private:
	// Maze size
	uint8_t _mazeWidth, _mazeHeight;

public:	// DEBUG
	// Walls (rendered at the pace level)
	// Each bit is one tile. The LSB is the easternmost wall,
	// and walls[0] is the southernmost.
	maze_t* walls;
};
