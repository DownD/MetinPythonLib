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

#ifndef ANY_ANGLE_ALGORITHM_H
#define ANY_ANGLE_ALGORITHM_H

#include <deque>
#include <queue>
#include <vector>
//#include <ext/hash_map>
#include "Timer.h"
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <limits>
#include <math.h>


#include "AnyAngleDefinitions.h"

#ifdef ANY_ANGLE_STATISTICS
#include "AnyAngleStatistics.h"
#endif

#ifdef ANY_ANGLE_ASSERTIONS
#include <cassert>
#endif

// Vertices are placed at the corners of cells.
// Given a map of size n x m (n x m cells, n+1 x m+1 corners):
// Top left cell is (0,0), top left corner is (0,0), and corner (x,y) is located at the top left corner of cell (x,y).
//
// We add a frame of obstacles around the map but don't add any more corners,
// resulting in n+2 x m+2 cells, n+1 x m+1 corners
//
// Now, corner x,y is located at the bottom right corner of cell x,y, and every corner is surrounded by cells
// so we don't have to do any out-of-map-bounds checks
//
// 'cells' store the blockage information of cells (only used for LOS checks after initialization)
// 'cornerIds' assign an id for each corner that has at least one neighboring cell without obstacles
// 'cornerLocs' is the reverse mapping of cornerIds: for each cornerId, it stores its location

// 1 means it is walkable
class AnyAngleAlgorithm {

public:

#ifdef ANY_ANGLE_RUNNING_IN_HOG
    AnyAngleAlgorithm(const MapEnvironment *env);                // Constructs the class using a map given as a MapEnvironment in HOG
#endif
	AnyAngleAlgorithm(const std::vector<bool> &bits, const int width, const int height);
	~AnyAngleAlgorithm();

	// Finds an any-angle path between the two xyLoc's and returns the cost. Cost is infinite if there is no path.
	// Automatically validates the path, smoothes the path (if enabled) and reports statistics (if enabled).
	// Calls either FindXYLocPath(...) or FindXYLocContPath(...) depending on the value of UsingXYLocCont().
	cost FindPath(const xyLoc from, const xyLoc to);

	// Any derived class should implement either indXYLocPath(...) or FindXYLocContPath(...) and set the value of UsingXYLocCont() accordingly.
	// xyLoc have integer coordinates, whereas xyLocCont have float coordinates.
	// For instance, Field A* finds xyLocCont paths because the midpoints of the paths can be any point on the boundary of a grid cell.
	virtual cost FindXYLocPath(xyLoc from, xyLoc to, std::vector<xyLoc> &path)          {return INFINITE_COST;}
    virtual cost FindXYLocContPath(xyLoc from, xyLoc to, std::vector<xyLocCont> &path)  {return INFINITE_COST;}

    // By default, we assume anyangle algorithms only use corner locations as midpoints in the paths and the paths are smoothed afterwards.
    // Any implementation that differs from this norm should override the relevant functions below.
    virtual const bool UsingXYLocCont() const {return false;}
    virtual const bool ShouldSmoothPaths() const {return true;}

    // Returns a (unique) name to identify the algorithm.
    // Used for setting up the statistics file. Is not called during construction, so the algorithms can generate names based on their parameters (if any).
    virtual const std::string GetName() const = 0;

    // Generates a random problem with valid start and goal locations (solution might not exist).
    void GetRandomProblem(xyLoc &l1, xyLoc &l2) const;

#ifdef ANY_ANGLE_STATISTICS
    // Generates two AnyAngleStatistics instances, for statistics with and without smoothing.
    virtual void SetupStatistics();

    // Opens up output files (algorithm_id-mapname) for writing the statistics.
    virtual void SetStatisticsFiles(const std::string mapname);

    // Prints all the statistics kept by the AnyAngleStatistics classes to the output files.
    virtual void PrintStatistics();

    // Called by PrintStatistics in case the derived class has additional statistics to report.
    virtual void PrintAdditionalStatistics(AnyAngleStatistics* stats) {};
#endif

#ifdef ANY_ANGLE_RUNNING_IN_HOG
	virtual void OpenGLDraw(const MapEnvironment *env) {};   // Visualization in HOG
    virtual void ShowPath(const MapEnvironment *env, float r = 1, float g = 0, float b = 0);
    virtual void ShowSmoothedPath(const MapEnvironment *env, float r = 0, float g = 0, float b = 1);
    virtual void VisualizeAlgorithm(const MapEnvironment *env) {}    // Algorithm specific visualization.

    virtual void SetDebugLoc(const xyLoc l) {debug_loc_ = l; ProcessDebugLoc(debug_loc_);}
    virtual void ProcessDebugLoc(const xyLoc l) {}
#endif

protected:

	// Dimensions of the map, after adding a frame of blocked cells. That is, the map has height x width cells.
	// It has (height - 1) * (width - 1) corners, each surrounded by four blocked cells.
	unsigned int height_, width_;

	// Identifies whether each cell is blocked (= 0) or not (= 1)
	bool** cell_traversable_;

	// Stores the cornerId of each valid corner (that is, any grid corner not surrounded by 4 blocked cells). -1 if the corner is surrounded by 4 blocked cells.
	cornerId** corner_ids_;

	// Reverse mapping of corner_ids_. corner_locations_[i] gives the location of the corner with id 'i'.
	std::vector<xyLoc> corner_locations_;

	// A list of all corners whose surrounding cells contain at least one unblocked cell (used for generating random instances).
	std::vector<xyLoc> valid_corner_locations_;

	// Determines whether the HeuristicDistance function returns the octile distance or the Euclidean distance.
	bool using_octile_distance_;

	// FindPath function stores the paths in the following variables.
	std::vector<xyLoc> xyloc_path_, smoothed_xyloc_path_;
	std::vector<xyLocCont> xyloc_cont_path_, smoothed_xyloc_cont_path_;


	// Start and goal of the current search (for visualization and path validation)
	xyLoc from_, to_;

	// Called by both the HOG and non-HOG constructor to add a frame of obstacles to the map and assign cornerIds.
	void Initialize();

	// Returns all the neighbors of a grid corner that can be reached by a straight line with cost 1 or sqrt(2).
	std::vector<cornerId> GetNeighbors(const cornerId c) const;
	std::vector<xyLoc> GetNeighbors(const xyLoc l) const;

	// Returns true if the straight line between the two points do not pass through the interior of an obstacle.
	// The straight line cannot pass between two blocked cells that share an edge, but can pass between two blocked cells that share only a corner.
	bool LineOfSight(xyLoc l1, xyLoc l2);
	bool LineOfSight(xyLocCont l1, xyLocCont l2);
	bool LineOfSight(cornerId c1, cornerId c2) {return LineOfSight(corner_locations_[c1], corner_locations_[c2]);}

	// Smooths a path by performing line of sight checks linear in the number of points on the original non-smooothed path.
	cost SmoothPath(const std::vector<xyLoc> &path, std::vector<xyLoc> &smoothed_path);
	cost SmoothPath(const std::vector<xyLocCont> &path, std::vector<xyLocCont> &smoothed_path);

#ifdef ANY_ANGLE_STATISTICS
    // Statistics variables. Number of line of sight checks, elapsed time, and heading changes are maintained by this class.
    // Any child class should maintain search related statistics by incrementing the relevant variables.
    int num_expansions_, num_generated_, num_percolations_, num_los_checks_;
    int num_heading_changes_, num_freespace_heading_changes_, num_taut_corner_heading_changes_, num_non_taut_corner_heading_changes_;

    Timer timer_;
    double elapsed_time_;

    AnyAngleStatistics *statistics_, *statistics_with_smoothing_;

    // Generates statistics for the paths and (optionally) validates the path by checking if it intersects with any obstacles.
    // Validation performs line-of-sight checks, but these are not reported in the statistics.
    cost EvaluatePath(const std::vector<xyLoc> & path, const bool validate_path = true);
    cost EvaluatePath(const std::vector<xyLocCont> & path, const bool validate_path = true);

    // Normalizes the degree of an angle to the range [0, 360)
    double NormalizeAngle(const double theta) const;

    // Returns the degree of the angle [0, 360) between the x-axis and the vector (x1,y1)->(x2,y2).
    double GetAngle(const double x1, const double y1, const double x2, const double y2) const;

    // Returns true if the path <(x1, y1), (x2, y2), (x3, y3)> is taut, where (x2, y2) is a corner point.
    bool IsTautCornerTurn(const double x1, const double y1, const int x2, const int y2, const double x3, const double y3) const;

    // Resets all the statistics variables (should be called before each search)
    void StartStatistics(const xyLoc from, const xyLoc to);

    // Reports all the statistics for the current search to the AnyAngleStatistics classes.
    void ReportStatistics(const std::vector<xyLoc> & path, AnyAngleStatistics* stats, const bool validate_path = true);
    void ReportStatistics(const std::vector<xyLocCont> & path, AnyAngleStatistics* stats, const bool validate_path = true);
#endif

#ifdef ANY_ANGLE_RUNNING_IN_HOG
    // Used to specify a corner and print/display information about it.
    xyLoc debug_loc_;

    // Draws lines between consecutive points on the path. If draw_mid_points is true, also draws spheres at each point on the path.
    // priority determines the size of the drawn spheres.
    void DrawPath(const MapEnvironment *env, const std::vector<xyLoc> &path, const bool draw_mid_points = true, const int priority = 0) const;
    void DrawPath(const MapEnvironment *env, const std::vector<xyLocCont> &path, const bool draw_mid_points = true, const int priority = 0) const;

    // Draws a sphere at the specified location, whose size is determined by 'priority'.
    void DrawPoint(const MapEnvironment *env, const float x, const float y, const int priority = 0) const;
    void DrawPoint(const MapEnvironment *env, const xyLocCont &l, const int priority = 0)   const {DrawPoint(env, (float)l.x, (float)l.y, priority);}
    void DrawPoint(const MapEnvironment *env, const xyLoc &l, const int priority = 0)       const {DrawPoint(env, (float)l.x, (float)l.y, priority);}

    // Draw a line between two points
    void DrawLine(const MapEnvironment *env, const float x1, const float y1, const float x2, const float y2) const;
    void DrawLine(const MapEnvironment *env, const xyLocCont &l1, const xyLocCont &l2)const {DrawLine(env, (float)l1.x, (float)l1.y, (float)l2.x, (float)l2.y);}
    void DrawLine(const MapEnvironment *env, const xyLoc &l1, const xyLoc &l2) const {DrawLine(env, (float)l1.x, (float)l1.y, (float)l2.x, (float)l2.y);}
#endif

    // Returns true if the cell is unblocked
    bool IsTraversable(const xyLoc l) const    {return cell_traversable_[l.x][l.y];}
    bool IsTraversable(const unsigned int x, const unsigned int y) const   {return cell_traversable_[x][y];}

    // Returns the cornerId of the given xyLoc.
    cornerId ToCornerId(const uint16_t x, const uint16_t y) const {return corner_ids_[x][y];}
    cornerId ToCornerId(const xyLoc & l) const {return corner_ids_[l.x][l.y];}

    // Returns the xyLoc of the given cornerId
    xyLoc ToXYLoc(const cornerId id) const {return xyLoc(corner_locations_[id].x, corner_locations_[id].y);}

	// Returns the neighboring cell of a corner, specified by the direction in function's name.
	// Corner (x,y)'s surrounding cells are (x,y), (x+1,y), (x,y+1), (x+1,y+1).
	xyLoc NorthWestCell(const xyLoc l) const {return l;}
	xyLoc NorthEastCell(const xyLoc l) const {return xyLoc(l.x+1,l.y);}
	xyLoc SouthWestCell(const xyLoc l) const {return xyLoc(l.x,l.y+1);}
	xyLoc SouthEastCell(const xyLoc l) const {return xyLoc(l.x+1,l.y+1);}

	xyLoc NorthWestCell(const unsigned int x, const unsigned int y) const {return xyLoc(x,y);}
	xyLoc NorthEastCell(const unsigned int x, const unsigned int y) const {return xyLoc(x+1,y);}
	xyLoc SouthWestCell(const unsigned int x, const unsigned int y) const {return xyLoc(x,y+1);}
	xyLoc SouthEastCell(const unsigned int x, const unsigned int y) const {return xyLoc(x+1,y+1);}

	xyLoc NorthWestCell(const cornerId id) const {return NorthWestCell(ToXYLoc(id));}
	xyLoc NorthEastCell(const cornerId id) const {return NorthEastCell(ToXYLoc(id));}
	xyLoc SouthWestCell(const cornerId id) const {return SouthWestCell(ToXYLoc(id));}
	xyLoc SouthEastCell(const cornerId id) const {return SouthEastCell(ToXYLoc(id));}

	// Returns true if the corner is a convex corner of an obstacle (that is, a visibility graph vertex).
	bool IsConvexCorner(const xyLoc l) const
        {return ((IsTraversable(NorthWestCell(l)) && IsTraversable(SouthEastCell(l))) && !(IsTraversable(NorthEastCell(l)) && IsTraversable(SouthWestCell(l))))
			|| (!(IsTraversable(NorthWestCell(l)) && IsTraversable(SouthEastCell(l))) && (IsTraversable(NorthEastCell(l)) && IsTraversable(SouthWestCell(l))));}
	bool IsConvexCorner(const unsigned int x, const unsigned int y) const  {return IsConvexCorner(xyLoc(x,y));}
	bool IsConvexCorner(const cornerId c) const  {return IsConvexCorner(corner_locations_[c]);}

	// Returns x such that (x,y), (x1,y1) and (x2,y2) are colinear.
	double GetIntersectingX(const double x1, const double y1, const double x2, const double y2, const double y) const
        {return (fabs(y2-y1) < EPSILON) ? y1 : ((x2-x1)/(y2-y1))*(y-y1) + x1;}

    // Returns y such that (x,y), (x1,y1) and (x2,y2) are colinear.
	double GetIntersectingY(const double x1, const double y1, const double x2, const double y2, const double x) const
        {return (fabs(x2-x1) < EPSILON) ? x1 : ((y2-y1)/(x2-x1))*(x-x1) + y1;}

	// Returns true if the interval (f1, f2) or (f2, f1) contains an integer value.
	bool IntervalContainsInteger(const double f1, const double f2) const
        {if (fabs(f1 -f2) < EPSILON) return false;	// f1 = f2: The interval is empty.
		return (f1 < f2) ? (ceil(f1 - EPSILON) == floor(f2 + EPSILON)) : (ceil(f2 - EPSILON) == floor(f1 + EPSILON));}

	// Returns true if (x1,y1), (x2,y2) and (x3,y3) are colinear.
	bool CoLinear(const double x1, const double y1, const double x2, const double y2, const double x3, const double y3) const
        {return fabs((y2-y1)*(x3-x2) - (x2-x1)*(y3-y2)) < EPSILON;}

	// Returns the Octile distance between two corners.
	cost OctileDistance(const xyLoc l1, const xyLoc l2) const
		{int dx = (l1.x>l2.x)?(l1.x-l2.x):(l2.x-l1.x);	int dy = (l1.y>l2.y)?(l1.y-l2.y):(l2.y-l1.y);
		return (dx>dy)?(dx*CARD_COST + dy*DIAG_DIFF):(dy*CARD_COST + dx*DIAG_DIFF);}

    cost OctileDistance(const cornerId c1, const cornerId c2) const
        {return OctileDistance(corner_locations_[c1], corner_locations_[c2]);}

    // Returns the Euclidean distance between two points.
	cost EuclideanDistance(const xyLoc l1, const xyLoc l2) const
		{double dx = (l2.x-l1.x);	double dy = (l2.y-l1.y);	return sqrt((cost)dx*dx + dy*dy);}

	cost EuclideanDistance(const double x1, const double y1, const double x2, const double y2) const
		{double dx = (x2-x1);	double dy = (y2-y1);	return sqrt((cost)dx*dx + dy*dy);}

	cost EuclideanDistance(const xyLocCont l1, const xyLocCont l2) const
		{double dx = (l2.x-l1.x);	double dy = (l2.y-l1.y);	return sqrt((cost)dx*dx + dy*dy);}

	cost EuclideanDistance(const cornerId c1, const cornerId c2) const
        {return EuclideanDistance(corner_locations_[c1], corner_locations_[c2]);}

	// Returns either the Octile or the Euclidean distance, depending on the value of 'using_octile_distance_'.
	cost HeuristicDistance(const xyLoc l1, const xyLoc l2) const
        {return using_octile_distance_?OctileDistance(l1,l2):EuclideanDistance(l1,l2);}

	cost HeuristicDistance(const cornerId c1, const cornerId c2) const
        {return HeuristicDistance(corner_locations_[c1], corner_locations_[c2]);}

	// Sets which heuristic function to use.
	void UseOctileDistance(){using_octile_distance_ = true;}
	void UseEuclideanDistance(){using_octile_distance_ = false;}
};

#endif
