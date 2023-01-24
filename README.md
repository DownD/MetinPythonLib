# MetinPythonLib

Before everything i must emphasize, <br>
I created this project JUST FOR LEARNING PURPOSES, I never used this either to gain an unfair advantage or to profit from it!<br>
For this reason, the patterns won't be available.<br>
This is an old project and I was learning while developing, thus some code parts are a bit messy and is missing function documentation. And currently my focus is on other projects so that's something i am not gonna change anytime soon.<br>
This project is not supported anymore.<br>

## Motivation
This is probably my biggest project yet, it took more then 2 years of my free time to reach the current state and much more of learning.
But the knowledge that i gathered from this project was like no other, thanks to this i got to learn a lot mainly on the following areas:
- Reverse Engineering (Static analysis, dinamyc analysis)
- C++/Python/ASM programming (and Cython based modules)
- Software architecture
- Sockets communication
- Path-planning/path-finding algorithms
- REST API's
- Windows Operating System
- Process security and injection vulnerabilites
- OS Memory management

I created this just for the fun of learning, testing and understanding how a game works and can be exploited.

## Introduction
This project was made for [Metin2](https://gameforge.com/en-US/play/metin2), a MMORPG game from 2007.
It creates a DLL that will hook multiple game functions and creates Python bindings that can be used to create complex automation scripts with the game memory.
Since the game itself already has multiple bindings to Python, this library enhances it and allows to inject python scripts into the game memory.

With this library is possible to:
- Gather information from players or instances arround
- Find a path between points with state of the art algorithms ([ANYA](https://github.com/DownD/AnyAnglePathPlaning))
- Send attacks
- Move the main player
- Walk trough walls
- Make requests to HTTP server
- Create Websockets
- Pickup items with filters
- And much more...

All from a python script!
An example of the usage of this library can be found here [MetinPythonExtension](https://github.com/DownD/MetinPythonExtension).

In addition to this, it's possible to load patterns from a remote server and not include them inside the produced DLL, this was a usecase that i thought in the begging about update configuration remotely and uploading data from multiple clients into a central server, but it never took off completly.

### Structure
- MetinPythonLib -> The main DLL
- PatternScanner -> Project that attempts to find the offsets needed by running a memory scan for specific patterns
- PacketSniffer -> Will hook specific network functions and dump every unencrypted communication to specific files.

#### Remarks
The PacketSniffer was the last addition that i made and is the only project that doesn't make use of the ```common``` folder, because of compiler related issues, just for sake of simplicity I just copy the files directly into the project.

## Python Exports
- Module net_packet
  - Get(\<string\> filePath) returns \<bytearray\><br>
    Similar to old app.Get, allows to extract any file encrypted.
    
  - IsPositionBlocked(\<int\>x,\<int\>y) returns \<boolean\><br>
    Allows to check if a map position is walkable(mobs don't count), true if is walkable or false if is not walkable.<br>
    Note: For better pathfinding, unblocked points that are close(1 unit) to a blocked point, are considered blocked too.<br>
    There is a bug with objects, since i can't figure out how to load the objects.
    
  - FindPath(\<int\>x_start,\<int\>y_start,\<int\>x_end,\<int\>y_end) returns \<tuple\>(x,y)<br>
    Finds a path between 2 points. <br>
    The path will not contain the current point.<br>
    It's possible to edit the maps, by changing the files in Resources/Maps, 0 represents a blocked location and the 1 represents a walkable position. The module will generate a new map if the same does not exist.<br>
    
  - SendPacket(\<int\>size,\<bytearray\>buffer) return None<br>
    Sends a packet to the server bypassing any encryption set.<br>
    
  - SendAttackPacket(\<int\>vid,\<byte\>type)<br>
    Sends an attack packet to the server, the type is usually 0.<br>
    
  - SendStatePacket(\<float\>x,\<float\>y,\<float\>rotation_angle,\<byte\>eFunc, \<byte\>uArgs)<br>
    Sends a packet containing the current state of the main player, can be used to change position,rotation and attack state on server side only.
    The value eFunc can take the fallowing values: CHAR_STATE_ATTACK,CHAR_STATE_STOP,CHAR_STATE_WALK<br>
    If eFunc == CHAR_STATE_ATTACK then the uArgs value can take one of the following values:
  - CHAR_STATE_ARG_HORSE_ATTACK1
	- CHAR_STATE_ARG_HORSE_ATTACK2
	- CHAR_STATE_ARG_HORSE_ATTACK3
	- CHAR_STATE_ARG_COMBO_ATTACK1
	- CHAR_STATE_ARG_COMBO_ATTACK2
	- CHAR_STATE_ARG_COMBO_ATTACK3
	- CHAR_STATE_ARG_COMBO_ATTACK4<br>
   otherwise the value can be NULL or CHAR_STATE_ARG_NONE<br>
   All this constants are defined in the module.<br>
       
  - \<dict\>InstancesList<br>
    -> Is a dictionary containing all vids currently in sight as keys and values<br>
    Note: Use the keys as vids, the values may be changed on a new version<br>

  - IsDead(\<int\>vid) returns 1 or 0<br>
    Returns 1 if the instance with the vid provided is dead or doesn't exist and returns 0 otherwise<br>

  - SendStartFishing(\<word\>direction)<br>
    Sends a packet to the server to start fishing, the direction parameter is a word only understandable by the server(will be converted in the future).
    
  - SendStopFishing(\<byte\>type,\<float\>timeLeft)<br>
    Sends a packet to the server to stop fishing, the type can be any of the following parameters:<br>
    - SUCCESS_FISHING -> The mini game was solve successfully.
    - UNSUCCESS_FISHING -> The mini game was not solve.<br>
    The timeLeft represents the time left to fish.<br>

  - SendAddFlyTarget(\<int\>vid,\<float\>x,\<float\>y)<br>
    Sends a packet to send an arrow at an enemy.<br>
 
  - SendShoot(\<byte\>uSkill)<br>
    Sends an attack packet to the current selected enemy (should be used after SendAddFlyTarget).<br>
    uSkill can be:<br>
    - COMBO_SKILL_ARCH -> Normal attack.
    
  - BlockFishingPackets()<br>
    Blocks client from sending fishing packets (this module will still be able to send)<br>
    
  - UnblockFishingPackets()<br>
    Unblocks client from sending fishing packets.<br>
    
  - DisableCollisions()<br>
    Disable client colisions with objects and the terrain (Wallhack).<br>
    
  - EnableCollisions()<br>
    Enable client colisions.<br>
    
  - RegisterNewShopCallback(\<callable_function\>callback)<br>
    Sets a callback function, that will be called whenever a new private shop is created arround.<br>
    That callback will be called with the shop vid as the first argument.<br>
	
  - RegisterDigMotionCallback(\<callable_function\>callback)<br>
    Sets a callback function, that will be called whenever a dig motion(mining packet) is called.<br>
	  The callback function will be called with the following arguments (player_vid,ore_vid,count)<br>
	
  - \<string\>PATH<br>
    Path of the location where the library was injected<br>

  - GetCloseItemGround(\<int\>x,\<int\>y) returns a tupple (\<int\>vid,\<int\>x,\<int\>y)<br>
    Returns the closest pickable item in the ground relative to the position given.<br>
    The items will be fitler acording to the pickup filter (see below).<br>
    Also, it will ignore items owned by other player.

  - SendPickupItem(\<int\>itemVID)<br>
    Sends a packet to pickup an item from the ground<br>

  - SetMoveSpeedMultiplier(\<float\> speed)<br>
    Set the character movement boosting movement speed. 

  - SendUseSkillPacket(\<int\> skillIndex, \<int\> vid)<br>
    Uses a skill by providing a skillIndex and a target vid without doing the animation.

  - SendUseSkillPacketBySlot(\<int\> skillSlotIndex, \<int\> vid)<br>
    The same as SendUseSkillPacket but the first argument is a slot index instead of the skill index<br>
    And also sets the cooldown of the skill on the client.<br>

  - IsPathBlocked(\<int\>x_start,\<int\>y_start,\<int\>x_end,\<int\>y_end)<br>
    Returns false if none of the points is blocked in a straight line other, if any of the points is blocked returns true.<br>
    
  - BlockAttackPackets()<br>
    Blocks all attack packets sent from the client, the only attack packets allowed are the ones sent by this module.<br>

  - UnblockAttackPackets()<br>
    Unblocks all attack packets.<br>

  - SkipRenderer()<br>
    Instruct the client to skip the drawing process, this can save CPU.<br>

  - UnskipRenderer()<br>
    Instruct the client to start drawing again.<br>

  - SyncPlayerPosition(\<list\>victims)<br>
    This is part of an exploit that allows to teleport other players.<br>
    The argument victims is a list of lists, each row containing vid victim, x coordinates and y coordinates.<br>
    As far as analyzed, for this to work, a special state packet need to be sent with the following arguments ```net_packet.SendStatePacket(mx,my,0,3,17)```
    where mx and my are the mob coordinates.<br>

  - SetRecvChatCallback(\<function\>callbackFunction)<br>
    Sets a callback function that will be called with the following arguments: ```int vid,int type,int empire,string msg,string locale```.<br>
    This function will be called whenever the client recives a chat message or command from the server.
    vid is the vid that send the message or 0 if is a command message.
    type is the type of message and can take the following values ``` CHAT_TYPE_TALKING,CHAT_TYPE_INFO,CHAT_TYPE_NOTICE,CHAT_TYPE_PARTY,CHAT_TYPE_GUILD,CHAT_TYPE_COMMAND,CHAT_TYPE_SHOUT,CHAT_TYPE_WHISPER``` that can be found on the chat module. 
    msg is the message content.
    empire is the empire of the player.
    locale is the region from the player that send the message or empty if was not a player.



### Remote Communication
Communication with the outside world.
All functions are asynchronous.

 - GetRequest(\<string\> url,\<callable_function\>callback) returns \<int\><br>
   Sends a async GET request to the specified url.
   The callback is called when the response arrives from the server, it will be called with 2 arguments, the ID of the request and a string message respectively.
   If the request is successful it returns an ID of the request otherwise returns -1.


 - OpenWebsocket(\<string\> url,\<callable_function\>callback) returns \<int\><br>
   Opens a websocket to the specified url.
   The callback is responsible for handling the receive messages. It will be called every time a message is received, with 2 arguments, the ID of the socket and a string containing the message respectively .
   If the request is schedule successfully it returns an ID of the socket otherwise returns -1.
   
 - SendWebsocket(\<int\> id,\<string\>message) returns \<int\><br>
   Sends a message to the socket with the specified id.
   If the message is schedule successfully it returns 1 otherwise returns 0.

 - CloseWebsocket(\<int\> id) returns \<int\><br>
   Closes a socket with the specified id.
   If the message is schedule successfully it returns 1 otherwise returns 0.

### Pickup Filter
A filter o be applied when calling GetCloseItemGround, by default the filter is set to pick items not present in filter.

  - ItemGrndDelFilter(\<int\> index)<br>
    Deletes an item id from the filter.

  - ItemGrndAddFilter(\<int\> index)<br>
    Adds an item id to the filter.

  - ItemGrndOnFilter()<br>
    Changes the filter mode, to only return items in the filter.

  - ItemGrndNotOnFilter()<br>
    Changes the filter mode, to ignore all items present in the filter.

  - ItemGrndFilterClear()<br>
    Deletes every item in the filter.

  - GetItemGrndID(\<int\> VID)<br>
    Return the ID of an item in the ground with the specified VID<br> 

  - ItemGrndSelectRange(\<float\> range)<br>
    Set's the maximum range to pick items<br> 

  - ItemGrndItemFirst()<br>
    Makes the function GetCloseItemGround return the closest item if there is no item returns the closest yang<br> 

  - ItemGrndNoItemFirst()<br>
    Makes the function GetCloseItemGround return the closest item or yang closest<br>

  - ItemGrndInBlockedPath()<br>
    Ignore items items that are in a blocked path<br> 

  - ItemGrndNotInBlockedPath()<br>
    Allow to return items that are in a blocked path<br> 

  - SetRecvAddGrndItemCallback(\<function\>callbackFunction)<br>
    Sets a callback function that will be called with the following arguments: ```int vid, int itemIndex, long x, long y, string owner```.<br>
    This function will be called whenever the server sends a new ground item.

  - SetRecvChangeOwnershipGrndItemCallback(\<function\>callbackFunction)<br>
    Sets a callback function that will be called with the following arguments: ```int vid, string owner```.<br>
    This function will be called whenever the server sends a change in a ground item ownership (every item that is dropped with an owner will also recive this packet after the append)
    If the owner argument is an empty string then the item doesn't have an owner.

  - SetRecvDelGrndItemCallback(\<function\>callbackFunction)<br>
    Sets a callback function that will be called with the following arguments: ```int vid```.<br>
    This function will be called whenever the server sends the command to delete a current item on the ground (this includes, the items being out of range and the item disappearement)
    WARNING: If you are trying are creating a list with items on the ground it might be needed to manually clear all items on phase change (Not tested)

### Simulation of old functions
These simulates the functions that were removed in recent versions of the game.

  - GetPixelPosition(\<int\>vid) returns a tupple (x,y,z)<br>
    Returns the position of the player by vid

  - MoveToDestPosition(\<float\> x,\<float\> y)<br>
    Moves to a destination.
	
  - SetMoveSpeed(\<float\> speed)<br>
    Set the character movement speed. The client might change the speed again need to be called constantly for now.
    If speed bigger then 2.0, the client will disconnect.


### This are relative to a Packet Filter for debug purposes
By default every packet will be shown.

  - LaunchPacketFilter()<br>
    Launches a console to print the packets.
  
  - ClosePacketFilter()<br>
    Closes the console from packet filter.
  
  - StartPacketFilter()<br>
    Start filtering packets.

  - StopPacketFilter()<br>
    Stop filtering packets.

  - SkipInHeader(\<int\>packet header)<br>
    Skips a packet coming from the server.

  - SkipOutHeader(\<int\>packet header)<br>
    Skips a packet going to the server.
  
  - DoNotSkipInHeader(\<int\>packet header)<br>
    Removes a packet coming from the server from the skipped packets.

  - DoNotSkipInHeader(\<int\>packet header)<br>
    Removes a packet going to the server from the skipped packets.

  - ClearOutput()<br>
    Clear what's in the console.
  
  - ClearInFilter()<br>
    Clear all headers from the filter coming from the server.

  - ClearOutFilter()<br>
    Clear all headers from the filter going to the server.

  - SetOutFilterMode(\<int\>mode)<br>
    Changes filter mode for outgoing packets, if set to 1, it will shows all packets that  correspond to the filter, if set to 0 it will show all packets that are not within the filter.

  - SetInFilterMode(\<int\>mode)<br>
    Changes filter mode for incoming packets, if set to 1, it will shows all packets that  correspond to the filter, if set to 0 it will show all packets that are not within the filter

## Compiling Notes

Python 2.7 (32 bits) needs to be installed in the system (C:/Python27) by default.

- Dependencies using vcpkg:
  cpprestsdk -> ```vcpkg install --recurse cpprestsdk[default-features,websockets]:x86-windows-static```<br>
  curl -> ```vcpkg install curl[core,openssl]:x86-windows-static```<br>
  jsoncpp -> ```vcpkg install jsoncpp:x86-windows-static```<br>
  websocketpp -> ```vcpkg install websocketpp:x86-windows-static```<br>
  boost -> ```vcpkg install boost:x86-windows-static```<br>

Also the project is using [SimpleIni](https://github.com/brofield/simpleini) to parse the .ini configuration file and [Date](https://github.com/HowardHinnant/date) to format date.


