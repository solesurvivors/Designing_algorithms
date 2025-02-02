
#pragma once

#include <vector>
#include <fstream>

#include "CRandValue.hpp"

namespace cfg
{
   /// @brief cost (penalty) for picking this edge is huge. It is used to mark missing edges
   static const int hasNoConnection = std::numeric_limits<int>::max();
   /// @brief  default feromone volume for edges
   const std::pair<double, double> defaultFeromoneRange = {0.001, 0.03};
}

class CAdjancencyMatrix
{
public:

   struct SEdge
   {
      SEdge(int weight, double feromone)
         : mDistance(weight)
         , mFeromone(feromone)
      {
      }
      /// @brief weight of an edge
      int mDistance;
      /// @brief feromones status (get back() element for actual status)
      double mFeromone;
   };

   /// @brief ctr
   /// @param vertexCount amount of vertices in the graph
   /// @param directed true if graph is directed
   /// @param noDeadends in case of directed graph, 2 connected vertices are always connected to each other with 2 edges with different lenght.
   /// @param noConnectionValue weight of the edge where is no connection between vertices
   explicit CAdjancencyMatrix(int vertexCount, bool directed, bool noDeadends = true, int noConnectionValue = cfg::hasNoConnection)
      : mVCount(vertexCount)
      , mIsDirected(directed)
      , mHasNoDeadends(noDeadends)
      , mNoConnectionValue(noConnectionValue)
   {
      init(noConnectionValue);
   }

   explicit CAdjancencyMatrix(const std::string& fileName)
      : mVCount(0)
      , mNoConnectionValue(cfg::hasNoConnection)
   {
      load(fileName);
   }

   /// @brief returns size of the graph
   /// @return amount of vertices
   int size() const
   {
      return mVCount;
   }

   void updateFeromones(double ro)
   {
      for (auto& row : mData) {
         for (auto& edge : row) {
            edge.mFeromone *= ro;
         }
      }
   }

   int getVertexPower(int i) const
   {
      return mVertexPower[i];
   }

   std::vector<int> getNeighbours(int vertex) const
   {
      std::vector<int> vertices;
      for (int i = 0; i < mVCount; ++i) {
         if (mData[vertex][i].mDistance != mNoConnectionValue) {
            vertices.push_back(i);
         }
      }
      return vertices;
   }

   bool isClique(std::vector<int>& clique, int vertex) const
   {
      bool res = true;
      for (int i = 0; res && (i < clique.size()); i++)
      {
         res = mData[vertex][clique[i]].mDistance != mNoConnectionValue;
      }
      return res;
   }

   bool isClique(const std::vector<int>& clique) const
   {
      bool res = true;
      for (int i = 0; res && (i < clique.size()); i++)
      {
         for (int j = i + 1; res && (j < clique.size()); ++j) {
            res = mData[clique[i]][clique[j]].mDistance != mNoConnectionValue;
         }
      }
      return res;
   }

   /// @brief generates initial state of the graph with given parameters
   /// @param vertexPowerRange range of values for power of a vertices
   /// @param edgeLenghtRange range of values for edges lenght
   void generate(std::pair<int, int> vertexPowerRange, std::pair<int, int> edgeLenghtRange, std::pair<double, double> feromoneRange = cfg::defaultFeromoneRange)
   {
      CRandValue vertexPowerGen(vertexPowerRange.first, vertexPowerRange.second);
      CRandValue lenghtPowerGen(edgeLenghtRange.first, edgeLenghtRange.second);
      CRandValue hasConnectionPowerGen(0, 1);
      CRandValueReal feromoneGen(feromoneRange.first, feromoneRange.second);

      std::vector<int> vertexPower;
      vertexPower.resize(mVCount);
      for (auto& v : vertexPower) {
         v = vertexPowerGen();
      }

      for (int i = 0; i < mVCount; ++i)
      {
         const int iPower = getVertexPower(i);
         // list of available vertices
         std::vector<int> whiteList;
         if (iPower < vertexPowerRange.second) {
            for (int j = 0; j < mVCount; ++j) {
               if ((j != i) && (mData[i][j].mDistance == mNoConnectionValue) && (getVertexPower(j) < vertexPower[j])) {
                  whiteList.push_back(j);
               }
            }
         }

         const int vertexPowerAdd = vertexPower[i] - iPower; // vertexPower shall be less than whiteList.size()
         for (int j = 0; (j < vertexPowerAdd) && !whiteList.empty(); ++j)
         {
            // pick one random vertex from the white list
            CRandValue edgeGen(0, int(whiteList.size()) - 1);
            const int vIdx = edgeGen();
            const int cVertex = whiteList[vIdx];
            // remove already used vertex from the list
            whiteList.erase(whiteList.begin()+vIdx);
            // create an edge
            auto len = lenghtPowerGen();
            auto feromone = feromoneGen();
            mData[i][cVertex] = { len, feromone };
            if (!mIsDirected) {
               mData[cVertex][i] = { len, feromone };
            } else {
               const int hasConnection = mHasNoDeadends || (hasConnectionPowerGen() != 0);
               if (hasConnection) {
                  auto len_back = lenghtPowerGen();
                  mData[cVertex][i] = { len_back, feromone };
               }
            }

            mVertexPower[i]++;
            mVertexPower[cVertex]++;
         }
      }
   }

   /// @brief saves graph to file
   /// @param fileName name of a file
   void save(const std::string& fileName) const
   {
      std::ofstream saveFile;
      saveFile.open(fileName);

      if (!saveFile.is_open())
         return;

      saveFile << mVCount << std::endl;
      saveFile << mNoConnectionValue << std::endl;

      for (auto pw : mVertexPower) {
         saveFile << pw << " ";
      }
      saveFile << std::endl;

      for (auto& row : mData) {
         for (auto& item : row) {
            saveFile << item.mDistance << " " << item.mFeromone << " | ";
         }
         saveFile << std::endl;
      }
      saveFile << std::endl;

      saveFile.close();
   }

   /// @brief load graph from file
   /// @param fileName name of a file
   /// @return true on success
   bool load(const std::string& fileName)
   {
      std::ifstream saveFile;
      saveFile.open(fileName);

      if (!saveFile.is_open())
         return false;

      saveFile >> mVCount;
      saveFile >> mNoConnectionValue;

      init(mNoConnectionValue);

      for (auto& pw : mVertexPower) {
         saveFile >> pw;
      }

      std::string separator;
      for (auto& row : mData) {
         for (auto& item : row) {
            saveFile >> item.mDistance >> item.mFeromone >> separator;
         }
      }

      saveFile.close();

      return true;
   }

private:

   /// @brief just memory allocation
   /// @param noConnectionValue default value of the distance between 2 vertices where is no edge
   /// @param defaultFeromone default initial value of a feromone
   void init(int noConnectionValue, double defaultFeromone = 0.0)
   {
      mVertexPower.resize(mVCount, 0);
      mData.resize(mVCount);
      for (int i = 0; i < mVCount; ++i)
      {
         mData[i].resize(mVCount, SEdge(noConnectionValue, defaultFeromone));
      }
   }

public:
   /// @brief amount of vertices
   int    mVCount;
   /// @brief true if graph is directed
   bool   mIsDirected;
   /// @brief no dead ends in the graph
   bool   mHasNoDeadends;
   /// @brief value if no edge between vertices
   int    mNoConnectionValue;

   /// @brief power of each vertex. To do not calculate it every single time
   std::vector<int>                mVertexPower;
   /// @brief the adjancency matrix
   /// element [i][j] contains the information about a connection between these nodes such as distance and feromones status
   std::vector<std::vector<SEdge>> mData;
};
