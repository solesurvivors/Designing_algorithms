#ifndef CPLAYGROUND_H
#define CPLAYGROUND_H

#include <vector>

class CPlayground
{
public:
    enum EMoveDirection {
        Horizontal = 0,
        Vertical   = 1,
        NotSet     = 2
    };

    using tMoveCandidate = std::pair<int, int>;
    using tMoveList      = std::vector<tMoveCandidate>;

    explicit CPlayground(int tableSize);

    int getSize() const;

    int getPlayersCount() const;
    int getNextPlayerId(int playerId) const;

    tMoveList& getAvailableMoves(int playerIdx);
    const tMoveList& getAvailableMoves(int playerIdx) const;

    const tMoveList& getMoves(int playerIdx) const;

    bool isMoveValid(int playerIdx, const tMoveCandidate& move) const;

    void makeMove(int playerIdx, const tMoveCandidate& move);

private:

    int mTableSize;

    int                    mPlayersCount;
    /// @brief available moves for every player
    std::vector<tMoveList> mAvailableMoves;
    /// @brief performed moves for every player
    std::vector<tMoveList> mMoves;
};

#endif // CPLAYGROUND_H
