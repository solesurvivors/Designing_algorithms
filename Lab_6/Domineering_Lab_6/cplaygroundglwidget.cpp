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

    mNodes.clear();

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

    // edges
    auto graph = mPlayground->getConnections();
    for (int i = 0; i < graph.size(); ++i) {
        for (int j = 0; j < i; ++j) {
            const auto& edge = graph.mData[i][j];
            if (edge.mDistance != graph.mNoConnectionValue) {

                QColor colorPlayer = mAppSettings->getPlayerColor(edge.mDistance);
                QBrush lineBrush = QBrush(colorPlayer);
                QPen   linePen   = QPen(colorPlayer);
                linePen.setWidth(5);

                painter.setBrush(lineBrush);
                painter.setPen(linePen);

                painter.drawLine(mNodes[i].screenRect.center(), mNodes[j].screenRect.center());
            }
        }
    }

    // regular nodes
    painter.setBrush(mCircleBrush);
    painter.setPen(mCirclePen);
    for (auto& node: mNodes) {
        painter.drawEllipse(node.screenRect);
    }

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
        const auto  dir = mPlayground->getPlayerDirection(mPlayerIdx);
        if (dir == CPlayground::NotSet) {
            const auto& playerMoves = mPlayground->getPlayerMoves(CPlayground::Horizontal);
            std::tie(mHighlightedNodeStart, mHighlightedNodeEnd) = findMoveRect(event->pos(), playerMoves, mNodes);
            if (mHighlightedNodeStart >= mNodes.size() ) { // not found
                const auto& playerMoves = mPlayground->getPlayerMoves(CPlayground::Vertical);
                std::tie(mHighlightedNodeStart, mHighlightedNodeEnd) = findMoveRect(event->pos(), playerMoves, mNodes);
            }
        } else {
            const auto& playerMoves = mPlayground->getPlayerMoves(dir);
            std::tie(mHighlightedNodeStart, mHighlightedNodeEnd) = findMoveRect(event->pos(), playerMoves, mNodes);
        }

        repaint();
    }
}

void CPlaygroundGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (mReceiver && event) {
        const int vFrom = mHighlightedNodeStart;
        const int vTo   = mHighlightedNodeEnd;

        mHighlightedNodeStart = mNodes.size();
        mHighlightedNodeEnd   = mNodes.size();

        QCoreApplication::sendEvent(mReceiver, new CMyTurnEvent(vFrom, vTo));
    }
}
