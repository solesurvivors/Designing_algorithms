#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent, const AppSettings& settings)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , mSettings(settings)
{
    ui->setupUi(this);

    ui->cbPlaygroundSize->addItem("5");
    ui->cbPlaygroundSize->addItem("6");
    ui->cbPlaygroundSize->addItem("7");
    ui->cbPlaygroundSize->addItem("8");
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
    ui->cbPlaygroundSize->setCurrentText(QString::number(mSettings.getPlaygroundSize()));

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

    const int playgroundSize = ui->cbPlaygroundSize->currentText().toInt();
    if ((playgroundSize > 0) && (playgroundSize < 100)) {
        mSettings.setPlaygroundSize(playgroundSize);
    }

    QDialog::accept();
}

AppSettings SettingsDialog::getSettings() const
{
    return mSettings;
}
