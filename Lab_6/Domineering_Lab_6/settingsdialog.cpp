#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent, const AppSettings& settings)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , mSettings(settings)
{
    ui->setupUi(this);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::setVisible(bool visible)
{
    QRadioButton* complexityBtn = nullptr;
    switch(mSettings.getComplexity()) {
    case AppSettings::Easy:   complexityBtn = ui->rbComplexityEasy; break;
    case AppSettings::Normal: complexityBtn = ui->rbComplexityNormal; break;
    case AppSettings::Expert: complexityBtn = ui->rbComplexityExpert; break;
    }
    complexityBtn->setChecked(true);

    QRadioButton* gameModeBtn = nullptr;
    switch(mSettings.getGameMode()) {
    case AppSettings::Human_PC:    gameModeBtn = ui->rbGameModeHumanPC; break;
    case AppSettings::Human_Human: gameModeBtn = ui->rbGameModeHumanHuman; break;
    case AppSettings::PC_PC:       gameModeBtn = ui->rbGameModePCPC; break;
    }
    gameModeBtn->setChecked(true);

    ui->cbHumanTurnsFirst->setChecked(mSettings.getHumanTurnsFirst());

    QDialog::setVisible(visible);
}

void SettingsDialog::accept()
{
    AppSettings::EComplexity complexity;
    if (ui->rbComplexityEasy->isChecked()) {
        complexity = AppSettings::Easy;
    }
    else if (ui->rbComplexityNormal->isChecked()) {
        complexity = AppSettings::Normal;
    }
    else if (ui->rbComplexityExpert->isChecked()) {
        complexity = AppSettings::Expert;
    }
    mSettings.setComplexity(complexity);

    AppSettings::EGameMode gameMode;
    if (ui->rbGameModeHumanPC->isChecked()) {
        gameMode = AppSettings::Human_PC;
    }
    else if (ui->rbGameModeHumanHuman->isChecked()) {
        gameMode = AppSettings::Human_Human;
    }
    else if (ui->rbGameModePCPC->isChecked()) {
        gameMode = AppSettings::PC_PC;
    }
    mSettings.setGameMode(gameMode);

    mSettings.setHumanTurnsFirst(ui->cbHumanTurnsFirst->isChecked());

    QDialog::accept();
}

AppSettings SettingsDialog::getSettings() const
{
    return mSettings;
}
