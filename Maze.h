#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "Items.h"

/*
* This class holds the data that defines a maze:
* - walls
* - doors
* It does not include items in the maze, e.g.:
* - player
* - keys
*
* This class is not responsible for creating the maze or ensuring that it
* is solvable. The maze is populated by the MazeMaker class.
*/

typedef uint32_t maze_t;

class Maze {
public:
	Maze(uint8_t width, uint8_t height) {
		_mazeWidth = width;
		_mazeHeight = height;
		_walls = new maze_t[height];

		items = new Items();
	}

	~Maze() {
		delete[] _walls;
		delete items;
	}

	// The positions of items in the maze
	Items* items;

	bool isWall(uint8_t x, uint8_t y) const;
	void setWall(uint8_t x, uint8_t y, bool wall);

	// DEBUG
	maze_t getRow(uint8_t y) const {
		return _walls[y];
	}

	void setAllWalls(const maze_t *walls, const uint8_t width, const uint8_t height);

	uint8_t getWidth() const;
	uint8_t getHeight() const;

	void printMaze() const {
		for (int y = _mazeHeight - 1; y >= 0; y--) {

			Serial.print("y=\t");
			Serial.print(y);
			Serial.print("\t");

			for (int x = _mazeWidth - 1; x >= 0; x--) {
				if (isWall(x, y)) {
					// Wall
					Serial.print("[]");
				} else if (items->isPlayer(x, y)) {
					// Player
					Serial.print("P1");
				} else if (items->isMidKey(x, y)) {
					// Middle key
					Serial.print("MK");
				} else if (items->isExitKey(x, y)) {
					// Exit key
					Serial.print("EK");
				} else {
					// Space
					Serial.print(". ");
				}
			}
			Serial.println();
		}
		Serial.println();

		// Print the maze data as a hex array
		/*for (int y = _mazeHeight - 1; y >= 0; y--) {
			Serial.print("walls[");
			Serial.print(y);
			Serial.print("] = 0x");
			Serial.print(maze->getRow(y), HEX);
			Serial.println(";");
		}
		Serial.println();*/
	}

private:
	// Walls (rendered at the pace level)
	// Each bit is one tile. The LSB is the easternmost wall,
	// and walls[0] is the southernmost.
	maze_t* _walls = NULL;

	// Maze size
	uint8_t _mazeWidth, _mazeHeight;
};
