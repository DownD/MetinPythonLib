#pragma once
#include "stdafx.h"
#include "Singleton.h"
#include "../common/utils.h"
#include "defines.h"
#include "PythonUtils.h"
class CPlayer : public CSingleton<CPlayer>
{
public:
	CPlayer();
	~CPlayer();

	void importPython();


	//Hooks callbacks
	void __GetEter(CMappedFile& file, const char* fileName, void** buffer);
	bool __MoveToDestPosition(ClassPointer p, fPoint& pos);
	bool __MoveToDirection(ClassPointer p,float rot);
	bool __BackgroundCheckAdvanced(ClassPointer classPointer, void* instanceBase); //wallhack buildings
	bool __InstanceBaseCheckAdvanced(ClassPointer classPointer);//wallhack envoirnment_monsters 


	PyObject* GetEterPacket(PyObject* poSelf, PyObject* poArgs);
	EterFile* CGetEter(const char* name);
	bool  moveToDestPosition(DWORD vid, fPoint& pos);
	void setPixelPosition(fPoint fPos);

	BYTE getLastMovementType();
	fPoint getLastDestPosition();

	std::string getPlayerName();

	//Wallhack
	void inline SetBuildingWallHack(bool val) { wallHackBuildings = val; }; //1 turn on wallhack
	void inline SetMonsterTerrainWallHack(bool val) { wallHackTerrainMonsters = val; };  //1 turn on wallhack

private:

private:

	bool getTrigger;
	EterFile eterFile;

	//If sets this variable according to the last type of movement
	BYTE lastMovement;
	fPoint lastDestPos;

	//Wallhack
	bool wallHackBuildings;
	bool wallHackTerrainMonsters;

	PyObject* chr_mod;
	PyObject* player_mod;
};

