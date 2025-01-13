#ifndef DATADIALOG_H
#define DATADIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class DataDialog;
}

class DataDialog : public QDialog {
    Q_OBJECT

public:
    explicit DataDialog(QWidget *parent = nullptr);
    ~DataDialog();
    void setData(const QString &data);

private slots:
    void on_OKButton_clicked();

private:
    Ui::DataDialog *ui;
};

#endif // DATADIALOG_H
