#include "cplaygroundcalculator_est.h"
#include "CRandValue.h"

#include <cassert>

CPlaygroundCalculator_Estimation::CPlaygroundCalculator_Estimation(int playerId, std::shared_ptr<CPlayground> playground, int maxMoves, int maxSolutions)
    : mPlayerIdx(playerId)
    , mPlayground(playground)
    , mMaxMoves(maxMoves)
    , mMaxSolutions(maxSolutions)
    , mRoot(nullptr)
{
    mNodesCreated.resize(100, 0);
}

std::vector<int> CPlaygroundCalculator_Estimation::getNodesProcessed() const
{
    return mNodesCreated;
}

namespace {
void removeMoves(std::vector<CPlayground::tMoveCandidate>& moves, const CPlayground::tMoveCandidate& move)
{
    auto it = std::remove_if(moves.begin(), moves.end(), [move](const CPlayground::tMoveCandidate& item){
        return (item.first == move.first) || (item.second == move.first) || (item.first == move.second) || (item.second == move.second);
    });
    moves.erase(it, moves.end());
}
}

void CPlaygroundCalculator_Estimation::calculateMinMaxTreeRecursive(SNode& node)
{
    mNodesCreated[node.mLevel]++;

    if ((node.mLevel - mCurrentMove->mLevel) >= mMaxMoves) {
        return;
    }

    const bool hasMoves = !node.mState.mPlayersMoves[node.mPlayerIdx].empty();
    if (hasMoves) {
        const int nextPlayerIdx = mPlayground->getNextPlayerId(node.mPlayerIdx);
        // game is not finished yet
        auto& playerMoves = node.mState.mPlayersMoves[node.mPlayerIdx];
        for (auto& moveCandidate : playerMoves) {

            auto nodeCandidate = std::make_unique<SNode>();
            nodeCandidate->mPlayerIdx  = nextPlayerIdx;
            nodeCandidate->mLevel      = node.mLevel + 1;
            nodeCandidate->mState      = node.mState;

            // perform the move, update playground state
            for (auto& moves: nodeCandidate->mState.mPlayersMoves) {
                removeMoves(moves, moveCandidate);
            }
            // estimate the new state
            for (int i=0; i < nodeCandidate->mState.mPlayersMoves.size(); ++i) {
                auto& moves = nodeCandidate->mState.mPlayersMoves[i];
                nodeCandidate->mEstimation += (i == node.mPlayerIdx) ? moves.size() : -moves.size();
            }
            // add to the list of potential moves
            node.mMoves.push_back({moveCandidate, std::move(nodeCandidate)});
        }

        std::sort(node.mMoves.begin(), node.mMoves.end(), [](const SNode::SMove& l, const SNode::SMove& r) {
            return (l.mNode->mEstimation > r.mNode->mEstimation);
        });

        // calculate child nodes with best estimation
        const int maxEstimation = node.mMoves[0].mNode->mEstimation;
        for (int i=0; i<std::min(size_t(mMaxSolutions), node.mMoves.size()); ++i) {
            auto& move = node.mMoves[i];
            if (move.mNode->mEstimation < maxEstimation)
                break; // dont need to get deeper for moves with low level estimation
            calculateMinMaxTreeRecursive(*move.mNode);
        }

        // 1. sort child node list again?
        // 2. what if the node with new max estimation is not calculated yet?

        // update current node estimation based on subtrees are built
    }
}

void CPlaygroundCalculator_Estimation::createMinMaxTree()
{
    mRoot = std::make_shared<SNode>();
    mRoot->mPlayerIdx  = mPlayerIdx;
    mRoot->mEstimation = 0;

    for (int playerIdx = 0; playerIdx < mPlayground->getPlayersCount(); ++playerIdx) {
        mRoot->mState.mPlayersMoves.push_back(mPlayground->getAvailableMoves(playerIdx));
    }

    mCurrentMove = mRoot;
    calculateMinMaxTreeRecursive(*mRoot);
}

void CPlaygroundCalculator_Estimation::updateMinMaxTree(SNode& node)
{
    if (node.mMoves.empty()) {
        calculateMinMaxTreeRecursive(node);
    } else {
        for (int i=0; i<std::min(size_t(mMaxSolutions), node.mMoves.size()); ++i) {
            auto& move = node.mMoves[i];
            updateMinMaxTree(*move.mNode);
        }
    }
}

CPlayground::tMoveCandidate CPlaygroundCalculator_Estimation::makeMove()
{
    if (mRoot == nullptr) {
        createMinMaxTree();
    }

    if (mCurrentMove && mCurrentMove->mMoves.empty()) {
        // if opponent choosen a node what was not calculated - calculate it now.
        // this is possible because of limitation of child nodes to be processed
        calculateMinMaxTreeRecursive(*mCurrentMove);
    }

    if (mCurrentMove && !mCurrentMove->mMoves.empty()) {
        assert(mCurrentMove->mPlayerIdx == mPlayerIdx);

        const int maxEstimation = mCurrentMove->mMoves[0].mNode->mEstimation;
        const int maxEstimationCount = std::count_if(mCurrentMove->mMoves.begin(), mCurrentMove->mMoves.end(), [maxEstimation](const SNode::SMove& item) {
            return (item.mNode->mEstimation == maxEstimation);
        });

        CRandValue moveRandomizer(0, maxEstimationCount-1);
        // pick a random move
        const auto& nextMove = mCurrentMove->mMoves[moveRandomizer()];
        // move ptr to next state of the playground
        mCurrentMove = nextMove.mNode;

        updateMinMaxTree(*mCurrentMove);

        return nextMove.mMove;
    }
    return {};
}

void CPlaygroundCalculator_Estimation::opponentMoves(const CPlayground::tMoveCandidate& move)
{
    if (mCurrentMove) {
        auto it = std::find_if(mCurrentMove->mMoves.begin(), mCurrentMove->mMoves.end(), [&move](const SNode::SMove& item){
            return (item.mMove == move);
        });
        mCurrentMove = it->mNode;
    }
}
