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

#ifndef ANYA_H
#define ANYA_H

#include "AnyAngleAlgorithm.h"
#include "ANYAIndexTable.h"

#define TOP 0
#define BOT 1
#define LEFT 0
#define RIGHT 1

#ifdef ANY_ANGLE_RUNNING_IN_HOG
//#define ANYA_DEBUG
#endif

class ANYA: public AnyAngleAlgorithm {
public:
    ANYA(std::vector<bool> &bits, int width, int height);
#ifdef ANY_ANGLE_RUNNING_IN_HOG
    ANYA(MapEnvironment *env);
#endif
    ~ANYA();

    const std::string GetName() const
    {
        return "ANYA";
    }
    //const bool ShouldSmoothPaths() const {return false;}
    cost FindXYLocPath(xyLoc from, xyLoc to, std::vector<xyLoc> &thePath);

#ifdef ANY_ANGLE_RUNNING_IN_HOG
    void VisualizeAlgorithm(const MapEnvironment *env);
#endif

private:
    struct Corner {
        Corner()
        {
        }

        // For all the indices, Left = 0, Right = 1 and Top = 0, Bottom = 1

        int interval_clearance[2]; // Length of the (half) interval extending left/right from this corner.
        bool interval_blocked[2][2]; // Whether the corner's surrounding cells are blocked	(used to determine the type of the interval).
        int clearance[2][2]; // 0,0 is Top Left and represents the number of unblocked cells towards left, starting from the top-left cell of the corner.

        bool is_transition_point; // True if it is on the boundary of an interval (that is, interval_blocked[i][0] != interval_blocked[i][1]).
        bool is_convex_corner; // True if it is at a convex corner of an obstacle.
    };
    Corner** corners_;

    struct Interval {
        Interval(int _row = 0, double _left = 0, double _right = 0) :
            row(_row), f_left(_left), f_right(_right)
        {
            i_left = floor(f_left + EPSILON);
            i_right = ceil(f_right - EPSILON);
        }

        // An interval is defined by its left and right end-points (double) and its row (int).
        // For convenience, we also maintain integer end-points. Interval with double end-points is a subset of the interval with integer end-points.
        int row, i_left, i_right;
        double f_left, f_right;

        // True if its top [0] or bottom [1] are completely blocked by obstacles. False if its top [0] or bottom [1] have no obstacles.
        // By definition of intervals, there is no middle ground where the top (or bottom) side of the interval has both blocked and unblocked cells.
        bool blocked[2];

        // Assumption: map dimensions are at most 2^11 (after adding surrounding obstacles).
        // We use the integer end-points for hashing. This can result in some ambiguity, which is addressed in the GenerateState(..) function.
        uint64_t GetHash() const
        {
            uint64_t h = 0;
            h = h | row;
            h = h | (((uint64_t) i_left) << 12);
            h = h | (((uint64_t) i_right) << 24);

            return h;
        }
    };

    // Search info
    struct State {
        State()
        {
        }
        State(xyLoc _l, Interval _interval) :
            l(_l), interval(_interval)
        {
        }

        xyLoc l;
        Interval interval;

        stateId parent; // Parent of the state
        cost g_val; // Initialized to infinity when generated
        cost h_val; // Initialized to the heuristic distance to the goal when generated
        char list; // Initially NO_LIST, can be changed to OPEN_LIST or CLOSED_LIST

        int heap_index; // The location of the state on the open list
    };
    std::vector<State> states_; // For each state, keeps the search related information

    // Priority queue
    struct HeapElement {
        HeapElement(stateId _id, cost _g_val, cost _f_val) :
            id(_id), g_val(_g_val), f_val(_f_val)
        {
        }
        stateId id;

        cost g_val; // Used for tie-breaking
        cost f_val; // Main key

        bool operator<(const HeapElement &rhs) const
        {
            //return fVal < rhs.fVal;	// no-tie break
            return (f_val + EPSILON < rhs.f_val) ? true : ((rhs.f_val + EPSILON < f_val) ? false : g_val > rhs.g_val); // tie-break towards larger g-values
        }
    };
    std::vector<HeapElement> open_list_;

    // Hash table that is used to lookup a stateId, given a state's key (generated from its xyLoc and Interval).
    ANYAIndexTable anya_index_table_;

    void ANYAInitialize();

    // Compute clearance values for cells that will be useful for generating intervals on-the-fly.
    void ComputeClearances();

    // Generates a key for a state = (xyLoc, Interval).
    // The keys are unique (except for intervals that have the same integer end-points but different double end-points.
    const ANYAStateKey GenerateKey(const xyLoc l, const Interval I) const;

    // Uses the index table to determine whether a state has been generated before. If not, it generates the state by adding it to the states_ vector.
    const stateId GenerateState(const xyLoc l, const Interval I, const xyLoc goal);

    // Setting (x,y) as one of the end-points, returns the largest interval that stretches towards left (or right).
    Interval GetLeftInterval(const int x, const int y) const;
    Interval GetRightInterval(const int x, const int y) const;

    // Computes and places all intervals in the 'row' between 'left' and 'right' in the 'sub' vector.
    void GetSubIntervals(const int row, const double left, const double right, std::vector<Interval> & sub) const;

    // Generates states for each interval computed by 'GetSubIntervals(..)' with roots 'root'. Appends the id's of the generated states to the 'ids' vector.
    void GenerateSubIntervals(const int row, const double left, const double right, const xyLoc root, const xyLoc goal, std::vector<stateId> & ids);

    // Starting from the point (fx, y), generates intervals towards left (or right) as long as their top or bottom (identified by 'unblocked_dir') do not have
    // any blocked cells, or the 'bound' is reached (by default, the left-most and right-most columns of the map, respectively), using the function
    // GenerateSubIntervals(..).
    // It then generates a state for each such generated interval, setting 'root' as their root points.
    void GenerateIntervalsTowardsLeft(const double fx, const int y, const int unblocked_dir, const xyLoc root, const xyLoc goal,
            std::vector<stateId> & successors, const double bound = 0);
    void GenerateIntervalsTowardsRight(const double fx, const int y, const int unblocked_dir, const xyLoc root, const xyLoc goal,
            std::vector<stateId> & successors, const double bound = INFINITE_COST);

    // Generate all the successors of a state (root, interval). GenerateSuccessors(..) calls either GenerateSuccessors_SameRow(..) or
    // GenerateSuccessors_DifferentRow(..), depending on whether the root is on the same row as the interval. The id's of the successors are stored in the
    // 'successors' vector.
    void GenerateSuccessors_SameRow(const xyLoc root, const Interval interval, const xyLoc goal, std::vector<stateId> & successors);
    void GenerateSuccessors_DifferentRow(const xyLoc root, const Interval interval, const xyLoc goal, std::vector<stateId> & successors);
    void GenerateSuccessors(const xyLoc root, const Interval interval, const xyLoc goal, std::vector<stateId> & successors);

    // Initializes the open list with the set of states generated by 'expanding' the start state.
    // The start state is a special state since its root and both end points of its interval correspond to the same point, namely, the start location.
    void InitializeSearch(const xyLoc from, const xyLoc to);

    // An interval with double end-points x1 and x2 have the integer end-points GetLeftColumn(x1) and GetRightColumn(x2).
    int GetLeftColumn(const double x) const {return floor(x + EPSILON);}
    int GetRightColumn(const double x) const {return ceil(x - EPSILON);}

    // ANYA uses a different heuristic function: It estimates the length of a shortest path from 'root' to the goal through the interval.
    cost HeuristicDistance(const xyLoc root, const Interval interval, const xyLoc goal) const;

    // Clears the states_ vector, the index table, and the open list.
    void ResetSearch();

    // Heap operations.
    void AddToOpen(const stateId id);
    HeapElement GetMin() const {return open_list_.front();}
    void PopMin();
    void PercolateUp(int index);
    void PercolateDown(int index);


#ifdef ANYA_DEBUG
    void PrintState(stateId i)
    {
        State* s = &states_[i];
        printf("Root: (%u, %u)\tInterval: [%.2f (%u), %.2f (%u)], row %u \t g(%.2f) + h(%.2f) = f(%.2f)\n", s->l.x, s->l.y, s->interval.f_left,
                s->interval.i_left, s->interval.f_right, s->interval.i_right, s->interval.row, s->g_val, s->h_val, s->g_val + s->h_val);
    }
#endif

#ifdef ANY_ANGLE_RUNNING_IN_HOG
    // For visualization
    std::vector<stateId> state_path_;
    void ProcessDebugLoc(xyLoc l);
#endif

};

#endif
