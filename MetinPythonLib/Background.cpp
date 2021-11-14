#include "stdafx.h"
#include "Background.h"
#include "NetworkStream.h"

CBackground::CBackground()
{
	background_mod = 0;
	currMap = 0;
}

CBackground::~CBackground()
{
	freeCurrentMap();
	Py_DECREF(background_mod);
}

void CBackground::importPython()
{
	background_mod = PyImport_ImportModule("background");
	if (!background_mod) {
		DEBUG_INFO_LEVEL_1("Error importing background python module");
	}
}

bool CBackground::setCurrentCollisionMap()
{
	std::string map_name;
	PyObject* poArgs = Py_BuildValue("()");

	if (!PyCallClassMemberFunc(background_mod, "GetCurrentMapName", poArgs, map_name)) {
#ifdef _DEBUG
		DEBUG_INFO_LEVEL_1("Error calling GetCurrentMap %s\n", map_name.c_str());
#endif
		Py_XDECREF(poArgs);
		return false;
	}
	DEBUG_INFO_LEVEL_2("Setting collision map name=%s",map_name.c_str());
	//printf("Setting Map Collision %s\n", map_name.c_str());
	Py_XDECREF(poArgs);
	if (currMap) {
		if (map_name.compare(currMap->getMapName()) == 0)
			return true;
		delete currMap;
		currMap = 0;
	}
	currMap = new MapCollision(map_name.c_str());
	return true;
}

void CBackground::freeCurrentMap()
{
	if (currMap) {
		delete currMap;
	}
	currMap = 0;
}

bool CBackground::isBlockedPosition(int x, int y)
{
	if (currMap) {
		return currMap->isBlocked(x, y);
	}

	CNetworkStream& net = CNetworkStream::Instance();
	if (net.GetCurrentPhase() == PHASE_GAME) {
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

BYTE CBackground::getAttrByte(int x, int y)
{
	if (currMap)
		return currMap->getByte(x, y);
	return 0;
}

bool CBackground::findPath(int x_start, int y_start, int x_end, int y_end, std::vector<Point>& path)
{
	if (currMap) {
		return currMap->findPath(x_start, y_start, x_end, y_end, path);
	}

	CNetworkStream& net = CNetworkStream::Instance();
	if (net.GetCurrentPhase() == PHASE_GAME) {
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

bool CBackground::isPathBlocked(int x_start, int y_start, int x_end, int y_end)
{
	x_start /= 100;
	y_start /= 100;
	x_end /= 100;
	y_end /= 100;
	DEBUG_INFO_LEVEL_4("x_start: %d, y_start: %d | x_end: %d, y_end: %d", x_start, y_start, x_end, y_end);
	CNetworkStream& net = CNetworkStream::Instance();
	if (net.GetCurrentPhase() == PHASE_GAME) {
		if (currMap == 0) {
			setCurrentCollisionMap();
		}
		if(currMap) {

			//swap points if start is bigger then end
			if (x_start > x_end) {
				int tmpx = x_start;
				int tmpy = y_start;
				x_start = x_end;
				y_start = y_end;
				x_end = tmpx;
				y_end = tmpy;
			}

			//calc position blocked
			float m = 0;
			float b = 0;
			if (abs(x_start - x_end)>0) {
				m = (y_end - y_start) / (x_end - x_start);
				b = y_start - m * x_start;
				for (int ix = x_start; ix <= x_end; ix++) {
					int y = m * ix + b;
					if (currMap->isBlocked(ix, y)) {
						return true;
					}

				}
			}
			//Slope undifined
			else {
				m = 1;
#ifdef _DEBUG_FILE
				int y_min = min(y_end, y_start);
				int y_max = max(y_end, y_start);
#else
				int y_min = std::min(y_end, y_start);
				int y_max = std::max(y_end, y_start);
#endif
				for (int iy = y_min; iy <= y_max; iy++) {
					if (currMap->isBlocked(x_start, iy)) {
						return true;
					}

				}
			}

			return false;
			
		}
	}
	return true;
}

bool CBackground::getClosestUnblocked(int x_start, int y_start, Point* buffer)
{
	Point closestPoints[8];

	//Initalize all coords
	for (int i = 0; i < 8; i++) {
		closestPoints[i] = { x_start,y_start };
	}

	while (true) {
		closestPoints[0].x++;

		closestPoints[1].x++;
		closestPoints[1].y++;

		closestPoints[2].x++;
		closestPoints[2].y--;

		closestPoints[3].x--;
		closestPoints[3].y++;

		closestPoints[4].x--;

		closestPoints[5].x--;
		closestPoints[5].y--;

		closestPoints[6].y++;

		closestPoints[7].y--;

		if (closestPoints[3].x-- <= 0) {
			return false;
		}

		for (int i = 0; i < 8; i++) {
			if (!isBlockedPosition(closestPoints[i].x, closestPoints[i].y)) {
				*buffer = closestPoints[i];
				return true;
			}
		}
	}
	return false;
}
