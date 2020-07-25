# MetinPythonLib

## Things that need to be checked moving across servers:

- Packet Headers and structures
- Patterns of send, recvm GetEther and network class pointer
- Structure of CMappedFile
- All original modules on init.py, the script is resposible for making sure the following functions work properly:
-- Module app
--- Function OpenTextFile
--- Function IsExistFile


## Python Exports
- Module net_packet
  - Get(<string> filePath) returns <bytearray>
    -> Similar to old app.Get, allows to extract any file encrypted.
  - IsPositionBlocked(<int>x,<int>y) returns <boolean>
    -> Allows to check if a map position is walkable(mobs don't count), true if is walkable or false if is not walkable
    Note: For better pathfinding, unblocked points that are close(1 unit) to a blocked point, are considered blocked too
   - GetCurrentPhase() returns <int>
  -> Allows to get the current phase in integer format.
-- FindPath(<int>x_start,<int>y_start,<int>x_end,<int>y_end) returns <tuple>(x,y)
  -> Finds a path between 2 points using A* Jump Point Search(https://github.com/fgenesis/tinypile) solves any path tested in less then 200ms
-- SendPacket(<int>size,<bytearray>buffer) return None
  -> Sends a packet to the server bypassing any encryption set
-- <dict>InstancesList
  -> Is a dictionary containing all vids currently in sight as keys and values
-- <string>PATH
  -> Path of the location where the library was injected


