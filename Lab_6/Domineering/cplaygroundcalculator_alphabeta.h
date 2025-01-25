#ifndef CPLAYGROUNDCALCULATOR_ALPHABETA_H
#define CPLAYGROUNDCALCULATOR_ALPHABETA_H

#include "IPlaygroundCalculator.h"

#include <memory>
#include <optional>

class CPlaygroundCalculator_AlphaBeta : public IPlaygroundCalculator
{
public:
    CPlaygroundCalculator_AlphaBeta(int playerId, std::shared_ptr<CPlayground> playground, int maxMoves, int maxSolutions);

    std::vector<int> getNodesProcessed() const override;

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
        /// @brief min-max value of a node
        std::optional<int> mMinMax;
        /// @brief alpha-beta value of a node
        std::pair<int, int> mAB;

        /// @brief estimation of a board after one single move to make a prioritized queue
        /// of a nodes to calculate
        int mEstimation = 0;

        struct SMove
        {
            /// @brief last move which leads to this state (performed by previous player)
            CPlayground::tMoveCandidate mMove;
            /// @brief node with new state of the playground after [mMove]
            std::shared_ptr<SNode>      mNode;
        };

        /// @brief all possible moves of [mPlayerIdx] on the current playground
        std::vector<SMove> mMoves;
    };

    void calculateMinMaxTreeRecursive(SNode& node, std::pair<int,int> alphaBetaValue);
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

    std::vector<int> mNodesCreated;
};

#endif // CPLAYGROUNDCALCULATOR_ALPHABETA_H
