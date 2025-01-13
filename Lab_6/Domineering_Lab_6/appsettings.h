#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QColor>

class AppSettings
{
public:
    enum EComplexity {
        Easy,
        Normal,
        Expert
    };

    enum EGameMode {
        Human_PC,
        Human_Human,
        PC_PC
    };

    AppSettings();

    int getMaxPlayersCount() const;

    void setPlaygroundSize(int size);
    int getPlaygroundSize() const;

    void setPlayersCount(int playersCount);
    int getPlayersCount() const;

    void setComplexity(EComplexity level);
    EComplexity getComplexity() const;

    void setGameMode(EGameMode mode);
    EGameMode getGameMode() const;

    void setHumanTurnsFirst(bool humanFirst);
    bool getHumanTurnsFirst() const;

    QColor getPlayerColor(int playerIdx) const;
    bool   isHumanPlayer (int playerIdx) const;

private:

    void updatePlayers();

    struct SPlayer {
        QColor color;
        bool   isHuman;
    };

    int                  mPlaygroundSize;
    EComplexity          mComplexity;
    EGameMode            mGameMode;
    bool                 mHumanTurnsFirst;
    int                  mPlayersCount;
    std::vector<SPlayer> mPlayerSettings;
};

#endif // APPSETTINGS_H
