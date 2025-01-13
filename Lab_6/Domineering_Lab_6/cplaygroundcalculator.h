#ifndef CPLAYGROUNDCALCULATOR_H
#define CPLAYGROUNDCALCULATOR_H

#include "cplayground.h"

class IPlaygroundCalculator
{
public:
    virtual ~IPlaygroundCalculator() = default;

    virtual CPlayground::tMoveCandidate makeMove() = 0;
    virtual void opponentMoves(const CPlayground::tMoveCandidate& move) = 0;

    virtual int getNodesProcessed() const = 0;
};


class CPlaygroundCalculator_MinMax : public IPlaygroundCalculator
{
public:

    CPlaygroundCalculator_MinMax(int playerId, std::shared_ptr<CPlayground> playground, int maxMoves, int maxSolutions);

    int getNodesProcessed() const override;

private:

    using tPlayerMoves = std::vector<CPlayground::tMoveCandidate>;

    struct SState
    {
        /// @brief available moves for every player
        std::vector<tPlayerMoves> mPlayersMoves;
    };

    struct SNode
    {
        /// @brief how deep node in the tree is
        int mLevel = 0;
        /// @brief player to make a move
        int mPlayerIdx = 0;

        /// @brief state of the playground
        SState mState;

        int mEstimation = 0;

        struct SMove
        {
            CPlayground::tMoveCandidate mMove;
            std::shared_ptr<SNode>      mNode;
        };

        /// @brief all possible moves of [mPlayerIdx] on the current playground
        std::vector<SMove> mMoves;
    };

    void calculateMinMaxTreeRecursive(SNode& node);
    void createMinMaxTree();
    void updateMinMaxTree(SNode& node);

    CPlayground::tMoveCandidate makeMove() override;
    void opponentMoves(const CPlayground::tMoveCandidate& move) override;

    const int mPlayerIdx;
    std::shared_ptr<CPlayground> mPlayground;
    const int mMaxMoves;
    const int mMaxSolutions;

    /// @brief real root node of entire tree
    std::shared_ptr<SNode> mRoot;
    /// @brief current position in the tree after all moves performed
    std::shared_ptr<SNode> mCurrentMove;

    int mNodesCreated;
};

#endif // CPLAYGROUNDCALCULATOR_H
