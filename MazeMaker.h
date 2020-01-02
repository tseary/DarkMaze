
#pragma once

#include "Maze.h"

class MazeMaker {
public:
	static void createMaze(Maze* maze, uint8_t& xPlayer, uint8_t& yPlayer) {
		for (uint8_t y = 0; y < maze->getHeight(); y++) {
			for (uint8_t x = 0; x < maze->getWidth(); x++) {
				maze->setWall(x, y, x == 0 || x == (maze->getWidth() - 1) ||
					y == 0 || y == (maze->getHeight() - 1));
			}
		}

		xPlayer = 10;
		yPlayer = 15;
	}

private:
	MazeMaker() {}
};
