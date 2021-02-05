
#include "Items.h"

bool Items::isPlayer(uint8_t x, uint8_t y) const {
	return x == _xPlayer && y == _yPlayer;
}
void Items::getPlayer(uint8_t& x, uint8_t& y) const {
	x = _xPlayer;
	y = _yPlayer;
}
void Items::setPlayer(uint8_t x, uint8_t y) {
	_xPlayer = x;
	_yPlayer = y;
}

bool Items::isMidKey(uint8_t x, uint8_t y) const {
	return x == _xMidKey && y == _yMidKey;
}
void Items::getMidKey(uint8_t& x, uint8_t& y) const {
	x = _xMidKey;
	y = _yMidKey;
}
void Items::setMidKey(uint8_t x, uint8_t y) {
	_xMidKey = x;
	_yMidKey = y;
}
void Items::clearMidKey() {
	_xMidKey = NO_PLACE;
	_yMidKey = NO_PLACE;
}

bool Items::isMidDoor(uint8_t x, uint8_t y) const {
	return x == _xMidDoor && y == _yMidDoor;
}

bool Items::isExitKey(uint8_t x, uint8_t y) const {
	return x == _xExitKey && y == _yExitKey;
}
void Items::getExitKey(uint8_t& x, uint8_t& y) const {
	x = _xExitKey;
	y = _yExitKey;
}
void Items::setExitKey(uint8_t x, uint8_t y) {
	_xExitKey = x;
	_yExitKey = y;
}
void Items::clearExitKey() {
	_xExitKey = NO_PLACE;
	_yExitKey = NO_PLACE;
}

bool Items::isExitDoor(uint8_t x, uint8_t y) const {
	return x == _xExitDoor && y == _yExitDoor;
}

int Items::distanceSqrToMidKey(uint8_t x, uint8_t y) const {
	// Return a large number if the key is not anywhere
	if (_xMidKey == NO_PLACE || _yMidKey == NO_PLACE) return INT16_MAX;

	int xDist = x - _xMidKey,
		yDist = y - _yMidKey;
	return xDist * xDist + yDist * yDist;
}
int Items::distanceSqrToExitKey(uint8_t x, uint8_t y) const {
	// Return a large number if the key is not anywhere
	if (_xExitKey == NO_PLACE || _yExitKey == NO_PLACE) return INT16_MAX;
	
	int xDist = x - _xExitKey,
		yDist = y - _yExitKey;
	return xDist * xDist + yDist * yDist;
}
