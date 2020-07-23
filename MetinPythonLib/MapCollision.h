#pragma once
#include "PythonModule.h"
#include "JPS.h"

class MapCollision
{
public:
	MapCollision(const char* map_name);
	~MapCollision();


	inline bool isBlocked(int x, int y);
	inline BYTE getByte(int x, int y);

	const char* getMapName() { return mapName.c_str(); }
	bool findPath(int x_start, int y_start, int x_end, int y_end, std::vector<Point>& path);

	inline unsigned operator()(unsigned x, unsigned y) const;
private:

	struct MapPiece{
		MapPiece(EterFile* file, int x, int y);
		~MapPiece();

		BYTE* getMapData();
		void printToFile(const char* name);

		WORD version;
		WORD height;
		WORD width;
		DWORD size;
		DWORD dataSize; //Without header
		BYTE* entireMap;
		int xStart;
		int yStart;
	};



private:
	bool fileExists(const char* file);
	bool constructMap();
	bool addMapPiece(MapPiece* piece);
	void printToFile(const char* name);

	inline void setByte(BYTE b, int x, int y);

	//Adds one extra blocked area to the edges of the map
	void increaseBlockedArea();


	std::string mapName;
	int maxX;
	int maxY;
	BYTE* map;
	JPS::Searcher<MapCollision>* pathFinding;
};


//REAL COORDINATES FROM PYTHON FUNCTIONS NEEDS TO BE DIVIDED BY 100
//BEFORE PASSING TO THE NEXT FUNCTIONS
bool setCurrentCollisionMap();
MapCollision* getCurrentCollisionMap();
void freeCurrentMap();

bool isBlockedPosition(int x, int y);
BYTE getAttrByte(int x, int y);
bool findPath(int x_start, int y_start, int x_end, int y_end, std::vector<Point>& path);