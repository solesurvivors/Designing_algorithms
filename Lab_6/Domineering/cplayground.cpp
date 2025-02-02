
#include "cplayground.h"

#include <cassert>

namespace {
void populateOpenMoves(int tableSize, std::vector<CPlayground::tMoveCandidate>& playerMoves, CPlayground::EMoveDirection dir)
{
    const int offset = (dir == CPlayground::Horizontal) ? 1 : tableSize;
    const int yMax   = (dir == CPlayground::Horizontal) ? tableSize : (tableSize - 1);
    const int xMax   = (dir == CPlayground::Horizontal) ? (tableSize - 1) : tableSize;

    for (int i = 0; i < yMax; ++i) {
        for (int j = 0; j < xMax; ++j) {
            const int idx = i*tableSize + j;
            playerMoves.push_back({idx, idx + offset});
        }
    }
}

void removeMoves(CPlayground::tMoveList& moves, const CPlayground::tMoveCandidate& move)
{
    auto it = std::remove_if(moves.begin(), moves.end(), [move](const CPlayground::tMoveCandidate& item){
        return (item.first == move.first) || (item.second == move.first) || (item.first == move.second) || (item.second == move.second);
    });
    moves.erase(it, moves.end());
}
}

CPlayground::CPlayground(int tableSize)
    : mTableSize(tableSize)
    , mPlayersCount(2)
    , mAvailableMoves()
    , mMoves()
{
    mAvailableMoves.resize(mPlayersCount);
    populateOpenMoves(tableSize, mAvailableMoves[0], EMoveDirection::Horizontal);
    populateOpenMoves(tableSize, mAvailableMoves[1], EMoveDirection::Vertical);

    mMoves.resize(mPlayersCount);
}

int CPlayground::getSize() const
{
    return mTableSize;
}

int CPlayground::getPlayersCount() const
{
    return mPlayersCount;
}

int CPlayground::getNextPlayerId(int playerId) const
{
    return (playerId + 1) == mPlayersCount ? 0 : (playerId + 1);
}

CPlayground::tMoveList& CPlayground::getAvailableMoves(int playerIdx)
{
    assert(playerIdx < mAvailableMoves.size());
    return mAvailableMoves[playerIdx];
}

const CPlayground::tMoveList& CPlayground::getAvailableMoves(int playerIdx) const
{
    assert(playerIdx < mAvailableMoves.size());
    return mAvailableMoves[playerIdx];
}

const CPlayground::tMoveList& CPlayground::getMoves(int playerIdx) const
{
    assert(playerIdx < mMoves.size());
    return mMoves[playerIdx];
}

void CPlayground::makeMove(int playerIdx, const tMoveCandidate& move)
{
    mMoves[playerIdx].push_back(move);

    for (auto& moves: mAvailableMoves) {
        removeMoves(moves, move);
    }
}

bool CPlayground::isMoveValid(int playerIdx, const tMoveCandidate& move) const
{
    const auto& moves = mAvailableMoves[playerIdx];
    auto it = std::find(moves.begin(), moves.end(), move);
    return it != moves.end();
}
