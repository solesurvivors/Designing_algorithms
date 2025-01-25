#include "appsettings.h"

namespace cfg {
const QColor defaultColor = Qt::white;
const int MaxPlayersCount = 2;
}

AppSettings::AppSettings()
    : mComplexity(Easy)
    , mGameMode(Human_PC)
    , mHumanTurnsFirst(true)
    , mPlaygroundSize(8)
    , mPlayersCount(2)
{
    mPlayerSettings = {
        { Qt::red, true},
        { Qt::blue, false}
    };
}

void AppSettings::setPlaygroundSize(int size)
{
    mPlaygroundSize = size;
}

int AppSettings::getPlaygroundSize() const
{
    return mPlaygroundSize;
}

int AppSettings::getPlayersCount() const
{
    return mPlayersCount;
}

AppSettings::EComplexity AppSettings::getComplexity() const
{
    return mComplexity;
}

void AppSettings::setComplexity(EComplexity complexity)
{
    mComplexity = complexity;
}

AppSettings::EGameMode AppSettings::getGameMode() const
{
    return mGameMode;
}

void AppSettings::setGameMode(EGameMode mode)
{
    mGameMode = mode;
    updatePlayers();
}

void AppSettings::setHumanTurnsFirst(bool humanFirst)
{
    mHumanTurnsFirst = humanFirst;
    updatePlayers();
}

bool AppSettings::getHumanTurnsFirst() const
{
    return mHumanTurnsFirst;
}

QColor AppSettings::getPlayerColor(int playerIdx) const
{
    return (playerIdx < mPlayerSettings.size()) ? mPlayerSettings[playerIdx].color : cfg::defaultColor;
}

bool AppSettings::isHumanPlayer(int playerIdx) const
{
    return (playerIdx < mPlayerSettings.size()) && mPlayerSettings[playerIdx].isHuman;
}

void AppSettings::updatePlayers()
{
    switch (mGameMode) {
    case Human_PC:
        mPlayerSettings[0].isHuman = mHumanTurnsFirst;
        mPlayerSettings[1].isHuman = !mHumanTurnsFirst;
        break;
    case Human_Human:
        mPlayerSettings[0].isHuman = true;
        mPlayerSettings[1].isHuman = true;
        break;
    case PC_PC:
        mPlayerSettings[0].isHuman = false;
        mPlayerSettings[1].isHuman = false;
        break;
    }
}
