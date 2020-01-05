
#pragma once

#include "Maze.h"

class MazeMaker {
public:
	static void createMaze(Maze* &maze, uint8_t& xPlayer, uint8_t& yPlayer) {
		if (maze != NULL) {
			delete maze;
		}

		maze = new Maze(22, 22);

		maze->walls[21] = 0xFFFFFFFF;
		maze->walls[20] = 0xFFFF07C1;
		maze->walls[19] = 0xFFF10001;
		maze->walls[18] = 0xFFF00001;
		maze->walls[17] = 0xFFF10001;
		maze->walls[16] = 0xFFFF06C1;
		maze->walls[15] = 0xFFFFDEC1;
		maze->walls[14] = 0xFFE08C41;
		maze->walls[13] = 0xFFE00001;
		maze->walls[12] = 0xFFE08C41;
		maze->walls[11] = 0xFFFFDFC1;
		maze->walls[10] = 0xFFF18FF7;
		maze->walls[9] = 0xFFF18C41;
		maze->walls[8] = 0xFFF00001;
		maze->walls[7] = 0xFFF18C41;
		maze->walls[6] = 0xFFF18EFF;
		maze->walls[5] = 0xFFFF06FF;
		maze->walls[4] = 0xFFE00023;
		maze->walls[3] = 0xFFE00023;
		maze->walls[2] = 0xFFE00023;
		maze->walls[1] = 0xFFFF07FF;
		maze->walls[0] = 0xFFFFFFFF;

		/*for (uint8_t y = 0; y < maze->getHeight(); y++) {
			for (uint8_t x = 0; x < maze->getWidth(); x++) {
				maze->setWall(x, y, x == 0 || x == (maze->getWidth() - 1) ||
					y == 0 || y == (maze->getHeight() - 1));
			}
		}*/

		xPlayer = 20;
		yPlayer = 15;
	}

private:
	MazeMaker() {}
};
