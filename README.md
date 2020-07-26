# MetinPythonLib V0.3

## Things that need to be checked moving across servers:

- Packet Headers and structures
- Patterns of send, recvm GetEther and network class pointer
- Structure of CMappedFile
- All original modules on init.py, the script is resposible for making sure the following functions work properly:
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
   All this constantes are defined in the module.<br>
       
  - \<dict\>InstancesList<br>
    -> Is a dictionary containing all vids currently in sight as keys and values<br>
    Note: Use the keys as vids, the values may be changed on a new version<br>
    
  - \<string\>PATH<br>
    -> Path of the location where the library was injected<br>


