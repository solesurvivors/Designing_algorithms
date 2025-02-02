#include "datadialog.h"
#include "ui_datadialog.h"

DataDialog::DataDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DataDialog) {
    ui->setupUi(this);
}

DataDialog::~DataDialog() {
    delete ui;
}

void DataDialog::on_OKButton_clicked() {
    close();
}

void DataDialog::setData(const QString &data) {
    ui->data->setText(data);
}
