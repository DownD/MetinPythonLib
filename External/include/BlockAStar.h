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

#ifndef BLOCK_A_STAR_H
#define BLOCK_A_STAR_H

#include "AnyAngleAlgorithm.h"

#ifdef ANY_ANGLE_RUNNING_IN_HOG
//#define BLOCK_A_STAR_DEBUG
#endif


typedef int32_t blockId;

class BlockAStar : public AnyAngleAlgorithm
{
public:
	BlockAStar(std::vector<bool> &bits, int width, int height, int block_size = 5);
#ifdef ANY_ANGLE_RUNNING_IN_HOG
    BlockAStar(MapEnvironment *env, int block_size = 5);
#endif
	~BlockAStar();

	const std::string GetName() const {return "B";}
    cost FindXYLocPath(xyLoc from, xyLoc to, std::vector<xyLoc> &path);

private:

	void BlockAStarInitialize();
	void InitializeBlock(xyLoc l);
	void ResetSearch();
	void GenerateBlock(blockId b, cornerId goal);
	void GenerateCorner(cornerId c, cornerId goal);
	blockId GetCommonBlock(cornerId c1, cornerId c2);
	cost BlockAStarSearch(xyLoc l1, xyLoc l2, std::vector<xyLoc> & thePath);

	bool OnHorizontalBlockEdge(xyLoc l){return l.x%block_cell_width_ == 0;}
	bool OnVerticalBlockEdge(xyLoc l){return l.y%block_cell_width_ == 0;}
	bool OnBlockEdge(xyLoc l){return OnHorizontalBlockEdge(l) || OnVerticalBlockEdge(l);}
	bool OnBlockEdge(cornerId c){return OnBlockEdge(corner_locations_[c]);}
	bool OnBlockCorner(xyLoc l){return OnHorizontalBlockEdge(l) && OnVerticalBlockEdge(l);}
	bool OnBlockCorner(cornerId c){return OnBlockCorner(corner_locations_[c]);}

	cost FindPathWithinBlock(blockId b, cornerId start, cornerId goal, std::vector<xyLoc> &path);
	void GetBoundaryCorners(cornerId c, std::vector<cornerId> &boundary_corners, std::vector<cost> &boundary_costs, std::vector<cornerId> &boundary_next);

	// A block of size blockSize x blockSize contains blockCellWidth^2 cells and blockSize^2 corners
	unsigned int block_size_, block_cell_width_, map_block_width_, map_block_height_;

	// Info for each corner on the map
	struct Corner{
		std::vector<blockId> belongs_to;
		std::vector<int> index_in_block;
		cost g_val;
		cost h_val;
		cornerId parent;
		uint16_t generated;
	};

	std::vector<Corner> corners_;

    // The search counter. Any state whose generated value is equal to this value is considered to be generated. Resets every 50,000 searches.
    uint16_t search_;

	// Info for each block on the map
	struct Block{
		Block(xyLoc l) :top_left(l){dist = NULL; next = NULL; last_g_val = NULL;}
		xyLoc top_left;	// Its top-left corner marks the location of the block

		std::vector<cornerId> visibility_corners;	// Visibility corners vector contains the corners on the boundary of the cell + corners that are at convex corners of obstacles
		unsigned int num_boundary;	// the first nBoundary corners in the vCorners array is on the boundary, the rest are just convex non-boundary corners

		cost** dist;	// The pairwise distances between boundary corners
						// Dimensions: vCorners.size()^2	// For convenience, otherwise nBoundary^2 is the norm

		char** next;	// Which visibility corner to move to in order to optimally reach a boundary corner.
						// Dimensions: vCorners.size()^2 	// For convenience, otherwise vCorners.size() x nBoundary


		uint16_t generated;
		cost g_val;
		cost h_val;
		char list;
		cost* last_g_val;
		int heap_index;
		bool is_goal_block;

		void clear();	// Frees memory by deleting the stored data pointed by dist, next and vis
		int getCornerIndex(cornerId c);
	};
	std::vector<Block> blocks_;

	// Priority queue
	struct HeapElement
	{
		HeapElement(blockId _id, cost _g_val, cost _f_val) : id(_id), g_val(_g_val), f_val(_f_val) {}
		blockId id;
		cost g_val;
		cost f_val;

		bool operator<(const HeapElement& rhs) const
		{
		    //return fVal < rhs.fVal;	// no-tie break
		  	return (f_val + EPSILON < rhs.f_val) ? true : ((rhs.f_val + EPSILON < f_val) ? false : g_val > rhs.g_val);	// tie-break towards larger g-values

		}
	};
	std::vector<HeapElement> open_list_;

	// Heap operations.
	void AddToOpen(const blockId id);
	HeapElement GetMin() const {return open_list_.front();}
	void PopMin();
	void PercolateUp(int index);
	void PercolateDown(int index);
};
#endif
