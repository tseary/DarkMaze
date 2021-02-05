
#include "MazeMaker.h"

MazeMaker* MazeMaker::instance = NULL;

void MazeMaker::setMazeDimensions(uint8_t roomsX, uint8_t roomsY, uint8_t roomWidth, uint8_t roomHeight) {
	ROOMS_X = roomsX;
	ROOMS_Y = roomsY;
	NUM_ROOMS = roomsX * roomsY;

	ROOM_W = roomWidth;
	ROOM_H = roomHeight;

	// Constants for door index
	NUM_DOORS_E = (ROOMS_X - 1) * ROOMS_Y;
	NUM_DOORS = NUM_DOORS_E + ROOMS_X * (ROOMS_Y - 1);

	MAZE_W = 2 * MAZE_BORDER + ROOMS_X * ROOM_W;
	MAZE_H = 2 * MAZE_BORDER + ROOMS_Y * ROOM_H;
}

void MazeMaker::createMaze(Maze*& maze) {
	/*
	* The maze at room level can be defined by w*(h-1)+(w-1)*h bits (40 bits for 5x5 rooms).
	*
	* _|_|_|_|_
	* _|_|_|_|_
	* _|_|_|_|_
	* _|_|_|_|_
	*  | | | |
	*
	* In order to determine connectivity, we need one byte per room (25 bytes).
	* Some rooms will already be connected at the start due to zero-thickness walls.
	*/

	// Define maze at the room level

	// mazeRooms contains the wall thickness and connectivity information for each room. It is indexed by [x][y].
	uint8_t** mazeRooms = new uint8_t * [ROOMS_X];

	// mazeRegions contains the region to which each room belongs. It is indexed by [x][y].
	uint8_t** mazeRegions = new uint8_t * [ROOMS_X];

	// mazeRegionSizes contains the number of rooms in each region. It is indexed by [region number].
	uint8_t* mazeRegionSizes = new uint8_t[NUM_ROOMS];

	Serial.println("Creating room-level model.");

	for (uint8_t x = 0; x < ROOMS_X; x++) {
		mazeRooms[x] = new uint8_t[ROOMS_Y];
		mazeRegions[x] = new uint8_t[ROOMS_Y];

		// Define regions and wall thicknesses
		// After this, the contents of mazeWalls will be accurate but
		// mazeRegions may contain regions that touch.
		for (uint8_t y = 0; y < ROOMS_Y; y++) {
			// Thickness of the vertical walls ||
			uint8_t vertWallW = random(ROOM_W / 2) & 0b111;
			// Thickness of the horizontal walls =
			uint8_t horzWallH = random(ROOM_H / 2) & 0b111;
			mazeRooms[x][y] = (horzWallH << H_SHIFT) | vertWallW;

			// Initial assignment of region and size
			uint8_t region = x + y * ROOMS_X;
			mazeRegions[x][y] = region;
			mazeRegionSizes[region] = 1;

			// Connect rooms that have zero wall thickness between them
			if (y >= 1 && horzWallH == 0 && getHWallHeight(mazeRooms, x, y - 1) == 0) {
				mazeRooms[x][y] |= S_MASK;
			}
			if (x >= 1 && vertWallW == 0 && getVWallWidth(mazeRooms, x - 1, y) == 0) {
				mazeRooms[x][y] |= E_MASK;
			}
		}
	}

	printRegionSizes(mazeRegionSizes);

	Serial.println("Combining regions by wall thickness...");

	// Combine regions that are connected by zero-thickness walls
	bool progress;
	do {
		progress = false;
		for (uint8_t y = 0; y < ROOMS_Y; y++) {
			for (uint8_t x = 0; x < ROOMS_X; x++) {
				// Connection to south
				if (isConnectedSouth(mazeRooms, x, y)) {
					progress |= replaceRegion(mazeRegions, mazeRegionSizes,
						mazeRegions[x][y], mazeRegions[x][y - 1]);
				}
				// Connection to east
				if (isConnectedEast(mazeRooms, x, y)) {
					progress |= replaceRegion(mazeRegions, mazeRegionSizes,
						mazeRegions[x][y], mazeRegions[x - 1][y]);
				}
			}
		}
	} while (progress);

	Serial.println("After combining by wall thickness:");
	printMazeWallsAndRegions(mazeRooms, mazeRegions);

	// The number of regions will be >= 1 and <= (ROOMS_X * ROOMS_Y)
	Serial.println("Creating doorways...");

	/*
	* TODO:
	* The method of combining regions until only two remain is flawed because
	* of the case where a large region fills the middle of the maze. The other smaller
	* regions surrounding it are not allowed to be connected because that makes the
	* middle region too big.
	*
	* Example:
	*  _ _ _ _ _
	* |   |_| |_|	 2| 2|22| 2|20		4
	* |_        |	 2| 2| 2| 2| 2		3
	* |      _ _|	 2| 2| 2| 2| 2		2
	* |_  |_|   |	 2| 2| 2| 0| 0		1
	* |_|_ _|_ _|	 4| 2| 2| 0| 0		0
	*
	* An alternate method is proposed:
	* 1. Choose two starting regions at random (not equal to each other).
	* 2. Grow the regions randomly by making connections to adjacent rooms.
	* e.g. Make connections from a region X room to a region 1 or 2 room.
	* Connections from region 1-1 or 2-2 are allowed, but not X-X.
	* 3. Continue until there are no more unassigned rooms.
	*
	* This method typically performs better, but it is still possible for
	* it to fail in the same way when using a certain maximum region size.
	*/

	// 1. Choose two starting regions
	uint8_t regionCount = countRegions(mazeRegionSizes);
	uint8_t regionIndex1 = random(regionCount),
		regionIndex2 = random(regionCount - 1);
	if (regionIndex2 >= regionIndex1) regionIndex2++;

	// Suppose there are N rooms and K regions of non-zero size.
	// The region indexes are numbers in [0,K).
	// This converts those to actual region numbers in [0,N).
	uint8_t startRegion1, startRegion2;
	for (uint8_t region = 0, regionIndex = 0; region < NUM_ROOMS; region++) {
		// If the size of the region is zero, skip it
		if (!mazeRegionSizes[region]) continue;

		// If we have reached one of the randomly-selected indexes, save the region
		if (regionIndex == regionIndex1) startRegion1 = region;
		if (regionIndex == regionIndex2) startRegion2 = region;

		regionIndex++;
	}

	Serial.println("1. Choose two starting regions");
	Serial.print("startRegion1 = ");
	Serial.println(startRegion1);
	Serial.print("startRegion2 = ");
	Serial.println(startRegion2);

	// 2. Create doorways at random until there are only 2 regions
	const uint8_t FINAL_REGION_COUNT = 2;
	const uint8_t MAX_REGION_SIZE = NUM_ROOMS;

	// DEBUG
	if (FINAL_REGION_COUNT * MAX_REGION_SIZE < NUM_ROOMS) {
		Serial.println("Assertion error: disagreement between "
			"FINAL_REGION_COUNT and MAX_REGION_SIZE.");
		return;
	}

	//uint8_t regionCount = countRegions(mazeRegionSizes);
	uint8_t debugNoChangeCounter = 0;
	while (regionCount > FINAL_REGION_COUNT && debugNoChangeCounter < 30) {
		// Choose random door location that touches exactly one of the starting regions
		uint8_t xDoor, yDoor;
		bool east;	// true = door to east, false = door to south

		// Start with a random door
		randomDoor(xDoor, yDoor, east);

		// Re-roll until we find a door touching exactly one starting region
		uint8_t region1, region2;
		for (uint16_t retry = 0; retry < NUM_DOORS; retry++) {
			// Check the success criteria
			region1 = mazeRegions[xDoor][yDoor];
			region2 = east ? mazeRegions[xDoor - 1][yDoor] : mazeRegions[xDoor][yDoor - 1];

			// Check if we already have this door
			bool alreadyHaveIt = mazeRooms[xDoor][yDoor] & (east ? E_MASK : S_MASK);

			// Break if a good door has been found
			if (!alreadyHaveIt && ((region1 == startRegion1 || region2 == startRegion1) ^
				(region1 == startRegion2 || region2 == startRegion2))) break;

			// Re-roll if the door isn't good
			nextDoor(xDoor, yDoor, east);
		}

		Serial.print("- creating door to ");
		Serial.print(east ? "east " : "south");
		Serial.print(" at ");
		Serial.print(xDoor);
		Serial.print(", ");
		if (yDoor <= 9) Serial.print(" ");
		Serial.println(yDoor);

		// Create the door
		mazeRooms[xDoor][yDoor] |= east ? E_MASK : S_MASK;

		// Swap the regions so that we keep the starting region
		if (region2 != startRegion1 && region2 != startRegion2) {
			uint8_t keeper = region1;
			region1 = region2;
			region2 = keeper;
		}

		// Combine the regions on either side of the door (they may be already connected)
		bool change = replaceRegion(mazeRegions, mazeRegionSizes, region1, region2);

		// Count down if a region was removed
		if (change) {
			regionCount--;

			debugNoChangeCounter = 0;

			// DEBUG
			Serial.print("regionCount = ");
			Serial.println(regionCount);
			printMazeWallsAndRegions(mazeRooms, mazeRegions);
		} else {
			debugNoChangeCounter++;
		}
	}

	Serial.println("After creating doorways:");
	printMazeWallsAndRegions(mazeRooms, mazeRegions);

	// Now mazeWalls and mazeRegions are accurate and there are at most 2 regions.
	// (Having just one region is very unlikely.)

	// It is possible that one region may have only one room,
	// and it is possible that one region may not touch the outer wall.

	/*
	* To finish building the maze at the room level, we must place some items.
	* The sequence determines which region each item is in.
	*
	* Items:
	* P - Player
	* k - Middle door key
	* M - Middle door
	* X - Exit door key
	* E - Exit door
	*
	* MAZE TOPOLOGY / SEQUENCE:
	*
	*			Linear			Parallel		Diversion		Closet (suitable when one region is very small):
	*			 _ _ _ _ _
	*			|1=2      |
	*			|         |
	*	Single	|  P      E			N/A				N/A				N/A
	*			|      X  |
	*			|_ _ _ _ _|
	*
	*			 _ _ _ _ _		 _ _ _ _ _		 _ _ _ _ _		 _ _ _ _ _
	*			|1    |2  |		|1    |2  |		|1  |2    |		|1  |2    |
	*			|  P  |   |		|  P  |   |		|   |  P  |		|   |  P  |
	*	Double	|     M   E		|     M   E		|   M     E		|   M     E
	*			|  k  |X  |		| k X |   |		|   | k X |		|  X|  k  |
	*			|_ _ _|_ _|		|_ _ _|_ _|		|_ _|_ _ _|		|_ _|_ _ _|
	*
	*			 _ _ _ _ _		 _ _ _ _ _		 _ _ _ _ _		 _ _ _ _ _
	*			|2 _ _ _ X|		|2 _ _ _  |		|2 _ _ _ k|		|2 _ _ _ k|
	*			| |1    | |		| |1   X| |		| |1    | |		| |1   X| |
	*	Nested	| |P k  M E		| |P k  M E		|P|     M E		|P|     M E
	*			| |_ _ _| |		| |_ _ _| |		| |_ _ _|X|		| |_ _ _| |
	*			|_ _ _ _ _|		|_ _ _ _ _|		|_ _ _ _ _|		|_ _ _ _ _|
	*
	*	Item regions by sequence:
	*				Player	Mid.key	Ex.key
	*	Linear		1		1		2
	*	Parallel	1		1		1
	*	Diversion	2		2		2
	*	Closet		2		2		1
	*
	*	The middle door is always between regions 1 and 2.
	*	The exit door is always in region 2.
	*
	* Sequence descriptions:
	* Linear: The player must escape from region 1 by finding the middle key.
	*		Then they must escape the maze by finding the exit key and exit door.
	*		In the single topology, the middle key and door are omitted.
	* Parallel: The player must find both keys in region 1, then escape through
	*		the middle door and exit door.
	* Diversion: The player must only escape region 2 by finding the exit key
	*		and exit door. The middle key unlocks region 1, which is empty.
	* Closet: The player must find the middle key to unlock region 1, which
	*		contains the exit key. They must then escape through the exit door.
	*/

	/*
	* Here we identify the topology of the maze regions.
	* This depends only upon the number of regions and their connection to the outside wall.
	* We also select a sequence, which determines which regions the items go in.
	*/

	const uint8_t TOPO_SINGLE = 1,
		TOPO_DOUBLE = 2,
		TOPO_NESTED = 3;
	const uint8_t SEQ_LINEAR = 1,
		SEQ_DIVERSION = 2,
		SEQ_PARALLEL = 3,
		SEQ_CLOSET = 4;

	uint8_t topology,
		sequence;

	if (regionCount == 1) {
		// Make the start regions equal and correct
		startRegion1 = mazeRegions[0][0];
		startRegion2 = mazeRegions[0][0];

		topology = TOPO_SINGLE;
		sequence = SEQ_LINEAR;
	} else {
		// Is any region not touching the outside wall?
		// The two regions are startRegion1 and startRegion2.
		bool touchingOutside1 = false,
			touchingOutside2 = false;
		// Check the vertical outside walls
		// This loop breaks if both regions are found to be touching the outside wall.
		for (uint8_t y = 0; y < ROOMS_Y && !(touchingOutside1 && touchingOutside2); y++) {
			// East wall
			uint8_t region = mazeRegions[0][y];
			touchingOutside1 |= region == startRegion1;
			touchingOutside2 |= region == startRegion2;

			// West wall
			region = mazeRegions[ROOMS_X - 1][y];
			touchingOutside1 |= region == startRegion1;
			touchingOutside2 |= region == startRegion2;
		}
		// Check the horizontal outside walls
		// This loop breaks if both regions are found to be touching the outside wall.
		for (uint8_t x = 1; x < (ROOMS_X - 1) && !(touchingOutside1 && touchingOutside2); x++) {
			// South wall
			uint8_t region = mazeRegions[x][0];
			touchingOutside1 |= region == startRegion1;
			touchingOutside2 |= region == startRegion2;

			// North wall
			region = region = mazeRegions[x][ROOMS_Y - 1];
			touchingOutside1 |= region == startRegion1;
			touchingOutside2 |= region == startRegion2;
		}

		// If startRegion2 is not touching the outside wall, swap with startRegion1
		if (!touchingOutside2) {
			uint8_t swapRegion = startRegion1;
			startRegion1 = startRegion2;
			startRegion2 = swapRegion;

			bool swapTouching = touchingOutside1;
			touchingOutside1 = touchingOutside2;
			touchingOutside2 = swapTouching;
		}

		// Determine if it is TOPO_DOUBLE or TOPO_NESTED
		topology = touchingOutside1 ? TOPO_DOUBLE : TOPO_NESTED;

		// Select a sequence randomly
		sequence = random(4) + 1;
	}

	// Now startRegion2 is definitely touching the outside wall.
	// In TOPO_SINGLE, startRegion1 == startRegion2.
	// In TOPO_NESTED, startRegion1 is not touching the outside wall.

	Serial.print("Topology: ");
	switch (topology) {
	case TOPO_SINGLE:
		Serial.println("SINGLE");
		break;

	case TOPO_DOUBLE:
		Serial.println("DOUBLE");
		break;

	case TOPO_NESTED:
		Serial.println("NESTED");
		break;
	}

	// Choose the regions for each item depending on the sequence
	uint8_t itemRegionPlayer,
		itemRegionMidKey,
		itemRegionExitKey;

	Serial.print("Sequence: ");
	switch (sequence) {
	case SEQ_LINEAR:
		itemRegionPlayer = startRegion1;
		itemRegionExitKey = startRegion2;
		Serial.println("LINEAR");
		break;

	case SEQ_PARALLEL:
		itemRegionPlayer = startRegion1;
		itemRegionExitKey = startRegion1;
		Serial.println("PARALLEL");
		break;

	case SEQ_DIVERSION:
		itemRegionPlayer = startRegion2;
		itemRegionExitKey = startRegion2;
		Serial.println("DIVERSION");
		break;

	case SEQ_CLOSET:
		itemRegionPlayer = startRegion2;
		itemRegionExitKey = startRegion1;
		Serial.println("CLOSET");
		break;
	}
	Serial.println();

	itemRegionMidKey = itemRegionPlayer;

	// For TOPO_DOUBLE, if one region has only one room, swap appropriately
	// so that the player does not start in the one-room region.
	if (topology == TOPO_DOUBLE && mazeRegionSizes[itemRegionPlayer] == 1) {
		uint8_t swapRegion = startRegion1;
		startRegion1 = startRegion2;
		startRegion2 = swapRegion;
	}

	// Choose rooms to put things in, in the appropriate regions
	uint8_t xPlayer, yPlayer;
	uint8_t xMidKey, yMidKey;
	uint8_t xExitKey, yExitKey;
	randomRoomInRegion(xPlayer, yPlayer, mazeRegions, itemRegionPlayer);
	randomRoomInRegion(xMidKey, yMidKey, mazeRegions, itemRegionMidKey);
	randomRoomInRegion(xExitKey, yExitKey, mazeRegions, itemRegionExitKey);

	const uint8_t DIR_N = 0, DIR_E = 1, DIR_S = 2, DIR_W = 3;

	// Choose door locations
	uint8_t xMidDoor, yMidDoor;
	bool dirMidDoorEast;	// If true, the middle door is between rooms to the east/west of each other.
	uint8_t xExitDoor, yExitDoor;
	uint8_t dirExitDoor;

	// Choose middle door location, if applicable. Must be between regions.
	if (topology != TOPO_SINGLE) {
		randomDoorBetweenDifferentRegions(xMidDoor, yMidDoor, dirMidDoorEast, mazeRegions);

		Serial.print("Location of middle door:\tx = ");
		Serial.print(xMidDoor);
		Serial.print("\ty = ");
		Serial.print(yMidDoor);
		Serial.print("\tdir = ");
		Serial.println(dirMidDoorEast ? "east" : "south");

		// Indicate that the rooms are connected, but do not combine regions
		mazeRooms[xMidDoor][yMidDoor] |= dirMidDoorEast ? E_MASK : S_MASK;
	} else {
		// The single topology has no middle door
		xMidDoor = -1;
		yMidDoor = -1;
		dirMidDoorEast = false;
	}

	// Choose exit door location. Must be in startRegion2 and on outside wall.
	randomRoomInRegion(xExitDoor, yExitDoor, mazeRegions, startRegion2);
	while (!isRoomOnOutsideWall(xExitDoor, yExitDoor)) {
		// TODO faulty implementation, does not guarantee room on outside wall
		nextRoom(xExitDoor, yExitDoor);
	}
	Serial.print("Location of exit door:\tx = ");
	Serial.print(xExitDoor);
	Serial.print("\ty = ");
	Serial.println(yExitDoor);

	//
	// Render at the pace level
	//

	// Define wall pointer
	maze_t* myWalls = new maze_t[MAZE_H];

	// Make empty border
	for (uint8_t y = 0; y < MAZE_H; y++) {
		const uint32_t VERTICAL_WALL = ~(~(0xffffffff << (ROOMS_X * ROOM_W)) << MAZE_BORDER);
		myWalls[y] = (y < MAZE_BORDER || y >= (ROOMS_Y * ROOM_H + MAZE_BORDER)) ?
			0xffffffff : VERTICAL_WALL;
	}

	/*
	* Each room is rectangular, with walls of random thickness.
	* Doors between rooms are added later.
	*
	* Example room with xWallW = 2 and yWallH = 1:
	*
	* ############
	* ####.'.'####
	* ####.'.'####
	* ####.'.'####
	* ####.'.'####
	* ############
	*
	* Each byte in mazeWalls:
	* 0bShhhEwww
	* S: 1 = no walls, or doorway to south, 0 = wall to south.
	* hhh: horizontal wall height ==
	* E: 1 = no walls, or doorway to east, 0 = wall to east.
	* www: vertical wall width ||
	*/
	for (uint8_t yRoom = 0; yRoom < ROOMS_Y; yRoom++) {
		for (uint8_t xRoom = 0; xRoom < ROOMS_X; xRoom++) {
			// Thickness of the vertical walls ||
			const uint8_t vertWallW = getVWallWidth(mazeRooms, xRoom, yRoom);
			// Thickness of the horizontal walls ==
			const uint8_t horzWallH = getHWallHeight(mazeRooms, xRoom, yRoom);

			// Add walls on each row of the room
			for (uint8_t yInRoom = 0; yInRoom < ROOM_H; yInRoom++) {
				uint32_t wall;
				if (yInRoom >= horzWallH && yInRoom < (ROOM_H - horzWallH)) {
					// A wall bounding the left and right of the room, in maze coordinates
					uint32_t vertWall = ~(0xffffffff << vertWallW);	// left wall
					vertWall = ~(vertWall << (ROOM_W - 2 * vertWallW));	// center of the room (inverted)
					vertWall = ~(vertWall << vertWallW);	// right wall
					vertWall <<= (MAZE_BORDER + xRoom * ROOM_W);	// maze coordinates

					// Vertical wall
					wall = vertWall;
				} else {
					// A wall the full width of the room, in maze coordinates
					uint32_t horzWall = ~(0xffffffff << ROOM_W);	// full wall
					horzWall <<= (MAZE_BORDER + xRoom * ROOM_W);	// maze coordinates

					// Horizontal wall
					wall = horzWall;
				}
				myWalls[MAZE_BORDER + yRoom * ROOM_H + yInRoom] |= wall;
			}
		}
	}

	// Render doorways
	for (uint8_t xRoom = 0; xRoom < ROOMS_X; xRoom++) {
		const uint32_t EAST_DOOR_MASK = ~(~(0xffffffff << ROOM_W) <<
			(xRoom * ROOM_W + MAZE_BORDER - ROOM_W / 2));
		Serial.print("EAST_DOOR_MASK =\t");
		Serial.println(EAST_DOOR_MASK, BIN);

		const uint32_t SOUTH_DOOR_MASK =
			~(0x1 << (xRoom * ROOM_W + MAZE_BORDER + ROOM_W / 2));
		Serial.print("SOUTH_DOOR_MASK =\t");
		Serial.println(SOUTH_DOOR_MASK, BIN);

		for (uint8_t yRoom = 0; yRoom < ROOMS_Y; yRoom++) {
			uint8_t yCenter = yRoom * ROOM_H + MAZE_BORDER + ROOM_H / 2;

			if (isConnectedEast(mazeRooms, xRoom, yRoom)) {
				myWalls[yCenter] &= EAST_DOOR_MASK;
			}

			if (isConnectedSouth(mazeRooms, xRoom, yRoom)) {
				for (uint8_t yPace = yCenter; yPace > yCenter - ROOM_H; yPace--) {
					myWalls[yPace] &= SOUTH_DOOR_MASK;
				}
			}
		}
	}

	// Re-create maze object and load in myWalls
	if (maze != NULL) {
		delete maze;
	}
	maze = new Maze(MAZE_W, MAZE_H);
	maze->setAllWalls(myWalls, MAZE_W, MAZE_H);

	// DEBUG Place Player (convert room to paces)
	roomToPaces(xPlayer, yPlayer);
	maze->items->setPlayer(xPlayer, yPlayer);
	roomToPaces(xMidKey, yMidKey);
	maze->items->setMidKey(xMidKey, yMidKey);
	roomToPaces(xExitKey, yExitKey);
	maze->items->setExitKey(xExitKey, yExitKey);

	// Clean up
	delete[] myWalls;
	for (uint8_t xRoom = 0; xRoom < ROOMS_X; xRoom++) {
		delete[] mazeRooms[xRoom];
		delete[] mazeRegions[xRoom];
	}
	delete[] mazeRooms;
	delete[] mazeRegions;
	delete[] mazeRegionSizes;
}

//
// Region-Level Helpers
//

// Returns true if a change was made.
bool MazeMaker::replaceRegion(uint8_t** mazeRegions, uint8_t* mazeRegionSizes,
	const uint8_t oldRegion, const uint8_t newRegion) const {
	// Do nothing if the regions are the same
	if (oldRegion == newRegion) {
		return false;
	}

	// Re-assign all the rooms in the old region
	bool change = false;
	for (uint8_t y = 0; y < ROOMS_Y; y++) {
		for (uint8_t x = 0; x < ROOMS_X; x++) {
			if (mazeRegions[x][y] == oldRegion) {
				mazeRegions[x][y] = newRegion;
				change = true;
			}
		}
	}

	// Update the region sizes
	if (change) {
		mazeRegionSizes[newRegion] += mazeRegionSizes[oldRegion];
		mazeRegionSizes[oldRegion] = 0;
	}

	return change;
}

uint8_t MazeMaker::countRegions(const uint8_t* mazeRegionSizes) const {
	uint8_t numRegions = 0;
	for (uint8_t region = 0; region < NUM_ROOMS; region++) {
		if (mazeRegionSizes[region]) numRegions++;
	}
	return numRegions;
}

//
// Room-Level Helpers
//

inline uint8_t MazeMaker::getHWallHeight(uint8_t** rooms, const uint8_t x, const uint8_t y) {
	return (rooms[x][y] & H_MASK) >> H_SHIFT;
}

inline uint8_t MazeMaker::getVWallWidth(uint8_t** rooms, const uint8_t x, const uint8_t y) {
	return rooms[x][y] & W_MASK;
}

inline bool MazeMaker::isConnectedSouth(uint8_t** rooms, const uint8_t x, const uint8_t y) {
	return y >= 1 && rooms[x][y] & S_MASK;
}

inline bool MazeMaker::isConnectedEast(uint8_t** rooms, const uint8_t x, const uint8_t y) {
	return x >= 1 && rooms[x][y] & E_MASK;
}

inline bool MazeMaker::isRoomOnOutsideWall(const uint8_t x, const uint8_t y) const {
	return x == 0 || x == (MAZE_W - 1) || y == 0 || y == (MAZE_H - 1);
}

//
// Helpers for choosing a random door
//

void MazeMaker::randomDoorBetweenDifferentRegions(uint8_t& x, uint8_t& y, bool& east, uint8_t** mazeRegions) const {
	// Start with a random door
	randomDoor(x, y, east);

	// Re-roll until we find a door between different regions
	bool good = false;
	for (uint16_t retry = 0; retry < NUM_DOORS && !good; retry++) {
		nextDoor(x, y, east);

		// Check the success condition
		uint8_t myRegion1 = mazeRegions[x][y],
			myRegion2 = east ? mazeRegions[x - 1][y] : mazeRegions[x][y - 1];
		good = myRegion1 != myRegion2;
	}
}

inline void MazeMaker::randomDoor(uint8_t& x, uint8_t& y, bool& east) const {
	indexToDoor(x, y, east, random(NUM_DOORS));
}

void MazeMaker::nextDoor(uint8_t& x, uint8_t& y, bool& east) const {
	uint16_t index = doorToIndex(x, y, east);
	nextPseudorandom(index, NUM_DOORS);
	indexToDoor(x, y, east, index);
}

inline uint16_t MazeMaker::doorToIndex(const uint8_t x, const uint8_t y, const bool east) const {
	return east ? (x - 1) + y * (ROOMS_X - 1) : NUM_DOORS_E + x + (y - 1) * ROOMS_X;
}

inline void MazeMaker::indexToDoor(uint8_t& x, uint8_t& y, bool& east, uint16_t index) const {
	if (east = (index < NUM_DOORS_E)) {
		// Door to east
		x = index % (ROOMS_X - 1) + 1;
		y = index / (ROOMS_X - 1);
	} else {
		// Door to south
		index -= NUM_DOORS_E;
		x = index % ROOMS_X;
		y = index / ROOMS_X + 1;
	}
}

//
// Helpers for choosing a random room
//

void MazeMaker::randomRoom(uint8_t& x, uint8_t& y) const {
	x = random(ROOMS_X);
	y = random(ROOMS_Y);
}

void MazeMaker::randomRoomInRegion(uint8_t& x, uint8_t& y, uint8_t** mazeRegions, uint8_t region) const {
	// Pick a random room
	randomRoom(x, y);

	// Re-roll until we find a room in the desired region
	for (uint8_t retry = 0; retry < NUM_ROOMS && mazeRegions[x][y] != region; retry++) {
		nextRoom(x, y);
	}
}

// Advances the selected room sequentially.
// Repeated calls to this function will pass through every room
// in the maze once before returning to the starting room.
// The implementation is such that the new room will typically
// not be adjacent to the old room, but this is not guaranteed.
void MazeMaker::nextRoom(uint8_t& x, uint8_t& y) const {
	uint16_t roomIndex = x + y * ROOMS_X;
	nextPseudorandom(roomIndex, NUM_ROOMS);

	x = roomIndex % ROOMS_X;
	y = roomIndex / ROOMS_X;
}

// Valid for n < 32749.
inline void MazeMaker::nextPseudorandom(uint16_t& k, const uint16_t n) {
	// From 0x1000: The previous prime is 32749. The next prime is 32771.
	const uint16_t PRIME = 32749;
	k = (k + PRIME) % n;
}

//
// Debug Printing
//

void MazeMaker::printMazeWallsAndRegions(uint8_t** rooms, uint8_t** mazeRegions) {
	for (int x = ROOMS_X - 1; x >= 0; x--) {
		Serial.print(" _");
	}
	Serial.println();

	for (int y = ROOMS_Y - 1; y >= 0; y--) {
		Serial.print('|');

		for (int x = ROOMS_X - 1; x >= 0; x--) {
			Serial.print(isConnectedSouth(rooms, x, y) ? ' ' : '_');
			Serial.print(isConnectedEast(rooms, x, y) ? ' ' : '|');
		}

		Serial.print('\t');

		for (int x = ROOMS_X - 1; x >= 0; x--) {
			if (mazeRegions[x][y] <= 9) Serial.print(' ');
			Serial.print(mazeRegions[x][y]);
			if (x > 0) Serial.print('|');
		}

		Serial.print('\t');
		Serial.println(y);
	}
	Serial.println();
}

void MazeMaker::printRegionSizes(uint8_t* mazeRegionSizes) {
	Serial.println("region\tsize");
	for (uint8_t region = 0; region < NUM_ROOMS; region++) {
		Serial.print(region);
		Serial.print(" ... \t");
		Serial.println(mazeRegionSizes[region]);
	}
	Serial.println();
}
