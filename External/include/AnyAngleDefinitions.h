/*
This code was developed by Tansel Uras (turas@usc.edu) at USC.
The code is hosted at 'http://idm-lab.org/anyangle'.
If you use this code in your research, please  cite our SoCS paper:

T. Uras and S. Koenig,  2015. An Empirical Comparison of Any-Angle Path-Planning Algorithms. In: Proceedings of the 8th Annual Symposium on Combinatorial
Search. Code available at: http://idm-lab.org/anyangle

Bibtex:
@inproceedings{uras:15,
  author = "T. Uras and S. Koenig",
  title = "An Empirical Comparison of Any-Angle Path-Planning Algorithms",
  booktitle = {Proceedings of the 8th Annual Symposium on Combinatorial Search},
  year = "2015",
  note = "Code available at: http://idm-lab.org/anyangle",
}
*/

#define NO_HOG

#ifndef ANY_ANGLE_DEFINITIONS_H
#define ANY_ANGLE_DEFINITIONS_H

#ifndef NO_HOG  // Use '-DNO_HOG' in the makefile to compile this outside the HOG platform.
#define ANY_ANGLE_RUNNING_IN_HOG    // Otherwise, the program assumes that it is running in HOG, which enables a specific constructor and visualization for HOG.
#include "Map2DEnvironment.h"
#include "FPUtil.h"
#endif

#ifdef _WIN32

#include <Windows.h>
#include <limits>
#endif

//#define ANY_ANGLE_STATISTICS		// Keep statistics and write them in a file

#ifdef ANY_ANGLE_RUNNING_IN_HOG
//#define ANY_ANGLE_ASSERTIONS		// Not necessary, just for debugging
#endif

// Some definitions that are used by the base class or some/all of the derived classes.

// Movement costs.
typedef double cost;
#ifdef _WIN32
#include <float.h>
#define INFINITE_COST DBL_MAX
#else
#define INFINITE_COST std::numeric_limits<cost>::max()
#endif
#define CARD_COST 1
#define DIAG_COST 1.414213
#define DIAG_DIFF 0.414213

#define MAX_SEARCH 50000	// After this many searches, reset the generated values.

// Location of a state in search.
#define NO_LIST 0
#define OPEN_LIST 1
#define CLOSED_LIST 2

// Directions
#define DIR_N 0
#define DIR_NE 1
#define DIR_E 2
#define DIR_SE 3
#define DIR_S 4
#define DIR_SW 5
#define DIR_W 6
#define DIR_NW 7

// If the difference between two floats is no more than this value, we treat them as the same value.
#define EPSILON 0.00001

// Corners and all search related information about corners are kept in an array. Specific corners are found in the array by their cornerIds.
typedef uint32_t cornerId;

// Some search methods do not use corners as states (such as ANYA), in which case, they might need to create an array that stores states.
typedef uint64_t stateId;

//#ifndef ANY_ANGLE_RUNNING_IN_HOG
// Redefine xyLoc as in HOG
struct xyLoc {
public:
    xyLoc() {}
    xyLoc(uint16_t _x, uint16_t _y) :x(_x), y(_y) {}
    uint16_t x;
    uint16_t y;
};

static bool operator==(const xyLoc &l1, const xyLoc &l2) {
    return (l1.x == l2.x) && (l1.y == l2.y);
}

//#endif

// Different from xyLoc, xyLocCont allows the x and y coordinates to be any float value.
struct xyLocCont {
    float x;
    float y;

    xyLocCont(float _x = 0, float _y = 0) :
        x(_x), y(_y)
    {
    }
    xyLocCont(xyLoc l) :
        x(l.x), y(l.y)
    {
    }

    // Rounds down the x and y values to return an xyLoc. Should only be used if it is known that the continuous values correspond to integers.
    xyLoc getCorner()
    {
        return xyLoc((int) (x + EPSILON), (int) (y + EPSILON));
    }

    // Returns true if the respective x and y coordinates of two xyLocCont variables differ by at most EPSILON.
    bool operator==(const xyLocCont& lhs) const
    {
        return (x <= lhs.x + EPSILON && x + EPSILON >= lhs.x) && (y <= lhs.y + EPSILON && y + EPSILON >= lhs.y);
    }

    // Returns true if the point lies on a column of the grid (that is, if x is an integer).
    //	bool OnVerticalLine(){float d = x - (int)(x + EPSILON); return (d < EPSILON) && (d > -EPSILON);}
    // TODO: fabs?

    // Returns true if the point lies on a row of the grid (that is, if y is an integer).
    //	bool OnHorizontalLine(){float d = y - (int)(y + EPSILON); return (d < EPSILON) && (d > -EPSILON);}
};

#endif
