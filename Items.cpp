
#include "Items.h"

inline bool Items::isPlayer(uint8_t x, uint8_t y) {
	return x == _xPlayer && y == _yPlayer;
}
void Items::getPlayer(uint8_t& x, uint8_t& y) {
	x = _xPlayer;
	y = _yPlayer;
}
void Items::setPlayer(uint8_t x, uint8_t y) {
	_xPlayer = x;
	_yPlayer = y;
}

inline bool Items::isMidKey(uint8_t x, uint8_t y) {
	return x == _xMidKey && y == _yMidKey;
}
void Items::getMidKey(uint8_t& x, uint8_t& y) {
	x = _xMidKey;
	y = _yMidKey;
}
void Items::setMidKey(uint8_t x, uint8_t y) {
	_xMidKey = x;
	_yMidKey = y;
}

inline bool Items::isMidDoor(uint8_t x, uint8_t y) {
	return x == _xMidDoor && y == _yMidDoor;
}

inline bool Items::isExitKey(uint8_t x, uint8_t y) {
	return x == _xExitKey && y == _yExitKey;
}
void Items::getExitKey(uint8_t& x, uint8_t& y) {
	x = _xExitKey;
	y = _yExitKey;
}
void Items::setExitKey(uint8_t x, uint8_t y) {
	_xExitKey = x;
	_yExitKey = y;
}

inline bool Items::isExitDoor(uint8_t x, uint8_t y) {
	return x == _xExitDoor && y == _yExitDoor;
}
