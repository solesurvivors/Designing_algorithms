
#pragma once

#include "CAdjancencyMatrix.hpp"

#include <iostream>
#include <algorithm>

class CAnt
{
public:
   CAnt()
        : CAnt(0, 0.0, 0.0)
   {
   }

   CAnt(int startVertex, double alpha, double beta)
      : mAlpha(alpha)
      , mBeta(beta)
      , mPathLen(0.0)
   {
      mPath.push_back(startVertex);
   }

   void reset()
   {
       *this = CAnt(mPath[0], mAlpha, mBeta);
   }

   /// @brief ant performs one single step to next vertex
   /// @param matrix adjancency matrix of a graph
   /// @return true if ant has not finished yet
   bool makeStep(const CAdjancencyMatrix& matrix)
   {
      std::vector<std::pair<int, double>> probEdges;
      const int startVertex = mPath.back();

      probEdges = calcEdgesProbability(matrix, startVertex);
      double sumAllEdges = 0.0;
      for (auto& item : probEdges) {
         sumAllEdges += item.second;
      }
      for (auto& item : probEdges) {
         item.second /= sumAllEdges;
      }

      const int nextVertex = pickEdge(probEdges);
      if (nextVertex == -1)
      {
         return false;
      }
      mPathLen += matrix.mData[startVertex][nextVertex].mDistance;
      mPath.push_back(nextVertex);

      if (mPath.size() == matrix.mVCount)
      {
         mPathLen += matrix.mData[mPath.back()][mPath.front()].mDistance;
         mPath.push_back(mPath.front());
      }

      return (mPath.size() < matrix.mVCount);
   }

   void applyFeromone(CAdjancencyMatrix& matrix, double Lmin)
   {
      const double feromoneDelta = Lmin / mPathLen;

      for (int i = 1; i < mPath.size(); ++i)
      {
         auto& edge = matrix.mData[mPath[i - 1]][mPath[i]];
         edge.mFeromone = edge.mFeromone + feromoneDelta;

         // for assimetric graph this is separate edge
         if (!matrix.mIsDirected) {
             auto& edgeMirror = matrix.mData[mPath[i]][mPath[i - 1]];
             edgeMirror.mFeromone = edgeMirror.mFeromone + feromoneDelta;
         }
      }
   }

   /// @brief calculate probabilities for all edges coming from 'i'
   /// @param matrix adjancency matrix of a graph
   /// @param i source vertex
   /// @return probability for every edge from 'i'th vertex
   std::vector<std::pair<int, double>> calcEdgesProbability(const CAdjancencyMatrix& matrix, int i)
   {
      // prob = (feromone[i->j]^alpha * weight[i->j]^beta) / sum(feromone[i->k]^alpha * weight[i->k]^beta), k -> [every vertex has edge with 'i-th']

      std::vector<std::pair<int, double>> prob;
      if (i < matrix.mVCount)
      {
         for (int k = 0; k < matrix.mData[i].size(); ++k)
         {
            auto& item = matrix.mData[i][k];
            // edge must exist and vertex is not visited yet
            if ((item.mDistance != matrix.mNoConnectionValue) && std::none_of(mPath.begin(), mPath.end(), [k](auto elem) {return elem == k;}))
            {
               prob.push_back({ k, std::pow(item.mFeromone, mAlpha) * std::pow(1.0 / item.mDistance, mBeta) });
            }
         }
      }
      return prob;
   }

   /// @brief randomly picks the egde
   /// @param prob all possible edges with theirs probabilities
   /// @return vertex ID to move ant to
   int pickEdge(const std::vector<std::pair<int, double>>& prob)
   {
      CRandValue rangeGen(0, 10000);
      const double rangeVal = rangeGen() / 10000.0;

      double edgeStart = 0.0;
      for (int edgeIdx = 0; edgeIdx < prob.size(); ++edgeIdx)
      {
         auto& item = prob[edgeIdx];
         if (rangeVal <= (edgeStart + item.second) || (edgeIdx == prob.size() - 1)) // found by probability or just a last item
         {
            return item.first;
         }
         edgeStart += item.second;
      }
      return -1;
   }

   /// @brief just a print path into console
   /// @param prefix string print before the path
   void printPath(const std::string& prefix) const
   {
      std::cout << prefix << "[" << mPathLen << "] => path[";
      for (auto v : mPath)
      {
         std::cout << v << " ";
      }
      std::cout << "]\n";
   }

   double mAlpha;
   double mBeta;
   /// @brief path through graph
   std::vector<int> mPath;
   /// @brief total lenght of the mPath
   double mPathLen;
};
