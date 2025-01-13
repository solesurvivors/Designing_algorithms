#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

#include "appsettings.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent, const AppSettings& settings);
    ~SettingsDialog();

    AppSettings getSettings() const;

private:

    void setVisible(bool visible) override;
    void accept() override;

    Ui::SettingsDialog *ui;

    AppSettings mSettings;
};

#endif // SETTINGSDIALOG_H
