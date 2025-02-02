#include "cplaygroundcalculator_alphabeta.h"
#include "CRandValue.h"

#include <cassert>
#include <algorithm>

CPlaygroundCalculator_AlphaBeta::CPlaygroundCalculator_AlphaBeta(int playerId, std::shared_ptr<CPlayground> playground, int maxMoves, int maxSolutions)
    : mPlayerIdx(playerId)
    , mPlayground(playground)
    , mMaxMoves(maxMoves)
    , mMaxSolutions(maxSolutions)
    , mRoot(nullptr)
{
    mNodesCreated.resize(100, 0);
}

std::vector<int> CPlaygroundCalculator_AlphaBeta::getNodesProcessed() const
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

void CPlaygroundCalculator_AlphaBeta::calculateMinMaxTreeRecursive(SNode& node, std::pair<int,int> alphaBetaValue)
{
    mNodesCreated[node.mLevel]++;

    const bool isMax = mPlayerIdx == node.mPlayerIdx;

    const bool doInterrupt = ((node.mLevel - mCurrentMove->mLevel) >= mMaxMoves);

    const bool hasMoves = !node.mState.mPlayersMoves[node.mPlayerIdx].empty();
    if (!doInterrupt && hasMoves) {
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

        node.mMinMax = isMax ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
        node.mAB     = std::make_pair(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

        // calculate child nodes with best estimation
        for (int i=0; i<std::min(size_t(mMaxSolutions), node.mMoves.size()); ++i) {
            auto& move = node.mMoves[i];
            calculateMinMaxTreeRecursive(*move.mNode, node.mAB);

            if (isMax) {
                node.mMinMax = std::max(move.mNode->mMinMax.value(), node.mMinMax.value());
                node.mAB.first = node.mMinMax.value();
                if (node.mAB.first >= alphaBetaValue.second) {
                    break;
                }
            } else {
                node.mMinMax = std::min(move.mNode->mMinMax.value(), node.mMinMax.value());
                node.mAB.second = node.mMinMax.value();
                if (node.mAB.second <= alphaBetaValue.first) {
                    break;
                }
            }
        }
        if (isMax) {
            node.mAB.second = node.mAB.first;
        } else {
            node.mAB.first = node.mAB.second;
        }
    }
    else
    {
        // no next move available. Calculate profit of this leaf.
        node.mMinMax = int(node.mState.mPlayersMoves[mPlayerIdx].size()) - int(node.mState.mPlayersMoves[mPlayerIdx ? 0 : 1].size());
    }
}

void CPlaygroundCalculator_AlphaBeta::createMinMaxTree()
{
    mRoot = std::make_shared<SNode>();
    mRoot->mPlayerIdx  = mPlayerIdx;
    mRoot->mEstimation = 0;

    for (int playerIdx = 0; playerIdx < mPlayground->getPlayersCount(); ++playerIdx) {
        mRoot->mState.mPlayersMoves.push_back(mPlayground->getAvailableMoves(playerIdx));
    }

    mCurrentMove = mRoot;
    calculateMinMaxTreeRecursive(*mRoot, std::make_pair(std::numeric_limits<int>::min(), std::numeric_limits<int>::max()));
}

void CPlaygroundCalculator_AlphaBeta::updateMinMaxTree(SNode& node)
{
    if (mCurrentMove) {
        const bool doInterrupt = ((node.mLevel - mCurrentMove->mLevel) >= mMaxMoves);
        if (!doInterrupt) {
            if (node.mMoves.empty()) {
                calculateMinMaxTreeRecursive(node, node.mAB);
            } else {
                for (int i=0; i<std::min(size_t(mMaxSolutions), node.mMoves.size()); ++i) {
                    auto& move = node.mMoves[i];
                    updateMinMaxTree(*move.mNode);
                }
            }
        }
    }
}

CPlayground::tMoveCandidate CPlaygroundCalculator_AlphaBeta::makeMove()
{
    if (mRoot == nullptr) {
        createMinMaxTree();
    }

    if (mCurrentMove) {
        if (mCurrentMove->mMoves.empty()) {
            // if opponent choosen a node what was not calculated - calculate it now.
            // this could happen because of limitation of child nodes to be processed
            calculateMinMaxTreeRecursive(*mCurrentMove, std::make_pair(std::numeric_limits<int>::min(), std::numeric_limits<int>::max()));
        }

        if (!mCurrentMove->mMoves.empty()) {
            assert(mCurrentMove->mPlayerIdx == mPlayerIdx);

            // sort by minmax
            auto& moves = mCurrentMove->mMoves;
            std::sort(moves.begin(), moves.end(), [](const SNode::SMove& l, const SNode::SMove& r) {
                return (l.mNode->mMinMax.value_or(std::numeric_limits<int>::min()) > r.mNode->mMinMax.value_or(std::numeric_limits<int>::min()));
            });

            if (moves[0].mNode->mMinMax.has_value()) {
                const int maxMinMax = moves[0].mNode->mMinMax.value();
                const int maxMinMaxCount = std::count_if(mCurrentMove->mMoves.begin(), mCurrentMove->mMoves.end(), [maxMinMax](const SNode::SMove& item) {
                    return item.mNode->mMinMax.has_value() && (item.mNode->mMinMax.value() == maxMinMax);
                });
                CRandValue moveRandomizer(0, maxMinMaxCount-1);
                // pick a random move
                const auto& nextMove = mCurrentMove->mMoves[moveRandomizer()];
                // move ptr to next state of the playground
                mCurrentMove = nextMove.mNode;

                updateMinMaxTree(*mCurrentMove);

                return nextMove.mMove;
            }
        }
    }
    return {};
}

void CPlaygroundCalculator_AlphaBeta::opponentMoves(const CPlayground::tMoveCandidate& move)
{
    if (mCurrentMove) {
        auto it = std::find_if(mCurrentMove->mMoves.begin(), mCurrentMove->mMoves.end(), [&move](const SNode::SMove& item){
            return (item.mMove == move);
        });
        mCurrentMove = it->mNode;
    }
}
