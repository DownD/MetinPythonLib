#include "MapCollision.h"
#include <sstream>
#include <iomanip>
#include "App.h"
#include "Network.h"
#include <iostream>
#include <fstream>

#define ATTR_WIDTH 256
#define ATTR_HEIGHT 256
#define HEADER_SIZE 6


MapCollision* currMap = 0;

MapCollision::MapCollision(const char* map_name){
	maxX = 0;
	maxY = 0;
	mapName = std::string(map_name);
	bool val = constructMap();
#ifdef _DEBUG
	if(val)
		printf("Current map %s loaded with success! Max-X: %d  Max-Y: %d\n",map_name,maxX,maxY);
#endif
	if (!val) {
		MessageBoxA(NULL, "Error constructing map! Functions that need map information  will not work", "Error", MB_OK);
		return;
	}

	pathFinding = new JPS::Searcher<MapCollision>(*this);

	//printToFile("map_no_objects.txt");
	addObjectsCollisions();

	increaseBlockedArea();

	//printToFile("map_objects.txt");

}


MapCollision::~MapCollision()
{
	free(map);
	delete pathFinding;
}

inline bool MapCollision::isBlocked(int x, int y){
	if (x < 0 || y < 0 || x >= maxX || y >= maxY) {
		return true;
	}
	return map[y*maxX + x] & 0x1;
}

inline BYTE MapCollision::getByte(int x, int y)
{
	return map[y*maxX + x];
}

bool MapCollision::findPath(int x_start, int y_start, int x_end, int y_end, std::vector<Point>& path)
{
	JPS::PathVector pathBuf;
	bool found = pathFinding->findPath(pathBuf, JPS::Pos(x_start, y_start), JPS::Pos(x_end, y_end), 0);

#ifdef _DEBUG
	if(found)
		printf("Path found from (%d,%d) to (%d,%d) with %d points!\n", x_start, y_start, x_end, y_end,path.size());
	else
		printf("No Path from (%d,%d) to (%d,%d)!!!\n", x_start, y_start, x_end, y_end, path.size());
#endif
	if (found) {
		for (JPS::PathVector::iterator it = pathBuf.begin(); it != pathBuf.end(); ++it)
			path.push_back(Point(it->x, it->y));
	}
	return found;

}

inline unsigned MapCollision::operator()(unsigned x, unsigned y) const
{
	if (x < 0 || y < 0 || x >= maxX || y >= maxY) {
		return true;
	}
	return !(map[y*maxX + x] & 0x1);
}



bool MapCollision::fileExists(const char * file)
{
	PyObject * mod = PyImport_ImportModule("app");
	PyObject * poArgs = Py_BuildValue("(s)",file);
	long ret = 0;

	if(PyCallClassMemberFunc(mod, "IsExistFile", poArgs,&ret))
		return ret;
	Py_DECREF(mod);
	return false;
}

bool MapCollision::constructMap()
{
	std::vector<MapPiece*> buffer;
	std::string baseFolder = mapName;
	baseFolder += "\\";
	int counter = 0;
	int xPieces = 0;
	for (int x = 0; ; x++, xPieces++) {
		std::ostringstream x_folder;
		x_folder << std::setw(3) << std::setfill('0') << x;
		std::string firstPiece = baseFolder + x_folder.str();
		firstPiece += "000\\attr.atr";
		if (!fileExists(firstPiece.c_str())) {
			break;
		}
		for (int y = 0, yPieces = 0; ; y++) {
			std::ostringstream y_folder;
			y_folder << std::setw(3) << std::setfill('0') << y;

			std::string fullPath = baseFolder + x_folder.str() + y_folder.str() + "\\attr.atr";
			if (fileExists(fullPath.c_str())) {
				EterFile* f = CGetEter(fullPath.c_str());
				MapPiece *p = new MapPiece(f, x, y, baseFolder + x_folder.str() + y_folder.str());
				buffer.push_back(p);
				//p->printToFile((x_folder.str() + y_folder.str()).c_str());

			}
			else {
				break;
			}
		}
	}
	if (buffer.size() == 0) {
#ifdef _DEBUG
		printf("No map found with name %s\n", mapName.c_str());
#endif
		return false;
	}



	maxY = (buffer.size() / xPieces)*ATTR_HEIGHT;
	maxX = xPieces * ATTR_WIDTH;

	int allocSpace = ATTR_WIDTH * ATTR_HEIGHT*buffer.size();
	map = (BYTE*)malloc(allocSpace);

	for (auto mapPiece : buffer) {
		if (!addMapPiece(mapPiece)) {
			#ifdef _DEBUG
			printf("MapPiece exceeds limits %s\n", mapName.c_str());
			#endif
			return false;
		}
		for (auto & object : mapPiece->area.vec) {
			objects.push_back(object);
		}

		delete mapPiece;
	}
	return true;

}

bool MapCollision::addMapPiece(MapPiece * piece)
{
	if (piece->dataSize > ATTR_HEIGHT*ATTR_WIDTH) {
		return false;
	}
	for (int y = 0; y < ATTR_HEIGHT; y++) {
		void* startLoc = (void*)((int)map + (piece->yStart + y)*maxX + piece->xStart);
		void* pieceStart = (void*)((int)piece->getMapData() + y * ATTR_WIDTH);
		memmove(startLoc, pieceStart, ATTR_WIDTH);
	}

	return true;
}

void MapCollision::printToFile(const char* name)
{
	std::ofstream myfile;
	myfile.open(name);
	char* line = (char*)malloc(maxX);
	for (int y = 0; y < maxY; y++) {
		for (int x = 0; x < maxX; x++) {
			line[x] = 48 + (int)isBlocked(x, y);
		}
		myfile << line;
		myfile << "\n";
	}
	myfile.close();
	
}

void MapCollision::addObjectsCollisions()
{
	for (auto obj : objects) {
		setByte(1,obj.x / 100, abs(obj.y / 100));
	}
}

inline void MapCollision::setByte(BYTE  b, int x, int y)
{
	map[y*maxX + x] = b;
}

void MapCollision::increaseBlockedArea()
{
	std::list<Point> bufferPoints;
	for (int y = 0; y < maxY; y++) {
		for (int x = 0; x < maxX; x++) {
			if (!isBlocked(x,y)) {
				if (isBlocked(x-1, y-1)) {
					bufferPoints.push_back(Point(x, y));
					continue;
				}
				if (isBlocked(x-1, y)) {
					bufferPoints.push_back(Point(x, y));
					continue;
				}
				if (isBlocked(x-1, y+1)) {
					bufferPoints.push_back(Point(x, y));
					continue;
				}
				if (isBlocked(x + 1, y-1)) {
					bufferPoints.push_back(Point(x, y));
					continue;
				}
				if (isBlocked(x + 1, y)) {
					bufferPoints.push_back(Point(x, y));
					continue;
				}
				if (isBlocked(x + 1, y+1)) {
					bufferPoints.push_back(Point(x, y));
					continue;
				}
				if (isBlocked(x, y+1)) {
					bufferPoints.push_back(Point(x, y));
					continue;
				}
				if (isBlocked(x, y-1)) {
					bufferPoints.push_back(Point(x, y));
					continue;
				}
			}
		}
	}
	 
	for (Point &a : bufferPoints) {
		setByte(1, a.x, a.y);
	}




}


MapCollision::MapPiece::MapPiece(EterFile * file,int x,int y,std::string path)
{
	size = file->size;
	entireMap = (BYTE*)malloc(size);
	memcpy(entireMap, file->data, size);
	WORD* buf = (WORD*)entireMap;
	version = buf[0];
	width = buf[1];
	height = buf[2];
	dataSize = size - HEADER_SIZE;
	xStart = x*ATTR_WIDTH;
	yStart = y*ATTR_HEIGHT;

	std::string areaData = path + "\\areadata.txt";
	auto eterArea = CGetEter(areaData.c_str());
	area.loadFile((char*)eterArea->data,eterArea->size);

	//printf("version %d width= %d height =%d size=%d x=%d y=%d\n", version, width, height, size, xStart, yStart);

}

MapCollision::MapPiece::~MapPiece()
{
	free(entireMap);
}

BYTE* MapCollision::MapPiece::getMapData()
{
	return (BYTE*)((int)entireMap + HEADER_SIZE);
}

void MapCollision::MapPiece::printToFile(const char * name)
{
	std::ofstream myfile;
	myfile.open(name);
	BYTE* data = getMapData();
	char* line = (char*)calloc(ATTR_WIDTH,1);
	for (int y = 0; y < ATTR_HEIGHT; y++) {
		for (int x = 0; x < ATTR_WIDTH; x++) {
			line[x] = 48 + (data[y*ATTR_WIDTH + x] & 0x1);
		}
		myfile << line;
		myfile << "\n";
	}
	myfile << "X_Start: " << std::to_string(xStart) << "Y_Start: " << std::to_string(yStart);
	myfile.close();
}

bool setCurrentCollisionMap()
{
	std::string map_name;
	PyObject * mod = PyImport_ImportModule("background");
	PyObject * poArgs = Py_BuildValue("()");

	if (!PyCallClassMemberFunc(mod, "GetCurrentMapName", poArgs, map_name)) {
#ifdef _DEBUG
		printf("Error calling GetCurrenMap %s\n", map_name.c_str());
#endif
		Py_DECREF(mod);
		Py_DECREF(poArgs);
		return false;
	}

	//printf("Setting Map Collision %s\n", map_name.c_str());
	Py_DECREF(mod);
	Py_DECREF(poArgs);

	if (currMap) {
		if(map_name.compare(currMap->getMapName()) == 0)
			return true;
		delete currMap;
	}
	currMap = new MapCollision(map_name.c_str());
	return true;
}

MapCollision * getCurrentCollisionMap()
{
	return currMap;
}

void freeCurrentMap()
{
	if (currMap) {
		delete currMap;
	}
	currMap = 0;
}

bool isBlockedPosition(int x, int y)
{
	if (currMap) {
		return currMap->isBlocked(x, y);
	}

	if (getCurrentPhase() == PHASE_GAME) {
		setCurrentCollisionMap();
		if (currMap == 0) {
			return true;
		}
		else {
			return currMap->isBlocked(x, y);
		}
	}
	else {
		return true;
	}
}

BYTE getAttrByte(int x, int y)
{
	if (currMap)
		return currMap->getByte(x, y);
}

bool findPath(int x_start, int y_start, int x_end, int y_end, std::vector<Point>& path)
{
	if (currMap) {
		return currMap->findPath(x_start, y_start, x_end, y_end, path);
	}

	if (getCurrentPhase() == PHASE_GAME) {
		setCurrentCollisionMap();
		if (currMap == 0) {
			return false;
		}
		else {
			return currMap->findPath(x_start, y_start, x_end, y_end, path);
		}
	}
	else {
		return false;
	}
}

void AreaData::loadFile(char * buffer, int size)
{
	std::stringstream membuf(std::ios::in | std::ios::out | std::ios::binary);
	membuf.write(buffer, size);
	membuf.seekg(std::ios::beg);
	std::string line;
	std::string startObjectsPrefix = "Start Object";
	while (std::getline(membuf, line)) {
		if (line.find(startObjectsPrefix) != std::string::npos) {
			Object obj;
			//Coords
			std::getline(membuf, line);
			const char* coordsText = line.c_str();
			sscanf(coordsText, "%f %f %f", &obj.x, &obj.y, &obj.z);

			//crc
			std::getline(membuf, line);
			/*coordsText = line.c_str();
			sscanf(coordsText, "%d", &obj.crc);*/

			//Rotation
			std::getline(membuf, line);
			coordsText = line.c_str();
			sscanf(coordsText, "%f#%f#%f", &obj.rotX, &obj.rotY, &obj.rotZ);

			//Uknown
			std::getline(membuf, line);
			coordsText = line.c_str();
			sscanf(coordsText, "%d", &obj.uknown);

			vec.push_back(obj);
		}

	}
}
