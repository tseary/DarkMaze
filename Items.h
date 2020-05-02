#pragma once

#include <Arduino.h>

/*
* This class holds the items in the maze, e.g.:
* - player
* - keys
* - doors
*/

class Items {
public:
	static const uint8_t
		ITEM_PLAYER = 1,
		ITEM_MID_DOOR = 95,
		ITEM_MID_KEY = 75,
		ITEM_EXIT_DOOR = 99,
		ITEM_EXIT_KEY = 79;

	inline bool isPlayer(uint8_t x, uint8_t y) {
		return x == _xPlayer && y == _yPlayer;
	}
	void getPlayer(uint8_t& x, uint8_t& y) {
		x = _xPlayer;
		y = _yPlayer;
	}
	void setPlayer(uint8_t x, uint8_t y) {
		_xPlayer = x;
		_yPlayer = y;
	}

	inline bool isMidDoor(uint8_t x, uint8_t y);
	inline bool isMidKey(uint8_t x, uint8_t y);

	inline bool isExitDoor(uint8_t x, uint8_t y);
	inline bool isExitKey(uint8_t x, uint8_t y);

private:
	uint8_t _xPlayer, _yPlayer;
	uint8_t _xMidDoor, _yMidDoor;
	uint8_t _xMidKey, _yMidKey;
	uint8_t _xExitDoor, _yExitDoor;
	uint8_t _xExitKey, _yExitKey;
};
