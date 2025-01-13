
#pragma once

#include <vector>

class CAdjancencyMatrix;

/// @brief find max size clique by brute force
/// @param matrix adjancency matrix of a graph
/// @return list of vertices belong to the clique
std::vector<int> findMaxCliqueBruteForce(const CAdjancencyMatrix& matrix);
