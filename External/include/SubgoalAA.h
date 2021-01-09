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

#ifndef SUBGOAL_AA_H
#define SUBGOAL_AA_H

#include "AnyAngleAlgorithm.h"
#include "SubgoalAADefinitions.h"
#include <string>
#include <sstream>
#include <cassert>

#define SUBGOAL_A_STAR 0
#define SUBGOAL_THETA_STAR 1

class SubgoalAA: public AnyAngleAlgorithm {
public:
    SubgoalAA(std::vector<bool> &bits, int width, int height, int level = 1, int search_method = SUBGOAL_THETA_STAR);
    ~SubgoalAA();

    const std::string GetName() const;
    cost FindXYLocPath(xyLoc from, xyLoc to, std::vector<xyLoc> &path);

#ifdef ANY_ANGLE_STATISTICS
    void PrintAdditionalStatistics(AnyAngleStatistics* stats);
#endif

#ifdef ANY_ANGLE_RUNNING_IN_HOG
    SubgoalAA(MapEnvironment *env, int level = 1, int search_method = SUBGOAL_THETA_STAR);
    void ProcessDebugLoc(const xyLoc loc);
    void VisualizeAlgorithm(const MapEnvironment *env);
#endif

private:

    // Use either A* or Theta*.
    unsigned int search_method_;

    // Partitioning will stop after generating this many levels.
    unsigned int max_graph_level_;

    // Number of all subgoals, global subgoals and levels in the graph, respectively.
    // graph_level_ can be smaller than max_graph_level_ if the global subgoal graph cannot be partitioned anymore.
    unsigned int num_subgoals_, num_global_subgoals_, graph_level_;

    // Given a mapLoc loc and a direction d, this array can be used to determine the loc' we end up at if we follow d from loc (loc' = loc + deltaMapLoc[d]).
    // Note that, even there are only 8 directions, we keep 3 copies for each direction for a total of 24 values in order to minimize the number of modulus
    // operations. This array is initialized after we know the width and height of the (padded) map.
    int delta_map_loc_[24];

    // The search counter. Any state whose generated value is equal to this value is considered to be generated. Resets every 50,000 searches.
    uint16_t search_;

    // Information to keep for each subgoal
    struct Subgoal {
        Subgoal() {}

        xyLoc loc;

        level lvl;  // Level of the subgoal.

        // Keeps all the neighbors that are at a higher level or, if this subgoal is at the highest level, the neighbors at the highest level.
        std::vector<subgoalId> neighbors;

        // Keeps all the neighbors at the same level. If this subgoal is at the highest level, then it has no local neighbors.
        // Local neighbors are not considered during search, but are used to add extra neighbors before a search.
        std::vector<subgoalId> local_neighbors;

        // When the graph is modified for a search, some subgoals get an extra edge, which is added to the neighbors vector.
        bool has_extra_edge;

        // Edge distances. Can be dropped for memory efficiency.
        std::vector<cost> neighbor_dist;
        std::vector<cost> local_neighbor_dist;

        uint16_t generated; // The id of the last search that this subgoal has been initialized in.
        subgoalId parent;
        cost g_val;
        cost h_val;
        char list; // Initially NO_LIST, can be changed to OPEN_LIST or CLOSED_LIST.

        int heap_index; // The location of the subgoal on the open list.
    };
    std::vector<Subgoal> subgoals_;

    struct Corner {
        Corner() {}
        subgoalId sg_id;

        // Currently, stores clearances in only four directions. Could be implemented in a more memory efficient way by dropping the unused indices.
        int clearance[8];
    };
    std::vector<Corner> corners_;

    std::vector<std::vector<subgoalId> > buckets_; // For connecting the goal to the graph

    void SubgoalAAInitialize(int level, int search_method);
    void CreateGraph(int level);

    void AddEuclideanEdge(subgoalId sg1, subgoalId sg2);
    void AddOctileEdge(subgoalId sg1, subgoalId sg2);
    void AddEdge(subgoalId sg1, subgoalId sg2, cost c);
    void MakeEdgeLocal(subgoalId sg1, subgoalId sg2);
    bool EdgeExists(subgoalId sg1, subgoalId sg2);

    /// Main preprocessing functions
    void SetDirections(); // Initialize the deltaMapLoc array (after we know the map dimensions)
    void IdentifySubgoals(); // Place a subgoal at every corner
    void ComputeClearances(); // Compute clearances (from an obstacle or subgoal) for the 4 cardinal directions
    void LinkSubgoals(); // Add edges so that every subgoal is optimally reachable from each other in the graph
    void GetDirectHReachableSubgoals(xyLoc & from, std::vector<subgoalId> & subgoals);
    void FinalizeGraph();

    /// Partitioning functions
    bool AddLevel();
    cost CostOtherPath(subgoalId sg, subgoalId sg1, subgoalId sg2, cost limit);
    bool CanDemoteSubgoal(subgoalId sg, std::vector<subgoalId> & from, std::vector<subgoalId> & to);
    bool IsNecessaryToConnect(subgoalId sg, subgoalId sg1, subgoalId sg2, std::vector<subgoalId> & from, std::vector<subgoalId> & to);
    void DemoteSubgoal(subgoalId sg, std::vector<subgoalId> & from, std::vector<subgoalId> & to);

    bool IsGlobalSubgoal(subgoalId s)
    {
        return subgoals_[s].lvl == graph_level_;
    }
    void DecrementLevel(subgoalId s)
    {
        if (subgoals_[s].lvl == graph_level_)
            subgoals_[s].lvl--;
    }

    void GenerateState(subgoalId s, subgoalId goal); // Initialize a state for a search if it has not been initialized before for this search
    void ResetSearch(); // Set the search counter and all generated values to 0

    cost FindSubgoalAnyAnglePath(xyLoc l1, xyLoc l2, std::vector<xyLoc> & thePath, bool theta = true);

    cost SubgoalAStarSearch(subgoalId sg1, subgoalId sg2, cost limit = INFINITE_COST); // Does not use LOS checks during search
    cost SubgoalThetaStarSearch(subgoalId sg1, subgoalId sg2, cost limit = INFINITE_COST);

    cost EuclideanDistanceSG(subgoalId sg1, subgoalId sg2)
    {
        return EuclideanDistance(subgoals_[sg1].loc, subgoals_[sg2].loc);
    }
    cost OctileDistanceSG(subgoalId sg1, subgoalId sg2)
    {
        return OctileDistance(subgoals_[sg1].loc, subgoals_[sg2].loc);
    }

    void ConnectStartAndGoalToGraph(xyLoc & start, xyLoc & goal, subgoalId & sgStart, subgoalId & sgGoal,
            std::vector<subgoalId> & startDirectHReachableSubgoals, std::vector<subgoalId> & goalDirectHReachableSubgoals);
    cost ExtractPath(subgoalId l1, subgoalId l2, std::vector<xyLoc> &path);

    bool IsSubgoal(const cornerId id) const {return corners_[id].sg_id != NON_SUBGOAL;}
    bool IsSubgoal(const xyLoc l) const {return IsSubgoal(ToCornerId(l));}

    subgoalId ToSubgoalId(const cornerId id) const  {return corners_[id].sg_id;}
    subgoalId ToSubgoalId(const xyLoc l) const      {return ToSubgoalId(ToCornerId(l));}

    void SetClearance(const cornerId id, const direction d, const int c)
    {
        corners_[id].clearance[d] = c;
    }
    int GetClearance(const cornerId id, const direction d) const
    {
        return corners_[id].clearance[d];
    }

    double ssg_construction_time_, partitioning_time_;

    // Priority queue
    struct HeapElement {
        HeapElement(subgoalId _id, cost _g_val, cost _f_val) :
            id(_id), g_val(_g_val), f_val(_f_val)
        {}

        subgoalId id;
        cost g_val; // Used for tie-breaking
        cost f_val; // Main key

        bool operator<(const HeapElement& rhs) const
        {
            //return fVal < rhs.fVal;	// no-tie break
            return (f_val + EPSILON < rhs.f_val) ? true : ((rhs.f_val + EPSILON < f_val) ? false : g_val > rhs.g_val); // tie-break towards larger g-values
        }
    };
    std::vector<HeapElement> open_list_;

    // Heap operations.
    void AddToOpen(const subgoalId id);
    HeapElement GetMin() const {return open_list_.front();}
    void PopMin();
    void PercolateUp(int index);
    void PercolateDown(int index);

};

#endif
