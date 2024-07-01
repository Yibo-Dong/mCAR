#ifndef UTILITY_H
#define UTILITY_H

#include "basic_data.h"
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <algorithm>


namespace car{

void print (const std::vector<int>& v);

void print (const std::unordered_set<int>& s);

void print (const std::unordered_set<unsigned>& s);

void print (const std::unordered_map<int, int>& m);

void print (const std::unordered_map<int, std::vector<int> >& m);

//elements in v1, v2 are in order
//check whether v2 is contained in v1 
// bool imply (const std::vector<int>& v1, const std::vector<int>& v2, bool inorder);

std::vector<int> vec_intersect (const std::vector<int>& v1, const std::vector<int>& v2);
inline std::vector<int> cube_intersect (const std::vector<int>& v1, const std::vector<int>& v2)
{
	return vec_intersect (v1, v2);
}

bool comp (int i, int j);

// my section

Cube negate(const Cube& cu);

// Cube minus(const Cube& c1, const Cube& c2);

// Cube intersect(const Cube& c1, const Cube& c2);

template <typename T>
void shuffle(std::vector<T>& vec) {
    #ifdef RANDSEED
        int seed = RANDSEED;
    #else
        int seed = 1;
    #endif
    std::mt19937 g(seed);
    std::shuffle(vec.begin(), vec.end(), g);
}

}

#endif


