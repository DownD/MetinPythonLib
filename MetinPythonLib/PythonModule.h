#pragma once
#include "stdafx.h"
#include <stdio.h>
#include <string>
#include "shlwapi.h"
#include "PythonUtils.h"


//SET OLD FUNCTION
PyObject* GetPixelPosition(PyObject* poSelf, PyObject* poArgs);
PyObject* moveToDestPosition(PyObject* poSelf, PyObject* poArgs);
PyObject* pySetMoveSpeed(PyObject* poSelf, PyObject* poArgs);


PyObject* GetEterPacket(PyObject* poSelf, PyObject* poArgs);
PyObject* IsPositionBlocked(PyObject* poSelf, PyObject* poArgs);
PyObject* pyIsPathBlocked(PyObject* poSelf, PyObject* poArgs);
//PyObject* GetCurrentPhase(PyObject * poSelf, PyObject * poArgs);
PyObject* GetAttrByte(PyObject* poSelf, PyObject* poArgs); //Debug purposes
PyObject* pySendAttackPacket(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendStatePacket(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendPacket(PyObject* poSelf, PyObject* poArgs);
PyObject* pyIsDead(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendStartFishing(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendStopFishing(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendAddFlyTarget(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendShoot(PyObject* poSelf, PyObject* poArgs);
PyObject* pyBlockFishingPackets(PyObject* poSelf, PyObject* poArgs);
PyObject* pyUnblockFishingPackets(PyObject* poSelf, PyObject* poArgs);
PyObject* pyDisableCollisions(PyObject* poSelf, PyObject* poArgs);
PyObject* pyEnableCollisions(PyObject* poSelf, PyObject* poArgs);
PyObject* pyRegisterNewShopCallback(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendUseSkillPacket(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendUseSkillPacketBySlot(PyObject* poSelf, PyObject* poArgs);
PyObject* pyRecvDigMotionCallback(PyObject* poSelf, PyObject* poArgs);
PyObject* pyRecvStartFishCallback(PyObject* poSelf, PyObject* poArgs);

PyObject* pyBlockAttackPackets(PyObject* poSelf, PyObject* poArgs);
PyObject* pyUnblockAttackPackets(PyObject* poSelf, PyObject* poArgs);

PyObject* pyItemGrndFilterClear(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndNotOnFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndOnFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndAddFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndItemFirst(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndNoItemFirst(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndDelFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndSelectRange(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndIgnoreBlockedPath(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndNoIgnoreBlockedPath(PyObject* poSelf, PyObject* poArgs);
PyObject* pyGetCloseItemGround(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendPickupItem(PyObject* poSelf, PyObject* poArgs);
PyObject* pyGetItemGrndID(PyObject* poSelf, PyObject* poArgs);

PyObject* pySkipRenderer(PyObject* poSelf, PyObject* poArgs);
PyObject* pyUnSkipRenderer(PyObject* poSelf, PyObject* poArgs);
PyObject* pySyncPlayerPosition(PyObject* poSelf, PyObject* poArgs);
PyObject* pySetRecvChatCallback(PyObject* poSelf, PyObject* poArgs);


//Python callbacks
PyObject* pySetRecvAddGrndItem(PyObject* poSelf, PyObject* poArgs);
PyObject* pySetRecvChangeOwnershipGrndItem(PyObject* poSelf, PyObject* poArgs);
PyObject* pySetRecvDelGrndItem(PyObject* poSelf, PyObject* poArgs);



//PyObject* pySetKeyState(PyObject* poSelf, PyObject* poArgs); //There is a similar function, OnKeyUp or OnKeyDown


//PACKET FILTER
PyObject* launchPacketFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* closePacketFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* startPacketFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* stopPacketFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* skipInHeader(PyObject* poSelf, PyObject* poArgs);
PyObject* skipOutHeader(PyObject* poSelf, PyObject* poArgs);
PyObject* doNotSkipInHeader(PyObject* poSelf, PyObject* poArgs);
PyObject* doNotSkipOutHeader(PyObject* poSelf, PyObject* poArgs);
PyObject* clearOutput(PyObject* poSelf, PyObject* poArgs);
PyObject* clearInFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* clearOutFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* setInFilterMode(PyObject* poSelf, PyObject* poArgs);
PyObject* setOutFilterMode(PyObject* poSelf, PyObject* poArgs);

//NETOWORKING RELATED
PyObject* pyGetRequest(PyObject* poSelf, PyObject* poArgs);
PyObject* pyOpenWebsocket(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendWebsocket(PyObject* poSelf, PyObject* poArgs);
PyObject* pyCloseWebsocket(PyObject* poSelf, PyObject* poArgs);

bool addPathToInterpreter(const char* path);
bool executePythonFile(const char* file);
void initModule();