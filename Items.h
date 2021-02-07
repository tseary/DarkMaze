#pragma once

#include <Arduino.h>
#include <stdint.h>

/*
* This class holds the items in the maze, e.g.:
* - player
* - keys
* - doors
*/

class Items {
public:
	Items() {
		// Initially clear everything
		clearMidKey();
		clearExitKey();
		clearMidDoor();
		clearExitDoor();
	}

	bool isPlayer(uint8_t x, uint8_t y) const;
	void getPlayer(uint8_t& x, uint8_t& y) const;
	void setPlayer(uint8_t x, uint8_t y);

	bool isMidKey(uint8_t x, uint8_t y) const;
	void getMidKey(uint8_t& x, uint8_t& y) const;
	void setMidKey(uint8_t x, uint8_t y);
	void clearMidKey();

	bool isMidDoor(uint8_t x, uint8_t y) const;
	void setMidDoor(uint8_t x, uint8_t y);
	void clearMidDoor();

	bool isExitKey(uint8_t x, uint8_t y) const;
	void getExitKey(uint8_t& x, uint8_t& y) const;
	void setExitKey(uint8_t x, uint8_t y);
	void clearExitKey();

	bool isExitDoor(uint8_t x, uint8_t y) const;
	void setExitDoor(uint8_t x, uint8_t y);
	void clearExitDoor();

	int distanceSqrToMidKey(uint8_t x, uint8_t y) const;
	int distanceSqrToExitKey(uint8_t x, uint8_t y) const;
	int distanceSqrToNearestKey(uint8_t x, uint8_t y) const {
		return min(distanceSqrToMidKey(x, y), distanceSqrToExitKey(x, y));
	}

private:
	uint8_t _xPlayer, _yPlayer;
	uint8_t _xMidKey, _yMidKey;
	uint8_t _xMidDoor, _yMidDoor;
	uint8_t _xExitKey, _yExitKey;
	uint8_t _xExitDoor, _yExitDoor;

	const uint8_t NO_PLACE = 255;
};
