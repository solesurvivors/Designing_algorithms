#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "settingsdialog.h"
#include "cplayground.h"
#include "cplaygroundcalculator_alphabeta.h"

#include <memory>
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
    const int result[] = {2, 4, 6};
    return result[settings.getComplexity()];
}

int getSolutionsToCalcCount(const AppSettings& settings)
{
    const int result[] = {5, 5, 5};
    return result[settings.getComplexity()];
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
    ui->textEdit_NodesPerLevel->hide();
    ui->label_4->hide();

    mPlayerCalculatorTimer = new QTimer(this);
    connect(mPlayerCalculatorTimer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::onCalculatorTimerUpdate));
}

MainWindow::~MainWindow()
{
    delete mPlayerCalculatorTimer;
    delete ui;
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
    onCalculatorTimerUpdate();

    if (mPlayground) {
        if (mPlayground->isMoveValid(mCurrentPlayer, {startIdx, endIdx})) {
            // change the playground
            mPlayground->makeMove(mCurrentPlayer, {startIdx, endIdx});

            // switch the player
            mCurrentPlayer = mPlayground->getNextPlayerId(mCurrentPlayer);

            // update opponent if this is PC player:
            if (mPlayerCalculator[mCurrentPlayer] != nullptr) {
                mPlayerCalculator[mCurrentPlayer]->opponentMoves({startIdx, endIdx});
            }

            // infor playground widget about next players turn
            ui->playgroundWidget->setPlayground(mPlayground, &mSettings, mCurrentPlayer);
            // update player name on the screen
            updatePlayerLabel("");

            // check if current player still can make a move
            const auto& moves = mPlayground->getAvailableMoves(mCurrentPlayer);
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
        updatePlayerLabel(QString(", thinking..."));

        const auto& nodes = mPlayerCalculator[mCurrentPlayer]->getNodesProcessed();
        QString text;
        for (int i=0; i<nodes.size() && (nodes[i] > 0); ++i) {
            text += QString::number(i) + " : " + QString::number(nodes[i]) + "\n";
        }
        ui->textEdit_NodesPerLevel->setText(text);
    }
}

void MainWindow::on_btnRestart_clicked()
{
    mCurrentPlayer = 0;

    mPlayground = std::make_shared<CPlayground>(mSettings.getPlaygroundSize());
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
            //mPlayerCalculator[i].reset(new CPlaygroundCalculator_AlphaBeta(i, mPlayground, getMovesToCalcCount(mSettings), getSolutionsToCalcCount(mSettings)));
            mPlayerCalculator[i].reset(new CPlaygroundCalculator_AlphaBeta(i, mPlayground, getMovesToCalcCount(mSettings), getSolutionsToCalcCount(mSettings)));
        }
    }

    if (!mSettings.isHumanPlayer(mCurrentPlayer)) {
        // human should not touch the playground
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


void MainWindow::on_btnSettings_clicked()
{
    auto settings = std::make_unique<SettingsDialog>(this, mSettings);
    if (settings) {
        if (settings->exec() == QDialog::Accepted) {
            mSettings = settings->getSettings();
        }
    }
}

