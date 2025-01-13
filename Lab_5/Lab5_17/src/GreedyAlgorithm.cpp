
#include "GreedyAlgorithm.hpp"

#include "../../common/CAdjancencyMatrix.hpp"

#include <vector>
#include <iostream>

bool recursiveTraverse(const CAdjancencyMatrix& matrix, std::vector<int> clique, int vertex, std::vector<int> whiteList, std::vector<int>& maxClique)
{
   clique.push_back(vertex);

   auto it = std::remove(whiteList.begin(), whiteList.end(), vertex);
   whiteList.erase(it, whiteList.end());

   if (whiteList.empty())
   {
      maxClique = clique;
      return true;
   }

   for (auto& item : whiteList) {
      if (matrix.isClique(clique, item)) {
         if (recursiveTraverse(matrix, clique, item, whiteList, maxClique)) {
            return true;
         }
      }
      if (maxClique.size() < clique.size()) {
         maxClique = clique;

         std::cout << "New max clique: size[" << clique.size() << "] path[";
         for (auto& v : clique) {
            std::cout << v << " ";
         }
         std::cout << "]" << std::endl;
      }
   }

   return false;
}

std::vector<int> findMaxCliqueBruteForce(const CAdjancencyMatrix& matrix)
{
   std::vector<int> cliqueMax;
   cliqueMax.reserve(matrix.mVCount);

   std::vector<int> whiteList;
   for (int i = 0; i < matrix.mVCount; ++i) {
      whiteList.push_back(i);
   }

   for (int vertex = 0; vertex < matrix.mVCount; ++vertex) {
      std::vector<int> clique;
      if (recursiveTraverse(matrix, clique, vertex, whiteList, cliqueMax)) {
         break; // all vertices are connected with each other
      }
   }

   return cliqueMax;
}
