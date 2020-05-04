
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

bool Items::isExitDoor(uint8_t x, uint8_t y) const {
	return x == _xExitDoor && y == _yExitDoor;
}
