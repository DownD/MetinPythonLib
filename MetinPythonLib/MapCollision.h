#pragma once
#include "stdafx.h"
#include "../common/utils.h"
#include "defines.h"
#include "JPS.h"
#include <ANYA.h>


//#define USE_JPS_PATHPLANNING
#define USE_ANYA_PATHPLANNING

class AreaData {

public:
	struct Object {
		float x, y, z;
		unsigned int crc; //Not working
		float rotX, rotY, rotZ;
		float uknown;
	};
	void loadFile(char* buffer, int size);

	std::vector<Object> vec;
};

class MapCollision
{
public:
	MapCollision(const char* map_name);
	~MapCollision();


	inline bool isBlocked(int x, int y);
	inline BYTE getByte(int x, int y) { return map[y * maxX + x]; }; //Deprecated, is equal to isBlocked function

	const char* getMapName() { return mapName.c_str(); }
	bool findPath(int x_start, int y_start, int x_end, int y_end, std::vector<Point>& path);

	inline unsigned operator()(unsigned x, unsigned y) const;
private:

	struct MapPiece{
		MapPiece(EterFile* attrFile ,int x, int y, std::string path);
		~MapPiece();

		BYTE* getMapData();
		void printToFile(const char* name);

		//Adds objects to the attr map file (impossible to know objects sizes)

		WORD version;
		WORD height;
		WORD width;
		DWORD size;
		DWORD dataSize; //Without header
		BYTE* entireMap;
		int xStart;
		int yStart;
		AreaData area;
	};



private:
	bool isMapSaved();
	bool fileExists(const char* file);
	bool constructMapFromClient();
	bool addMapPiece(MapPiece* piece);
	void printToFile(const char* name);

	//Adds objects to the attr map file (impossible to know objects sizes)
	void addObjectsCollisions();

	inline void setByte(BYTE b, int x, int y) { map[y * maxX + x] = b; };

	//Adds one extra blocked area to the edges of the map
	void increaseBlockedArea();

	bool loadMapFromDisk();
	void saveMap();


	std::string mapName;
	int maxX;
	int maxY;


	BYTE* map;
	std::vector<bool>* anyAngleMap;

	std::vector<AreaData::Object> objects;

	AnyAngleAlgorithm* aPathPlaning;
	JPS::Searcher<MapCollision>* pathFinding;
};
