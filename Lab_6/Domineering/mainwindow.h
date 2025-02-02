#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "appsettings.h"
#include "cplaygroundglwidget.h"

#include <QMainWindow>
#include <QEvent>
#include <QThread>
#include <QTimer>

#include <memory>

class QLabel;
class CPlayground;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class IPlaygroundCalculator;

class CMyTurnEvent : public QEvent
{
public:
    CMyTurnEvent(int vFrom, int vTo)
        : QEvent(getType())
        , mFrom(vFrom)
        , mTo(vTo)
    {
    }

    static QEvent::Type getType() {
        static QEvent::Type type = static_cast<QEvent::Type>(QEvent::registerEventType());
        return type;
    }

    int mFrom;
    int mTo;
};

class MainWindow : public QMainWindow//, public IPlaygroundWidgetCallbacks
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCalculatorTimerUpdate();
    void on_btnRestart_clicked();
    void on_btnSettings_clicked();

private:

    void customEvent(QEvent* event) override;

    void onNewConnection(int startIdx, int endIdx);

    void updatePlayerLabel(const QString& message);

    Ui::MainWindow *ui;

    AppSettings mSettings;
    std::shared_ptr<CPlayground> mPlayground;

    int mCurrentPlayer;

    std::vector<std::shared_ptr<IPlaygroundCalculator>> mPlayerCalculator;

    QThread* mPlayerCalculatorThread;
    QTimer*  mPlayerCalculatorTimer;

};
#endif // MAINWINDOW_H
