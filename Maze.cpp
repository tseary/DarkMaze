
#include "Maze.h"

bool Maze::isWall(uint8_t x, uint8_t y) const {
	return _walls[y] & ((maze_t)1 << x);
}

void Maze::setWall(uint8_t x, uint8_t y, bool wall) {
	const maze_t WALL_MASK = (maze_t)1 << x;
	if (wall) {
		_walls[y] |= WALL_MASK;
	} else {
		_walls[y] &= ~WALL_MASK;
	}
}

void Maze::setAllWalls(const maze_t* walls, const uint8_t width, const uint8_t height) {
	// Define the size of the maze
	_mazeWidth = width;
	_mazeHeight = height;

	if (_walls != NULL) 		delete[] _walls;

	// Deep copy the walls array
	_walls = new maze_t[_mazeHeight];
	for (uint8_t y = 0; y < _mazeHeight; y++) {
		_walls[y] = walls[y];
	}
}

uint8_t Maze::getWidth() const {
	return _mazeWidth;
}

uint8_t Maze::getHeight() const {
	return _mazeHeight;
}

void Maze::printMaze() const {

	// Print the size of the maze
	Serial.print("w=\t");
	Serial.print(_mazeWidth);
	Serial.print("\th=\t");
	Serial.println(_mazeHeight);

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
			} else if (items->isMidDoor(x, y)) {
				// Middle key
				Serial.print("MD");
			} else if (items->isExitDoor(x, y)) {
				// Exit key
				Serial.print("ED");
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
