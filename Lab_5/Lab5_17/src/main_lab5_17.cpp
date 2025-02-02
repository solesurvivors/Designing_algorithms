
#include "../../common/CRandValue.hpp"
#include "../../common/CAdjancencyMatrix.hpp"
#include "GreedyAlgorithm.hpp"

#include <iomanip>
#include <iostream>
#include <vector>
#include <string>

namespace cfg
{
   /// @brief amount of vertices in graph
   const int vertexCount = 300;

   const int swarmSize         = 90;
   const int beesCountEmployed = 140;
   const int beesCountOnlooker = 60;
   const int beesCountScout    = 160;

   /// @brief amount of solutions in the pool
   const int solutionsCount    = swarmSize;
   /// @brief how many failed improvements can be done before solution is removed from the pool
   const int maxTrialsCount    = 30;
   /// @brief max amount of iterations before stop
   const int maxGenerations    = 1000;

   /// @brief size of a clique to search for
   const int cliqueSize = 3;

   const std::string pathDump = "./";
}

/// @brief prints the current state of a graph into the file for 'dot'
/// @param fileName path name of a file
/// @see https://graphviz.org/
void printAsDot(const std::string& fileName, const CAdjancencyMatrix& matrix, const std::vector<int>& path);

struct SSolution
{
   std::vector<int> mPath;
   double mFitness = 0.0;
   int mTrialsCount = 0;
};
/// @brief how to sort solutions from best to worst
static auto solutionSortPred = [](const SSolution& l, const SSolution& r) { return (l.mFitness > r.mFitness); };

class CFitnessCalculator
{
public:
   double getMaxValue() const
   {
      return cfg::cliqueSize*(cfg::cliqueSize-1);
   }

   /// @brief max value is N*(N-1) for clique of size N
   /// @param matrix adjancency matrix for the graph
   /// @param path set of vertices
   /// @return fitness value
   double calc(const CAdjancencyMatrix& matrix, const std::vector<int>& path) const
   {
      double fitness = 0.0;

      for (int v1 = 0; v1 < path.size(); ++v1)
      {
         for (int v2 = 0; v2 < path.size(); ++v2)
         {
            if (matrix.mData[path[v1]][path[v2]].mDistance != matrix.mNoConnectionValue)
            {
               fitness += 1.0;
            }
         }
      }

      return fitness;
   }
};

/// @brief remove already visited vertices from the list of candidates
/// @param visitedVertices list of visited vertices
/// @param candidates list of candidates
void removeVisitedVertices(const std::vector<int>& visitedVertices, std::vector<int>& candidates)
{
   auto it = std::remove_if(candidates.begin(), candidates.end(), [&visitedVertices](int candidate) 
      { 
         return (visitedVertices.end() != std::find(visitedVertices.begin(), visitedVertices.end(), candidate)); 
      });
   candidates.erase(it, candidates.end());
}


/// @brief generate random set of vertices
/// @param matrix adjancency matrix of a graph (future purposes)
/// @param solutionSize amount of vertices in solution
/// @param whiteList list of available vertices
/// @return generated solution of size 'solutionSize'
SSolution generateSolution(const CAdjancencyMatrix& matrix, int solutionSize, const std::vector<int>& whiteList)
{
   std::vector<int> candidates = whiteList;
   SSolution solution;

   solution.mPath.resize(solutionSize);
   for (int v = 0; v < solutionSize; ++v)
   {
      CRandValue vertexGen(0, int(candidates.size() - 1));
      solution.mPath[v] = candidates[vertexGen()];
      auto it = std::remove(candidates.begin(), candidates.end(), solution.mPath[v]);
      candidates.erase(it, candidates.end());
   }

   return solution;
}

/// @brief pure random vertices
/// @param matrix adjancency matrix of a graph
/// @param vertexIdx index of vertex in the solution to be replaced by one of the candidates
/// @return list of candidates
std::vector<int> generateVertexCandidates_Random(const CAdjancencyMatrix& matrix, const std::vector<int>& solution, int vertexIdx)
{
   std::vector<int> candidates;
   for (int i = 0; i < matrix.mVCount; ++i) {
      if (i != solution[vertexIdx]) {
         candidates.push_back(i);
      }
   }
   return candidates;
}

/// @brief list of neighbors of all vertices from solution except 'vertexIdx'
/// @param matrix adjancency matrix of a graph
/// @param vertexIdx index of vertex in the solution to be replaced by one of the candidates
/// @return list of candidates
std::vector<int> generateVertexCandidates_neighbours(const CAdjancencyMatrix& matrix, const std::vector<int>& solution, int vertexIdx)
{
   std::vector<int> candidates;
   
   for (int i = 0; i < solution.size(); ++i) {
      if (i != vertexIdx) {
         auto neighbours = matrix.getNeighbours(solution[i]);
         candidates.insert(candidates.end(), neighbours.begin(), neighbours.end());
         // do not remove duplicates. More identical values - higher probability to pick it.
      }
   }
   return candidates;
}

void runEmployedBees(const CAdjancencyMatrix& matrix, const CFitnessCalculator& fitnessCalc, std::vector<SSolution>& solutions)
{
   for (int beeIdx = 0; (beeIdx < cfg::beesCountEmployed) && (beeIdx < solutions.size()); ++beeIdx)
   {
      // solution for bee (copy)
      auto solution = solutions[beeIdx];
      CRandValue vertexGen(0, int(solution.mPath.size() - 1));
      // pick vertex to be replaced
      const int vertexIdxToReplace = vertexGen();
      // find new vertex (euristic)
      std::vector<int> candidates = generateVertexCandidates_neighbours(matrix, solution.mPath, vertexIdxToReplace);

      removeVisitedVertices(solution.mPath, candidates);

      if (!candidates.empty()) {
         CRandValue vertexIdxGen(0, int(candidates.size()) - 1);
         solution.mPath[vertexIdxToReplace] = candidates[vertexIdxGen()];
         // re-calculate fitness value and replace the solution with better one
         solution.mFitness = fitnessCalc.calc(matrix, solution.mPath);
         if (solution.mFitness > solutions[beeIdx].mFitness) {
            solutions[beeIdx] = solution;
            solutions[beeIdx].mTrialsCount = 0;
         }
         else {
            solutions[beeIdx].mTrialsCount++;
         }
      } else {
         solutions[beeIdx].mTrialsCount++;
      }
   }
   std::sort(solutions.begin(), solutions.end(), solutionSortPred);
}

void runOnlookerBees(const CAdjancencyMatrix& matrix, const CFitnessCalculator& fitnessCalc, std::vector<SSolution>& solutions)
{
   for (int beeIdx = 0; (beeIdx < cfg::beesCountOnlooker) && (beeIdx < solutions.size()); ++beeIdx)
   {
      // solution for bee (copy)
      auto solution = solutions[beeIdx];
      if (solution.mTrialsCount > 0)
      {
         CRandValue vertexGen(0, int(solution.mPath.size() - 1));
         // pick vertex to be replaced
         const int vertexIdxToReplace = vertexGen();
         // find new vertex (random)
         std::vector<int> candidates = generateVertexCandidates_Random(matrix, solution.mPath, vertexIdxToReplace);

         removeVisitedVertices(solution.mPath, candidates);

         if (!candidates.empty()) {
            CRandValue vertexIdxGen(0, int(candidates.size()) - 1);
            solution.mPath[vertexIdxToReplace] = candidates[vertexIdxGen()];
            // re-calculate fitness value and replace the solution with better one
            solution.mFitness = fitnessCalc.calc(matrix, solution.mPath);
            if (solution.mFitness > solutions[beeIdx].mFitness) {
               solutions[beeIdx] = solution;
               solutions[beeIdx].mTrialsCount = 0;
            }
            else {
               solutions[beeIdx].mTrialsCount++;
            }
         }
         else {
            solutions[beeIdx].mTrialsCount++;
         }
      }
   }
   std::sort(solutions.begin(), solutions.end(), solutionSortPred);
}

void runScoutBees(const CAdjancencyMatrix& matrix, const CFitnessCalculator& fitnessCalc, std::vector<SSolution>& solutions, std::vector<int>& whiteList)
{
   int runScouts = 0;
   // replace all solutions which are not improved last 'cfg::maxTrialsCount' iterations by new one
   for (int i = 0; (i < solutions.size()) && (runScouts < cfg::beesCountScout); ++i) {
      auto solution = solutions[i];
      if (solution.mTrialsCount >= cfg::maxTrialsCount) {
         solution = generateSolution(matrix, cfg::cliqueSize, whiteList);
         solution.mFitness = fitnessCalc.calc(matrix, solution.mPath);
         solution.mTrialsCount = 0;

         runScouts++;
      }
   }
   std::sort(solutions.begin(), solutions.end(), solutionSortPred);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
int main()
{
   //--------------------------------------------------------------
   // generate random graph of size 'cfg::vertexCount' where all vertices connected to each other
   CAdjancencyMatrix matrix(cfg::vertexCount, false);
   //matrix.generate({ 2, 20 }, { 1, 1 }); // vertex-power-range, edge-weight-range
   
   // object will calculate finess value for solution
   CFitnessCalculator fitnessCalc;
   //--------------------------------------------------------------

   const bool build_matrix = !matrix.load(cfg::pathDump + "lr5_matrix_" + std::to_string(matrix.mVCount) + ".txt");
   // generate matrix and make sure that it contains clique with desired size
   if (build_matrix) 
   {
      std::vector<int> path;
      bool isFound = false;
      do {
         // generate random graph of size 'cfg::vertexCount'
         std::cout << "generate graph of size " << cfg::vertexCount << std::endl;
         matrix = CAdjancencyMatrix(cfg::vertexCount, false);
         matrix.generate({ 2, 30 }, { 1, 1 }); // vertex-power-range, edge-distance-range
         std::cout << "brute force searching of max clique..." << std::endl;
         path = findMaxCliqueBruteForce(matrix);
         isFound = path.size() >= cfg::cliqueSize;
      } while (!isFound);

      matrix.save(cfg::pathDump + "lr5_matrix_" + std::to_string(matrix.mVCount) + ".txt");

      std::cout << "Optimal answer: size[" << path.size() << "] path[";
      for (auto& v : path) {
         std::cout << v << " ";
      }
      std::cout << "]" << std::endl;

      printAsDot(cfg::pathDump + "gen_optimal_click.txt", matrix, path);
   }

   const std::string solutionsFileName = cfg::pathDump + "lr5_start_solutions_" + std::to_string(matrix.mVCount) + std::to_string(cfg::cliqueSize) + ".txt";

   std::vector<SSolution> solutions; // list of initial solutions

   std::ifstream solutionsFile;
   solutionsFile.open(solutionsFileName);

   if (solutionsFile.is_open()) {
      std::string tmp;
      int vertex;
      // read solutions from file
      while (!solutionsFile.eof()) {
         SSolution solution;

         solutionsFile >> tmp >> solution.mFitness >> tmp;
         if (solutionsFile.eof()) break;
         solution.mPath.reserve(cfg::cliqueSize);
         for (int k = 0; k < cfg::cliqueSize; ++k) {
            solutionsFile >> vertex;
            solution.mPath.push_back(vertex);
         }
         if (solution.mPath.size() == cfg::cliqueSize) {
            solutions.push_back(solution);
         }
      }
      // sort it by fitness value
      std::sort(solutions.begin(), solutions.end(), solutionSortPred);
   }

   double bestFitnessValue = 0.0;
   {
      // initial list of starting vertices to generate a solution
      std::vector<int> whiteList;
      for (int i = 0; i < cfg::vertexCount; ++i) {
         whiteList.push_back(i);
      }

      if (solutions.empty()) {

         // it was not loaded from file - generate it and save to file
         std::cout << "generate set of " << cfg::solutionsCount << " random solutions" << std::endl;

         solutions.resize(cfg::solutionsCount);
         // 1. generate random solutions with specific size
         //    need to implement more sophisticated way...
         //    e.g. random first vertex and then n-1 connected to it...
         for (int i = 0; i < cfg::solutionsCount; ++i)
         {
            solutions[i] = generateSolution(matrix, cfg::cliqueSize, whiteList);
            solutions[i].mFitness = fitnessCalc.calc(matrix, solutions[i].mPath);
         }

         // 1.1 sort it by fitness value
         std::sort(solutions.begin(), solutions.end(), solutionSortPred);

         // 1.2 save solutions for next run
         {
            std::ofstream solutionsFile;
            solutionsFile.open(solutionsFileName);

            if (solutionsFile.is_open()) {
               for (int i = 0; i < cfg::solutionsCount; ++i)
               {
                  const auto solution = solutions[i];
                  solutionsFile << "Fitness: " << solution.mFitness << " Subgraph: ";
                  for (int k = 0; k < solution.mPath.size(); ++k) {
                     solutionsFile << solution.mPath[k] << " ";
                  }
                  solutionsFile << std::endl;
               }
            }
         }
      }

      // 3. main loop
      int genIdx = 0;
      for (; genIdx < cfg::maxGenerations; ++genIdx)
      {
         // ... test best solution ...
         if (solutions[0].mFitness == fitnessCalc.getMaxValue()) {
            break; // found!
         }
         // 3.2. run employed bees
         runEmployedBees(matrix, fitnessCalc, solutions);
         // 3.3. run onlooker bees
         runOnlookerBees(matrix, fitnessCalc, solutions);
         // 3.4. run scouts, if available
         runScoutBees(matrix, fitnessCalc, solutions, whiteList);

         if (bestFitnessValue < solutions[0].mFitness) {
            std::cout << "Clique of size " << cfg::cliqueSize
               << " is not found after iteration " << genIdx
               << ". Best solution: fitness[" << solutions[0].mFitness << "/" << fitnessCalc.getMaxValue() << "]" << std::endl;
            bestFitnessValue = solutions[0].mFitness;
         }
      }

      if (fitnessCalc.getMaxValue() == solutions[0].mFitness) {
         std::cout << "Clique of size " << cfg::cliqueSize 
            << " is found after iteration " << genIdx 
            << ": fitness[" << solutions[0].mFitness << "] path[";
      }
      else {
         std::cout << "Clique of size " << cfg::cliqueSize 
            << " is not found after iteration " << genIdx 
            << ". Best solution: fitness[" << solutions[0].mFitness << "/" << fitnessCalc.getMaxValue() << "] path[";
      }
      for (auto& v : solutions[0].mPath) {
         std::cout << v << " ";
      }
      std::cout << "]" << std::endl;

      printAsDot(cfg::pathDump + "gen_bees_click.txt", matrix, solutions[0].mPath);
   }

   return 0;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------

/// @brief prints the current state of a graph into the file for 'graphwiz'
/// @param fileName path name of a file
void printAsDot(const std::string& fileName, const CAdjancencyMatrix& matrix, const std::vector<int>& path)
{
   static const char* header = "graph A {\n"
      "fontname=\"Helvetica,Arial,sans-serif\"\n"
      "node[fontname = \"Helvetica,Arial,sans-serif\"]\n"
      "edge[fontname = \"Helvetica,Arial,sans-serif\"]\n"
      "layout = twopi\n"
      "ranksep = 5;\n"
      "root = CENTER  // establish the (invisible) center node\n"
      "CENTER[style = invis]\n"
      "edge[style = invis]\n"
      ;
   static const char* footer = "}\n";

   std::ofstream outFile;
   outFile.open(fileName);

   if (!outFile.is_open())
      return;

   outFile << header << std::endl;

   outFile << "CENTER -- {\n";
   for (int i = 0; i < matrix.mData.size(); ++i)
   {
      outFile << "v" << i << std::endl;
   }
   outFile << "}\n";
   outFile << "edge[style = solid]\n";
   outFile << "node[shape = circle];\n";

   double maxFeromone = 0.0;
   for (int i = 0; i < matrix.mData.size(); ++i)
   {
      for (int j = 0; j < i; ++j)
      {
         maxFeromone = std::max(maxFeromone, matrix.mData[i][j].mFeromone);
      }
   }

   for (int i = 0; i < matrix.mData.size(); ++i)
   {
      for (int j = 0; j < i; ++j)
      {
         const double edgeWeight = int((matrix.mData[i][j].mFeromone / maxFeromone) * 2);
         if (matrix.mData[i][j].mDistance != matrix.mNoConnectionValue)
         {
            outFile
               << "v" << i << " -- v" << j << " "
               << "["
               << " label=\"" << "(" << matrix.mData[i][j].mDistance << "; " << matrix.mData[i][j].mFeromone << ")" << "\""
               << " fontcolor=\"invis\""
               << " color=\"black\""
               << " penwidth=" << edgeWeight
               << "];"
               << std::endl;
         }
      }
   }

   for (int v1 = 0; v1 < path.size(); ++v1)
   {
      for (int v2 = v1 + 1; v2 < path.size(); ++v2)
      {
         const int i = path[v1];
         const int j = path[v2];
         auto& edge = matrix.mData[i][j];
         const double edgeWeight = int((matrix.mData[i][j].mFeromone / maxFeromone) * 5);
         outFile
            << "v" << i << " -- v" << j << " "
            << "["
            << " label=\"" << "(" << matrix.mData[i][j].mDistance << "; " << matrix.mData[i][j].mFeromone << ")" << "\""
            << " color=\"green\""
            << " fontcolor=\"green\""
            << " penwidth=" << edgeWeight
            << "];"
            << std::endl;
      }
   }

   outFile << footer << std::endl;
   outFile.close();
}
