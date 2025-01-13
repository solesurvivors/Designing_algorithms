#include "cplayground.h"

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

void removeMoves(std::vector<CPlayground::tMoveCandidate>& moves, int idxFrom, int idxTo)
{
    auto it = std::remove_if(moves.begin(), moves.end(), [idxFrom, idxTo](const CPlayground::tMoveCandidate& item){
        return (item.first == idxFrom) || (item.second == idxFrom) || (item.first == idxTo) || (item.second == idxTo);
    });
    moves.erase(it, moves.end());
}
}

CPlayground::CPlayground(int tableSize, int playersCount)
    : mTableSize(tableSize)
    , mEdges(tableSize*tableSize, false)
    , mPlayersCount(playersCount)
    , mPlayerDirection()
    , mPlayerMoves()
{
    populateOpenMoves(tableSize, mPlayerMoves[EMoveDirection::Horizontal], EMoveDirection::Horizontal);
    populateOpenMoves(tableSize, mPlayerMoves[EMoveDirection::Vertical]  , EMoveDirection::Vertical);
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

CAdjancencyMatrix& CPlayground::getConnections()
{
    return mEdges;
}

const CAdjancencyMatrix& CPlayground::getConnections() const
{
    return mEdges;
}

void CPlayground::setFirstPlayerDirection(EMoveDirection dir)
{
    if (mPlayerDirection.empty()) {
        for(int i=dir; i<EMoveDirection::NotSet; ++i) {
            mPlayerDirection.push_back(EMoveDirection(i));
        }
        for(int i=0; i<dir; ++i) {
            mPlayerDirection.push_back(EMoveDirection(i));
        }
    }
}

CPlayground::EMoveDirection CPlayground::getPlayerDirection(int playerIdx) const
{
    return mPlayerDirection.empty() ? EMoveDirection::NotSet : mPlayerDirection[playerIdx % int(EMoveDirection::NotSet)];
}

std::vector<CPlayground::tMoveCandidate>& CPlayground::getPlayerMoves(CPlayground::EMoveDirection dir)
{
    return mPlayerMoves[dir];
}

const std::vector<CPlayground::tMoveCandidate>& CPlayground::getPlayerMoves(CPlayground::EMoveDirection dir) const
{
    return mPlayerMoves[dir];
}

void CPlayground::addConnection(int idxFrom, int idxTo, int playerIdx)
{
    mEdges.addEdge(idxFrom, idxTo, playerIdx, false);

    removeMoves(getPlayerMoves(Horizontal), idxFrom, idxTo);
    removeMoves(getPlayerMoves(Vertical), idxFrom, idxTo);
}
