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

#ifndef THETA_STAR_H
#define THETA_STAR_H

#include "AnyAngleAlgorithm.h"

#define A_STAR_EUC 0
#define A_STAR_OCT 1
#define THETA_STAR 2
#define LAZY_THETA_STAR 3

class ThetaStar : public AnyAngleAlgorithm
{
public:
	ThetaStar(std::vector<bool> &bits, int width, int height, int search_method = THETA_STAR);
#ifdef ANY_ANGLE_RUNNING_IN_HOG
	ThetaStar(MapEnvironment *env, int algorithm = THETA_STAR);
#endif
    ~ThetaStar();

    const std::string GetName() const;
	cost FindXYLocPath(xyLoc from, xyLoc to, std::vector<xyLoc> &path);

private:
	// Use either A* (with Euclidean or Octile distance), Theta* or Lazy Theta*.
	int search_method_;

    // The search counter. Any state whose generated value is equal to this value is considered to be generated. Resets every 50,000 searches.
    uint16_t search_;

	// Search info
	struct Corner{
		Corner(){}

		std::vector<cornerId> neighbors;	// Neighbors on the grid (max 8).
		std::vector<cost> neighbor_dist;	// neighbor_dist[i] is the distance from this corner to neighbors[i].
		uint16_t generated;	// The id of the last search in which this corner has been generated.
		cornerId parent;	// Parent of the corner.
		cost g_val;			// Initialized to infinity when generated.
		cost h_val;			// Initialized to the heuristic distance to the goal when generated.
		char list;			// Initially NO_LIST, can be changed to OPEN_LIST or CLOSED_LIST.

		int heap_index;		// The location of the corner on the open list
	};
	std::vector<Corner> corners_;

	// Priority queue
	struct HeapElement
	{
		HeapElement(cornerId _id, cost _g_val, cost _f_val) : id(_id), g_val(_g_val), f_val(_f_val) {}
		cornerId id;
		cost g_val;	// Used for tie-breaking
		cost f_val;	// Main key

		bool operator<(const HeapElement& rhs) const
		{
		    //return fVal < rhs.fVal;	// no-tie break
		  	return (f_val + EPSILON < rhs.f_val) ? true : ((rhs.f_val + EPSILON < f_val) ? false : g_val > rhs.g_val);	// tie-break towards larger g-values
		}
	};
	std::vector<HeapElement> open_list_;

    void ThetaStarInitialize(const int search_method);

    // Lazy Theta* assumes that there is always line-of-sight from the parent of an expanded state to a successor state.
    // When expanding a state, it checks if this assumption was true. If not, it updates the g-value of the state and puts it back in the open list.
    void ValidateParent(const cornerId s, const cornerId goal);

    // Main search methods.
    cost AStarSearch(const xyLoc from, const xyLoc to, std::vector<xyLoc> & path);
    cost ThetaStarSearch(const xyLoc from, const xyLoc to, std::vector<xyLoc> & path);
    cost LazyThetaStarSearch(const xyLoc from, const xyLoc to, std::vector<xyLoc> & path);

    // Generates a corner as a state if its 'generated' value is lower than 'search_counter_'. Each corner can be generated once per search.
    void GenerateState(const cornerId s, const cornerId goal);

    // Set the search counter and all generated values to 0
    void ResetSearch();

    // Heap operations.
	void AddToOpen(const cornerId id);
	HeapElement GetMin() const {return open_list_.front();}
	void PopMin();	// Remove the corner with the minimum f-value from the open list and move it to the closed list
	void PercolateUp(int index);
	void PercolateDown(int index);
};


#endif
