#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "settingsdialog.h"
#include "cplayground.h"
#include "cplaygroundcalculator.h"

#include <memory>
#include <algorithm>

#include <chrono>

#include <QLabel>
#include <QLayout>

using namespace std::chrono_literals;

namespace {
QString toString(AppSettings::EComplexity complexity)
{
    QString result;
    switch (complexity) {
    case AppSettings::Easy:        result = "Easy";
        break;
    case AppSettings::Normal:      result = "Normal";
        break;
    case AppSettings::Expert:      result = "Expert";
        break;
    }

    return result;
}

QString toString(AppSettings::EGameMode mode)
{
    QString result;
    switch (mode) {
    case AppSettings::Human_PC:    result = "Human vs. PC";
        break;
    case AppSettings::Human_Human: result = "Human vs. Human";
        break;
    case AppSettings::PC_PC:       result = "PC vs. PC";
        break;
    }

    return result;
}

int getMovesToCalcCount(const AppSettings& settings)
{
    int result = 0;
    switch (settings.getComplexity()) {
    case AppSettings::Easy: result=5; break;
    case AppSettings::Normal: result=8; break;
    case AppSettings::Expert: result=10; break;
    }
    return result;
}

int getSolutionsToCalcCount(const AppSettings& settings)
{
    int result = 0;
    switch (settings.getComplexity()) {
    case AppSettings::Easy: result=3; break;
    case AppSettings::Normal: result=3; break;
    case AppSettings::Expert: result=3; break;
    }
    return result;
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mSettings()
    , mPlayground()
    , mCurrentPlayer(0)
    , mPlayerCalculatorThread(nullptr)
    , mPlayerCalculatorTimer(nullptr)
{
    ui->setupUi(this);

    mPlayerCalculatorTimer = new QTimer(this);
    connect(mPlayerCalculatorTimer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::onCalculatorTimerUpdate));
}

MainWindow::~MainWindow()
{
    delete mPlayerCalculatorTimer;
    delete ui;
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionRestart_triggered()
{
    mCurrentPlayer = 0;

    mPlayground = std::make_shared<CPlayground>(mSettings.getPlaygroundSize(), mSettings.getPlayersCount());
    ui->playgroundWidget->setPlayground(mPlayground, &mSettings, mCurrentPlayer);
    ui->playgroundWidget->setEventReceiver(this);
    ui->playgroundWidget->setEnabled(true);

    ui->labelComplexity->setText(toString(mSettings.getComplexity()));
    ui->labelGameMode->setText(toString(mSettings.getGameMode()));

    mPlayerCalculator.clear();
    mPlayerCalculator.resize(mPlayground->getPlayersCount());

    updatePlayerLabel("");

    for (int i=0; i<mPlayerCalculator.size(); ++i) {
        if (!mSettings.isHumanPlayer(i)) {
            mPlayerCalculator[i].reset(new CPlaygroundCalculator_MinMax(i, mPlayground, getMovesToCalcCount(mSettings), getSolutionsToCalcCount(mSettings)));
        }
    }

    if (!mSettings.isHumanPlayer(mCurrentPlayer)) {
        // if PCs first move, always start horizontal
        mPlayground->setFirstPlayerDirection(CPlayground::Horizontal);
        // human does not touch the playground
        ui->playgroundWidget->setEnabled(false);

        // ask PC player to make a move
        auto makeMove = [calc = mPlayerCalculator[mCurrentPlayer], consumer = this]()
        {
            auto move = calc->makeMove();
            if (move != std::pair<int, int>(0, 0)) {
                QCoreApplication::postEvent(consumer, new CMyTurnEvent(move.first, move.second));
            }
        };
        mPlayerCalculatorThread = QThread::create(makeMove);
        mPlayerCalculatorThread->start();

        mPlayerCalculatorTimer->start(1s);
    }
}

void MainWindow::on_actionOptions_triggered()
{
    auto settings = std::make_unique<SettingsDialog>(this, mSettings);
    if (settings) {
        if (settings->exec() == QDialog::Accepted) {
            mSettings = settings->getSettings();
        }
    }
}

void MainWindow::updatePlayerLabel(const QString& message)
{
    auto playerLabel = ui->labelPlayerName;
    if (!message.isEmpty()) {
        playerLabel->setText(QString("Player-")+QString::number(mCurrentPlayer)+": " + message);
    } else {
        playerLabel->setText(QString("Player-")+QString::number(mCurrentPlayer));
    }

    QPalette palette = playerLabel->palette();
    palette.setColor(playerLabel->foregroundRole(), mSettings.getPlayerColor(mCurrentPlayer));
    playerLabel->setPalette(palette);
}

bool MainWindow::isPlayerMoveValid(int playerIdx, int startIdx, int endIdx)
{
    bool result = false;
    if (mPlayground) {
        const int distance = std::abs(startIdx-endIdx);

        if (playerIdx == 0) {
            mPlayground->setFirstPlayerDirection(distance == 1 ? CPlayground::Horizontal : CPlayground::Vertical);
        }

        const auto& moves = mPlayground->getPlayerMoves(mPlayground->getPlayerDirection(mCurrentPlayer));
        auto it = std::find_if(moves.begin(), moves.end(), [startIdx, endIdx](const CPlayground::tMoveCandidate& item){
            return (item.first == startIdx) && (item.second == endIdx) || (item.first == endIdx) && (item.second == startIdx);
        });
        result = it != moves.end();
    }
    return result;
}

void MainWindow::customEvent(QEvent* event)
{
    if (event && (event->type() == CMyTurnEvent::getType())) {
        CMyTurnEvent* turn = static_cast<CMyTurnEvent*>(event);
        onNewConnection(turn->mFrom, turn->mTo);
    }
}

void MainWindow::onNewConnection(int startIdx, int endIdx)
{
    mPlayerCalculatorTimer->stop();

    if (mPlayground) {
        if (isPlayerMoveValid(mCurrentPlayer, startIdx, endIdx)) {

            // change the playground
            mPlayground->addConnection(startIdx, endIdx, mCurrentPlayer);

            // update all other PC players in turns order:
            for (int i=mPlayground->getNextPlayerId(mCurrentPlayer); i!=mCurrentPlayer; i=mPlayground->getNextPlayerId(i)) {
                if (mPlayerCalculator[i] != nullptr) {
                    mPlayerCalculator[i]->opponentMoves({startIdx, endIdx});
                }
            }

            mCurrentPlayer = mPlayground->getNextPlayerId(mCurrentPlayer);
            // update playground
            ui->playgroundWidget->setPlayground(mPlayground, &mSettings, mCurrentPlayer);

            updatePlayerLabel("");

            const auto& moves = mPlayground->getPlayerMoves(mPlayground->getPlayerDirection(mCurrentPlayer));
            if (moves.empty()) {
                updatePlayerLabel(" has no moves. Game over!");
                ui->playgroundWidget->setEnabled(false);

                return;
            }
        } else {
            updatePlayerLabel("Incorrect move!!!");
        }

        const bool isHumanMove = mSettings.isHumanPlayer(mCurrentPlayer);
        ui->playgroundWidget->setEnabled(isHumanMove);

        if (!isHumanMove) {
            // PC makes move
            assert(mPlayerCalculator[mCurrentPlayer] != nullptr);

            // async call
            auto makeMove = [calc = mPlayerCalculator[mCurrentPlayer], consumer = this](){
                auto move = calc->makeMove();
                if (move != std::pair<int, int>(0, 0)) {
                    QCoreApplication::postEvent(consumer, new CMyTurnEvent(move.first, move.second));
                }
            };
            mPlayerCalculatorThread = QThread::create(makeMove);
            mPlayerCalculatorThread->start();

            mPlayerCalculatorTimer->start(1s);
        }
    }
}

void MainWindow::onCalculatorTimerUpdate()
{
    if (mPlayerCalculator[mCurrentPlayer] != nullptr) {
        updatePlayerLabel(QString(", thinking... Nodes processed: " + QString::number(mPlayerCalculator[mCurrentPlayer]->getNodesProcessed())));
    }
}
