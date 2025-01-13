#include "cplaygroundcalculator.h"

#include <cassert>

CPlaygroundCalculator_MinMax::CPlaygroundCalculator_MinMax(int playerId, std::shared_ptr<CPlayground> playground, int maxMoves, int maxSolutions)
    : mPlayerIdx(playerId)
    , mPlayground(playground)
    , mMaxMoves(maxMoves)
    , mMaxSolutions(maxSolutions)
    , mRoot(nullptr)
{
}

int CPlaygroundCalculator_MinMax::getNodesProcessed() const
{
    return mNodesCreated;
}

namespace {
int removeMoves(std::vector<CPlayground::tMoveCandidate>& moves, int idxFrom, int idxTo)
{
    int result = 0;
    auto it = std::remove_if(moves.begin(), moves.end(), [idxFrom, idxTo](const CPlayground::tMoveCandidate& item){
        return (item.first == idxFrom) || (item.second == idxFrom) || (item.first == idxTo) || (item.second == idxTo);
    });
    result = std::distance(it, moves.end());
    moves.erase(it, moves.end());

    return result;
}
}

void CPlaygroundCalculator_MinMax::calculateMinMaxTreeRecursive(SNode& node)
{
    mNodesCreated++;

    if ((node.mLevel - mCurrentMove->mLevel) >= mMaxMoves) {
        return;
    }

    const bool hasMoves = !node.mState.mPlayersMoves[node.mPlayerIdx].empty();
    if (hasMoves) {
        const int nextPlayerIdx = node.mPlayerIdx+1 == mPlayground->getPlayersCount() ? 0 : node.mPlayerIdx+1;
        // game is not finished yet
        auto& playerMoves = node.mState.mPlayersMoves[node.mPlayerIdx];
        for (auto& moveCandidate : playerMoves) {

            auto nodeCandidate = std::make_unique<SNode>();
            nodeCandidate->mPlayerIdx  = nextPlayerIdx;
            nodeCandidate->mLevel      = node.mLevel + 1;
            nodeCandidate->mState      = node.mState;
            nodeCandidate->mEstimation = 0;

            for (int i=0; i < nodeCandidate->mState.mPlayersMoves.size(); ++i) {
                removeMoves(nodeCandidate->mState.mPlayersMoves[i], moveCandidate.first, moveCandidate.second);
            }

            for (int i=0; i < nodeCandidate->mState.mPlayersMoves.size(); ++i) {
                auto& moves = nodeCandidate->mState.mPlayersMoves[i];
                nodeCandidate->mEstimation += (i == node.mPlayerIdx) ? moves.size() : -moves.size();
            }

            node.mMoves.push_back({moveCandidate, std::move(nodeCandidate)});
        }

        std::sort(node.mMoves.begin(), node.mMoves.end(), [](const SNode::SMove& l, const SNode::SMove& r) {
            return (l.mNode->mEstimation > r.mNode->mEstimation);
        });
        // calculate child nodes for best estimation
        const int maxEstimation = node.mMoves[0].mNode->mEstimation;
        for (int i=0; i<std::min(size_t(mMaxSolutions), node.mMoves.size()); ++i) {
            auto& move = node.mMoves[i];
            if (move.mNode->mEstimation < maxEstimation)
                break; // dont need to calculate moves with low level estimation
            calculateMinMaxTreeRecursive(*move.mNode);
        }
    }
}

void CPlaygroundCalculator_MinMax::createMinMaxTree()
{
    mRoot = std::make_shared<SNode>();
    mRoot->mPlayerIdx  = mPlayerIdx;
    mRoot->mEstimation = 0;

    for (int playerIdx = 0; playerIdx < mPlayground->getPlayersCount(); ++playerIdx) {
        mRoot->mState.mPlayersMoves.push_back(mPlayground->getPlayerMoves(mPlayground->getPlayerDirection(playerIdx)));
    }

    mCurrentMove = mRoot;
    calculateMinMaxTreeRecursive(*mRoot);
}

void CPlaygroundCalculator_MinMax::updateMinMaxTree(SNode& node)
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

CPlayground::tMoveCandidate CPlaygroundCalculator_MinMax::makeMove()
{
    mNodesCreated = 1;

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

void CPlaygroundCalculator_MinMax::opponentMoves(const CPlayground::tMoveCandidate& move)
{
    if (mCurrentMove) {
        auto it = std::find_if(mCurrentMove->mMoves.begin(), mCurrentMove->mMoves.end(), [&move](const SNode::SMove& item){
            return (item.mMove == move);
        });
        mCurrentMove = it->mNode;
    }
}
