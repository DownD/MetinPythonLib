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

#ifndef FIELD_A_STAR_H
#define FIELD_A_STAR_H

#include "AnyAngleAlgorithm.h"

#define FIELD_A_STAR_LOOKAHEAD 0
// TODO: Make this a parameter?

class FieldAStar : public AnyAngleAlgorithm
{
public:
	FieldAStar(std::vector<bool> &bits, int width, int height);
#ifdef ANY_ANGLE_RUNNING_IN_HOG
    FieldAStar(MapEnvironment *env);
#endif
	~FieldAStar();

	cost FindXYLocContPath(xyLoc from, xyLoc to, std::vector<xyLocCont> &path);


	const std::string GetName() const {return "F";}
    virtual const bool UsingXYLocCont() const {return true;}

private:
	// Search info
	struct Corner {
		Corner(){}

		std::vector<cornerId> neighbors;	// neighbors in the grid (max 8)
		std::vector<cost> neighbor_dist;

		uint16_t generated;
		cornerId parent;
		cost g_val;
		cost h_val;
		char list;

		int heap_index;
	};
	std::vector<Corner> corners_;

	// Priority queue
	struct HeapElement
	{
		HeapElement(cornerId _id, cost _g_val, cost _f_val) : id(_id), g_val(_g_val), f_val(_f_val) {}
		cornerId id;
		cost g_val;
		cost f_val;

		bool operator<(const HeapElement& rhs) const
		{
		    //return fVal < lhs.fVal;	// no-tie break
		  	return (f_val + EPSILON < rhs.f_val) ? true : ((rhs.f_val + EPSILON < f_val) ? false : g_val > rhs.g_val);	// tie-break towards larger g-values
		  	return (f_val + EPSILON < rhs.f_val) ? true : ((rhs.f_val + EPSILON < f_val) ? false : g_val < rhs.g_val);	// tie-break towards lower g-values
		}
	};
	std::vector<HeapElement> open_list_;

    // The search counter. Any state whose generated value is equal to this value is considered to be generated. Resets every 50,000 searches.
    uint16_t search_;

    void FieldAStarInitialize();

    void GenerateState(cornerId s, cornerId goal);
    void ResetSearch();

    cost ComputeCost(cornerId s, cornerId sc, cornerId sd);

    // If the start location is 'a' units away from the interpolation line and the interpolation line is 'b' units long, return the distance of the best point
    // from the cardinal neighbor.
    void InterpolateContinuous(double a, double b, double cCost, double dCost, double & y, double & cost);

    void GetNeighbors(xyLocCont l, std::vector<xyLocCont> & neighbors, std::vector<cost> & gValues, std::vector<bool> & exists, int lookahead = 0);

    void GetBestCostAndParent(xyLocCont l, xyLocCont & neighbor, cost & gVal, int lookahead = 1);
    cost ExtractPath(xyLoc l1, xyLoc l2, std::vector<xyLocCont> & thePath);
    cost FieldAStarSearch(xyLoc l1, xyLoc l2, std::vector<xyLocCont> & thePath);

	// Heap operations.
	void AddToOpen(const cornerId id);
	HeapElement GetMin() const {return open_list_.front();}
	void PopMin();
	void PercolateUp(int index);
	void PercolateDown(int index);
};


#endif
