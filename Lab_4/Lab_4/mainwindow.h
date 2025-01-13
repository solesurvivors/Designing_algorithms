#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QLineSeries>
#include <QChartView>

#include "CAdjancencyMatrix.hpp"
#include "CAnt.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct SState {
    int genNumber;

    std::shared_ptr<CAdjancencyMatrix> matrix;
    std::vector<CAnt> ants;

    int LMin;
    CAnt bestPath;
};

class Worker : public QObject
{
    Q_OBJECT

public slots:
    void doWork(SState state);

signals:
    void resultReady(SState state);
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool eventFilter(QObject *object, QEvent *event) override;

public slots:
    void handleResults(SState state);
signals:
    void operate(SState state);

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;

    QThread workerThread;
    bool    mIsJobRunning;
    bool    mDoCancelJob;

    std::shared_ptr<CAdjancencyMatrix> mMatrix;
    int mLMin;

    QLineSeries* mGenAntSeries;
    QLineSeries* mLMinSeries;

    QChart*     mChart;
    QChartView* mChartView;
};
#endif // MAINWINDOW_H
