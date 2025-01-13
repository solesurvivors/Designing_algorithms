#ifndef CPLAYGROUND_H
#define CPLAYGROUND_H

#include "CAdjancencyMatrix.h"

class CPlayground
{
public:
    enum EMoveDirection {
        Horizontal = 0,
        Vertical   = 1,
        NotSet     = 2
    };

    using tMoveCandidate = std::pair<int, int>;

    explicit CPlayground(int tableSize, int playersCount);

    int getSize() const;
    CAdjancencyMatrix& getConnections();
    const CAdjancencyMatrix& getConnections() const;

    int getPlayersCount() const;
    int getNextPlayerId(int playerId) const;

    void setFirstPlayerDirection(EMoveDirection dir);
    EMoveDirection getPlayerDirection(int playerIdx) const;

    std::vector<tMoveCandidate>& getPlayerMoves(EMoveDirection dir);
    const std::vector<tMoveCandidate>& getPlayerMoves(EMoveDirection dir) const;

    void addConnection(int idxFrom, int idxTo, int playerIdx);

private:

    int mTableSize;

    // @brief connections
    CAdjancencyMatrix mEdges;

    int                         mPlayersCount;
    // @brief direction of a player
    std::vector<EMoveDirection> mPlayerDirection;
    std::vector<tMoveCandidate> mPlayerMoves[EMoveDirection::NotSet];
};

#endif // CPLAYGROUND_H
