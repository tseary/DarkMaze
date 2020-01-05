
#pragma once

#include "Maze.h"

class MazeMaker {
public:
	static void createMaze(Maze* maze, uint8_t& xPlayer, uint8_t& yPlayer) {
		maze->walls[0]  = 0xFFFFFFFF;
		maze->walls[1]  = 0xFF9C3FFF;
		maze->walls[2]  = 0xE79C30C3;
		maze->walls[3]  = 0xE79C30C3;
		maze->walls[4]  = 0xE79C30C3;
		maze->walls[5]  = 0xE79C30C3;
		maze->walls[6]  = 0xFF9C3FFF;
		maze->walls[7]  = 0xFFFFE07F;
		maze->walls[8]  = 0xE7FFE043;
		maze->walls[9]  = 0xE79C2043;
		maze->walls[10] = 0xE79C2043;
		maze->walls[11] = 0xE7FFE043;
		maze->walls[12] = 0xFFFFE07F;
		maze->walls[13] = 0xFE043FFF;
		maze->walls[14] = 0xFE043FFF;
		maze->walls[15] = 0x80042043;
		maze->walls[16] = 0x80042043;
		maze->walls[17] = 0xFE043FFF;
		maze->walls[18] = 0xFE043FFF;
		maze->walls[19] = 0x80001FE7;
		maze->walls[20] = 0x80000067;
		maze->walls[21] = 0x80000067;
		maze->walls[22] = 0x80000067;
		maze->walls[23] = 0x80000067;
		maze->walls[24] = 0x80001FE7;
		maze->walls[25] = 0xFFFFFFFF;
		maze->walls[26] = 0xE7FFFFFF;
		maze->walls[27] = 0xE79810E7;
		maze->walls[28] = 0xE79810E7;
		maze->walls[29] = 0xE7FFFFFF;
		maze->walls[30] = 0xFFFFFFFF;
		maze->walls[31] = 0xFFFFFFFF;

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
