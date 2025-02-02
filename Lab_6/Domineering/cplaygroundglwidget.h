#ifndef CPLAYGROUNDGLWIDGET_H
#define CPLAYGROUNDGLWIDGET_H

#include "cplayground.h"

#include <QObject>
#include <QOpenGLWidget>
#include <QWidget>
#include <QPen>
#include <QBrush>

class AppSettings;

struct SNode {
    QRect screenRect;
};

class CPlaygroundGLWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    CPlaygroundGLWidget(QWidget *parent);

    void setEventReceiver(QObject* receiver);
    void setCallbacks();
    void setPlayground(std::shared_ptr<CPlayground> playground, AppSettings* settings, int currentPlayer);

protected:

    void paintEvent(QPaintEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:

    QObject* mReceiver;
    std::shared_ptr<CPlayground> mPlayground;
    AppSettings*                 mAppSettings;
    int                          mPlayerIdx;

    std::vector<SNode> mNodes;

    QBrush mBackground;

    QBrush mCircleBrush;
    QPen   mCirclePen;

    QBrush mCircleBrushHl;
    QPen   mCirclePenHl;

    int    mHighlightedNodeStart;
    int    mHighlightedNodeEnd;
};

#endif // CPLAYGROUNDGLWIDGET_H
