
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
