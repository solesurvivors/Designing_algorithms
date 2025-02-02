#ifndef IPLAYGROUNDCALCULATOR_H
#define IPLAYGROUNDCALCULATOR_H

#include "cplayground.h"

class IPlaygroundCalculator
{
public:
    virtual ~IPlaygroundCalculator() = default;

    virtual CPlayground::tMoveCandidate makeMove() = 0;
    virtual void opponentMoves(const CPlayground::tMoveCandidate& move) = 0;

    virtual std::vector<int> getNodesProcessed() const = 0;
};

#endif // IPLAYGROUNDCALCULATOR_H
