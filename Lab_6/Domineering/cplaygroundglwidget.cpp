#include "cplaygroundglwidget.h"
#include "appsettings.h"
#include "mainwindow.h"

#include <QPainter>
#include <QMouseEvent>

namespace cfg {
    const int dotSize    = 15;
    const int dotHlDelta =  3;
}


CPlaygroundGLWidget::CPlaygroundGLWidget(QWidget* owner)
    : QOpenGLWidget()
    , mReceiver(nullptr)
    , mHighlightedNodeStart(std::numeric_limits<int>::max())
    , mHighlightedNodeEnd(std::numeric_limits<int>::max())
{
    setFixedSize(400, 400);
    setAutoFillBackground(false);

    mBackground = QBrush(QColor(64, 32, 64));

    mCircleBrush = QBrush(Qt::gray);
    mCirclePen   = QPen(Qt::black);
    mCirclePen.setWidth(1);

    mCircleBrushHl = QBrush(Qt::gray);
    mCirclePenHl   = QPen(Qt::black);
    mCirclePenHl.setWidth(1);
}

void CPlaygroundGLWidget::setEventReceiver(QObject* receiver)
{
    mReceiver = receiver;
}

void CPlaygroundGLWidget::setPlayground(std::shared_ptr<CPlayground> playground, AppSettings* settings, int currentPlayer)
{
    mPlayground  = playground;
    mAppSettings = settings;
    mPlayerIdx   = currentPlayer;

    if (mPlayground && (mNodes.size() != (mPlayground->getSize()*mPlayground->getSize())))
    {
        mNodes.resize(0);

        const int tableCells = mPlayground->getSize();
        const auto widgetSize = this->size();
        const int hDelta = widgetSize.width()/(tableCells);
        const int vDelta = widgetSize.height()/(tableCells);
        if (mNodes.empty()) {
            for(int y = vDelta/2; y < widgetSize.height(); y += vDelta) {
                for(int x = hDelta/2; x < widgetSize.width(); x += hDelta) {
                    mNodes.push_back(SNode{QRect(x-cfg::dotSize/2, y-cfg::dotSize/2, cfg::dotSize, cfg::dotSize)});
                }
            }
        }
    }

    repaint();
}

void CPlaygroundGLWidget::paintEvent(QPaintEvent *event)
{
    if (mPlayground == nullptr) {
        return;
    }

    QPainter painter;
    painter.begin(this);

    painter.fillRect(rect(), mBackground);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(mCircleBrushHl);
    painter.setPen(mCirclePenHl);

    // highlighted from node
    if (mHighlightedNodeStart < mNodes.size()) {
        QRect rect = mNodes[mHighlightedNodeStart].screenRect;
        rect.adjust(-cfg::dotHlDelta, -cfg::dotHlDelta, cfg::dotHlDelta, cfg::dotHlDelta);
        painter.drawEllipse(rect);
    }
    // highlighted to node
    if (mHighlightedNodeEnd < mNodes.size()) {
        QRect rect = mNodes[mHighlightedNodeEnd].screenRect;
        rect.adjust(-cfg::dotHlDelta, -cfg::dotHlDelta, cfg::dotHlDelta, cfg::dotHlDelta);
        painter.drawEllipse(rect);
    }

    // already performed moves
    for (int i=0; i<mPlayground->getPlayersCount(); ++i) {
        const auto& moves = mPlayground->getMoves(i);
        QColor colorPlayer = mAppSettings->getPlayerColor(i);
        QBrush lineBrush = QBrush(colorPlayer);
        QPen   linePen   = QPen(colorPlayer);
        linePen.setWidth(5);

        painter.setBrush(lineBrush);
        painter.setPen(linePen);
        for (auto& move: moves) {
            painter.drawLine(mNodes[move.first].screenRect.center(), mNodes[move.second].screenRect.center());
        }
    }

    // regular nodes (drawn on top of 2 highlighted nodes and lines for already performed moves)
    painter.setBrush(mCircleBrush);
    painter.setPen(mCirclePen);
    for (auto& node: mNodes) {
        painter.drawEllipse(node.screenRect);
    }

    // draw the line for potential move of a player
    if ((mHighlightedNodeStart < mNodes.size()) && (mHighlightedNodeEnd < mNodes.size())) {
        QColor colorPlayer = mAppSettings->getPlayerColor(mPlayerIdx);
        QBrush lineBrush = QBrush(colorPlayer);
        QPen   linePen   = QPen(colorPlayer);
        linePen.setWidth(5);

        painter.setBrush(lineBrush);
        painter.setPen(linePen);

        painter.drawLine(mNodes[mHighlightedNodeStart].screenRect.center(), mNodes[mHighlightedNodeEnd].screenRect.center());
    }

    painter.end();
}

namespace {
std::tuple<int, int> findMoveRect(const QPoint& pos, const std::vector<CPlayground::tMoveCandidate>& moves, const std::vector<SNode>& nodes)
{
    std::tuple<int, int> result = {int(nodes.size()), int(nodes.size())};

    for (auto& move: moves) {
        const QRect edgeRect = nodes[move.first].screenRect.united(nodes[move.second].screenRect);
        if (edgeRect.contains(pos)) {
            result = {move.first, move.second};
            break;
        }
    }

    return result;
}
}

void CPlaygroundGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (mPlayground && event) {
        const auto playerMoves = mPlayground->getAvailableMoves(mPlayerIdx);
        std::tie(mHighlightedNodeStart, mHighlightedNodeEnd) = findMoveRect(event->pos(), playerMoves, mNodes);

        repaint();
    }
}

void CPlaygroundGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (mPlayground && mReceiver && event) {
        const int vFrom = mHighlightedNodeStart;
        const int vTo   = mHighlightedNodeEnd;

        mHighlightedNodeStart = mNodes.size();
        mHighlightedNodeEnd   = mNodes.size();

        // send event only if move is valid.
        if (mPlayground->isMoveValid(mPlayerIdx, {vFrom, vTo})) {
            QCoreApplication::sendEvent(mReceiver, new CMyTurnEvent(vFrom, vTo));
        }
    }
}
