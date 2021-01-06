# MetinPythonLib V0.3.1

Adds some functions to the python API, and try to inject a script.py from the current directory. 

## Things that need to be checked before moving across servers:

- Packet Headers and structures
- Patterns of send, recv, GetEther, SendAttackPacket, SendStatePacket and network class pointer
- Structure of CMappedFile
- The executed script is resposible for making sure the following functions work properly:
  - Module app
    - Function OpenTextFile
    - Function IsExistFile


## Python Exports
- Module net_packet
  - Get(\<string\> filePath) returns \<bytearray\><br>
    Similar to old app.Get, allows to extract any file encrypted.
    
  - IsPositionBlocked(\<int\>x,\<int\>y) returns \<boolean\><br>
    Allows to check if a map position is walkable(mobs don't count), true if is walkable or false if is not walkable.<br>
    Note: For better pathfinding, unblocked points that are close(1 unit) to a blocked point, are considered blocked too.<br>
    There is a bug with objects, since i can't figure out how to load the objects.
    
  - GetCurrentPhase() returns \<int\><br>
    Allows to get the current phase in integer format.<br>
    
  - FindPath(\<int\>x_start,\<int\>y_start,\<int\>x_end,\<int\>y_end) returns \<tuple\>(x,y)<br>
    Finds a path between 2 points using A* Jump Point Search(https://github.com/fgenesis/tinypile) solves any path tested in less then 200ms.<br>
    
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

  - \<string\>PATH<br>
    Path of the location where the library was injected<br>
  
  - BlockFishingPackets()<br>
    Blocks client from sending fishing packets (this module will still be able to send)
    
  - UnblockFishingPackets()<br>
    Unblocks client from sending fishing packets.
    

### Simulation of old functions
These simulates the functions that were removed from the modules by Gameforge.

  - GetPixelPosition(\<int\>vid) returns a tupple (x,y,z)<br>
    Returns the position of the player by vid

  - MoveToDestPosition(\<float\> x,\<float\> y)<br>
    Moves to a destination.


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


## Compiler Notes

Python 2.7 (32 biits) needs to be installed in the system (C:/Python27) by default.
