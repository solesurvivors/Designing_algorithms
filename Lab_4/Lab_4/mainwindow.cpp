#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "CAdjancencyMatrix.hpp"
#include "CAnt.hpp"

#include <QChart>
#include <QLayout>

#include <iostream>

namespace cfg
{
/// @brief amount of vertices in graph
const int  vertexCount   = 200;
const bool directedGraph = true;
/// @brief range to roll random edge lenght from
const std::pair<int, int> distanceRange = {1, 40};
/// @brief amount of ants
const int antsCount = 45;
/// @brief amount of wild ants
const int antsWildCount = 15;
/// @brief max amount of iterations
const int maxGenerations = 1000;
/// @brief feromones weight
const double alpha = 2.0;
/// @brief distance weight
const double beta = 4.0;
/// @brief feromones evaporation
const double ro = 0.7;

/// @brief amount of elite ants which runs along best path in generation
//const int antsEliteCount = 0;



const std::string pathDump = "./";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , workerThread()
    , mIsJobRunning(false)
    , mDoCancelJob(false)
    , mMatrix(std::make_shared<CAdjancencyMatrix>(cfg::vertexCount, cfg::directedGraph))
    , mLMin(std::numeric_limits<int>::max())
    , mGenAntSeries(new QLineSeries)
    , mLMinSeries(new QLineSeries)
    , mChart(new QChart)
    , mChartView(nullptr)
{
    ui->setupUi(this);

    ui->chartWidget->installEventFilter(this);

    mChart->legend()->hide();
    mChart->addSeries(mGenAntSeries);
    mChart->addSeries(mLMinSeries);
    mChart->createDefaultAxes();

    mChartView = new QChartView(mChart, ui->chartWidget);
    mChartView->setRenderHint(QPainter::Antialiasing);

    Worker *worker = new Worker;
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &MainWindow::operate, worker, &Worker::doWork);
    connect(worker, &Worker::resultReady, this, &MainWindow::handleResults);
    workerThread.start();
}

MainWindow::~MainWindow()
{
    workerThread.quit();
    workerThread.wait();

    delete ui;
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::Resize && object == ui->chartWidget && mChartView)
        mChartView->resize(ui->chartWidget->size());
    return QObject::eventFilter(object, event);
}

void MainWindow::on_pushButton_clicked()
{
    if (mIsJobRunning) {
        ui->pushButton->setEnabled(false);
        mDoCancelJob = true;
        return;
    }

    ui->pushButton->setText("Stop");

    const std::string fileName = cfg::pathDump +
                                 "lr4_matrix_" + std::to_string(cfg::vertexCount) +
                                 "_" + std::to_string(cfg::distanceRange.first) + "_" + std::to_string(cfg::distanceRange.second) +
                                 "_" + std::to_string(int(cfg::directedGraph)) + ".txt";
    const bool build_matrix = !mMatrix->load(fileName);
    if (build_matrix) {
        std::cout << "generate graph with size " << cfg::vertexCount << std::endl;
        mMatrix->generate({ cfg::vertexCount, cfg::vertexCount }, cfg::distanceRange); // vertex-power-range
        mMatrix->save(fileName);
    }

    mGenAntSeries->clear();
    mLMinSeries->clear();

    // initiate first step
    SState state;
    state.genNumber = 0;
    state.matrix = mMatrix;
    state.LMin = std::numeric_limits<int>::max();

    state.ants.reserve(cfg::antsCount);
    std::vector<int> whiteList;
    whiteList.resize(cfg::vertexCount);
    for (int i=0; i<cfg::vertexCount; ++i) {
        whiteList[i] = i;
    }

    for (int antId=0; antId<cfg::antsCount; ++antId) {
        CRandValue startVertexGen(0, whiteList.size() - 1);
        const int startVertex = whiteList[startVertexGen()];
        auto it = std::find(whiteList.begin(), whiteList.end(), startVertex);
        whiteList.erase(it);

        if (antId < cfg::antsWildCount)
        {
            state.ants.push_back(CAnt(startVertex, 0.0, 0.0)); // wild ant? - ignoring everything
        }
        else
        {
            state.ants.push_back(CAnt(startVertex, cfg::alpha, cfg::beta)); // regular ant
        }
    }

    mIsJobRunning = true;
    operate(state);
}

void MainWindow::handleResults(SState state)
{
    mChart->setTitle("Iteration " + QString::number(state.genNumber));

    QString bestPathStr;
    if (state.bestPath.mPath.size() > 1) {
        bestPathStr = "[" + QString::number(state.bestPath.mPath[0]);
        for (int i=1; i<state.bestPath.mPath.size(); ++i) {
            bestPathStr += ", " + QString::number(state.bestPath.mPath[i]);
        }
        bestPathStr += "]";
    }
    ui->bestPathTextEdit->setPlainText(QString("Best lenght: ") + QString::number(state.LMin) +
                                       QString("\nBest path: ") + bestPathStr);

    std::cout << "Best length: " << state.LMin << "\nBest path: ";
    if (!state.bestPath.mPath.empty()) {
        std::cout << "[" << state.bestPath.mPath[0];
        for (size_t i = 1; i < state.bestPath.mPath.size(); ++i) {
            std::cout << ", " << state.bestPath.mPath[i];
        }
        std::cout << "]" << std::endl;
    } else {
        std::cout << "No path found" << std::endl;
    }

    mGenAntSeries->append(state.genNumber, state.ants[0].mPathLen);
    mLMinSeries->append(state.genNumber, state.LMin);

    mChart->removeSeries(mGenAntSeries);
    mChart->addSeries(mGenAntSeries);

    mChart->removeSeries(mLMinSeries);
    mChart->addSeries(mLMinSeries);

    mChart->createDefaultAxes();

    if (!mDoCancelJob && state.genNumber < cfg::maxGenerations) {
        // next step
        state.genNumber++;
        for (auto& ant : state.ants) {
            ant.reset();
        }
        operate(state);
    } else {
        mIsJobRunning = false;
        mDoCancelJob  = false;

        ui->pushButton->setText("Start");
        ui->pushButton->setEnabled(true);
    }
}


void Worker::doWork(SState state)
{
    CRandValue startVertexGen(0, cfg::vertexCount - 1);

    if (state.matrix)
    {
        CAdjancencyMatrix& matrix = *state.matrix;

        // run every ant through the graph
        for (int antId=0; antId<cfg::antsCount; ++antId)
        {
            CAnt& ant = state.ants[antId];
            while (ant.makeStep(matrix))
            {
                // ant is walking...
            }
        }

        std::sort(state.ants.begin(), state.ants.end(), [](const CAnt& l, const CAnt& r) { return l.mPathLen < r.mPathLen; });
        const bool newMinFound = (state.ants[0].mPathLen < state.LMin);
        if (newMinFound) {
            state.LMin = state.ants[0].mPathLen;
            state.bestPath = state.ants[0];
        }

        //state.ants[0].printPath(std::string("Best ant was in gen [") + std::to_string(state.genNumber) + "] : ");

        // make current feromones state older
        matrix.updateFeromones(1.0 - cfg::ro);
        // apply feromones
        for (int antId = 0; antId < state.ants.size(); ++antId)
        {
            auto& ant = state.ants[antId];
            if (ant.mPath.size() == cfg::vertexCount + 1)
            {
                ant.applyFeromone(matrix, state.LMin);
            }
        }

        // run prio ants along best path
        // for (int antId = 0; antId < cfg::antsEliteCount; ++antId) {
        //     state.bestPath.applyFeromone(matrix, state.LMin);
        // }
    }

    emit resultReady(state);
}
