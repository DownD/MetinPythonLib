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

#ifndef SUBGOAL_AA_DEFINITIONS_H
#define SUBGOAL_AA_DEFINITIONS_H


typedef uint32_t subgoalId;
#ifdef _WIN32
#include <float.h>
#define NON_SUBGOAL UINT_MAX
#else
#define NON_SUBGOAL (subgoalId)std::numeric_limits<subgoalId>::max()
#endif

typedef uint32_t mapLoc;
typedef uint32_t direction;

typedef uint16_t level;

#endif
