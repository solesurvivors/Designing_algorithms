
#pragma once

#include <random>

/// @brief random integer value generator in interval [minVal, maxVal]. Uniform distribution.
/// copied from:
/// @see https://en.cppreference.com/w/cpp/numeric/random
class CRandValue
{
public:
   CRandValue(int minVal, int maxVal)
      : mMinVal(minVal)
      , mMaxVal(maxVal)
      , gen(rd())
      , distrib(minVal, maxVal)
   {
   }

   int operator()()
   {
      return distrib(gen);
   }

private:
   int mMinVal;
   int mMaxVal;

   std::random_device rd;  // a seed source for the random number engine
   std::mt19937 gen; // mersenne_twister_engine seeded with rd()
   std::uniform_int_distribution<> distrib;
};

/// @brief random integer value generator in interval [minVal, maxVal]. Uniform distribution.
/// copied from:
/// @see https://en.cppreference.com/w/cpp/numeric/random
class CRandValueReal
{
public:
    CRandValueReal(double minVal, double maxVal)
        : mMinVal(minVal)
        , mMaxVal(maxVal)
        , gen(rd())
        , distrib(minVal, maxVal)
    {
    }

    double operator()()
    {
        return distrib(gen);
    }

private:
    double mMinVal;
    double mMaxVal;

    std::random_device rd;  // a seed source for the random number engine
    std::mt19937 gen; // mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> distrib;
};
