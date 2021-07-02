# eXLib V1.1
## General Functions
- Get(\<string\> filePath) returns \<bytearray\><br>
  Similar to old app.Get, allows to extract any file encrypted.

- IsPositionBlocked(\<int\>x,\<int\>y) returns \<boolean\><br>
  Allows to check if a map position is walkable(mobs don't count), true if is walkable or false if is not walkable.<br>
  Note: For better pathfinding, unblocked points that are close(1 unit) to a blocked point, are considered blocked too.<br>
  There is a bug with objects, since i can't figure out how to load the objects.

- FindPath(\<int\>x_start,\<int\>y_start,\<int\>x_end,\<int\>y_end) returns \<tuple\>(x,y)<br>
  Finds a path between 2 points. <br>
  The path will not contain the current point.<br>
  It's possible to edit the maps, by changing the files in Resources/Maps, 0 represents a blocked location and the 1 represents a walkable position. The module will generate a new map if the same does not exist.br>

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
  Uses a skill with a target vid.
    
## Pickup Filter
A filter o be applied when calling GetCloseItemGround, by default the filter is set to pick items not present in filter.

  - ItemGrndDelFilter(\<int\> index)<br>
    Delets an item id from the filter.

  - ItemGrndAddFilter(\<int\> index)<br>
    Adds an item id to the filter.

  - ItemGrndOnFilter()<br>
    Changes the filter mode, to only return items in the filter.

  - ItemGrndNotOnFilter()<br>
    Changes the filter mode, to ignore all items present in the filter.

  - ItemGrndFilterClear()<br>
    Deletes every item in the filter.

## Simulation of old functions
These simulates the functions that were removed from the modules by Gameforge.

  - GetPixelPosition(\<int\>vid) returns a tupple (x,y,z)<br>
    Returns the position of the player by vid

  - MoveToDestPosition(\<float\> x,\<float\> y)<br>
    Moves to a destination.
	
  - SendUseSkillPacket(\<int\> skillIndex, \<int\> vid)<br>
    Uses a skill with a target vid.


# Updates
- v1.1:
    - Added SetMoveSpeedMultiplier
    - Added SendUseSkillPacket
    - Fixed some memory leaks
    - Fixed a bug where the pickup was picking wrong items
