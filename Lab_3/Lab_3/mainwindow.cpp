#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datadialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <sstream>
#include <random>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), btree(new BTree) {
    ui->setupUi(this);
    loadDataFromFile("database.txt");
}

MainWindow::~MainWindow() {
    saveDataToFile("database.txt");
    delete ui;
    delete btree;
}

void MainWindow::displayMessage(const QString &message) {
    QMessageBox::information(this, "Information", message);
}

void MainWindow::saveDataToFile(const QString &filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        btree->save(out);
        file.close();
        displayMessage("Data saved successfully.");
    } else {
        displayMessage("Failed to save data.");
    }
}

void MainWindow::loadDataFromFile(const QString &filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        btree->load(in);
        file.close();
        //displayMessage("Data loaded successfully.");
    } else {
        displayMessage("Failed to load data.");
    }
}

void MainWindow::on_addButton_clicked() {
    int key = ui->keyInput->toPlainText().toInt();
    QString data = ui->dataInput->toPlainText();

    if (btree->insert(key, data.toStdString())) {
        displayMessage("Key added successfully.");
    } else {
        QMessageBox msg = QMessageBox();
        msg.setWindowTitle("Error");
        msg.setText("Key already exists");
        msg.setIcon(QMessageBox::Critical);
        msg.exec();
    }
}

void MainWindow::on_editButton_clicked() {
    int key = ui->keyInput->toPlainText().toInt();
    QString data = ui->dataInput->toPlainText();

    if (btree->edit(key, data.toStdString())) {
        displayMessage("Key updated successfully.");
    } else {
        QMessageBox msg = QMessageBox();
        msg.setWindowTitle("Error");
        msg.setText("Key not found");
        msg.setIcon(QMessageBox::Critical);
        msg.exec();
    }
}

void MainWindow::on_deleteButton_clicked() {
    int key = ui->keyInput->toPlainText().toInt();

    if (btree->remove(key)) {
        displayMessage("Key deleted successfully.");
    } else {
        QMessageBox msg = QMessageBox();
        msg.setWindowTitle("Error");
        msg.setText("Key not found");
        msg.setIcon(QMessageBox::Critical);
        msg.exec();
    }
}

void MainWindow::on_searchButton_clicked() {
    int key = ui->keyInput->toPlainText().toInt();
    int comparisons = 0;

    auto result = btree->search(key, comparisons);
    if (result.has_value()) {
        QString message = QString("Key found: %1\nData: %2\nComparisons: %3")
        .arg(key)
            .arg(QString::fromStdString(result->first))
            .arg(comparisons);
        displayMessage(message);
    } else {
        QMessageBox msg = QMessageBox();
        msg.setWindowTitle("Error");
        msg.setText("Key not found");
        msg.setIcon(QMessageBox::Critical);
        msg.exec();
    }
}

/*void MainWindow::on_generateButton_clicked() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(1, 20000);

    for (int i = 0; i < 10000; ++i) {
        int randomKey = dist(rng);
        QString randomData = QString("Data%1").arg(randomKey);
        btree->insert(randomKey, randomData.toStdString());
    }
    displayMessage("10 000 random nodes generated.");
}*/

void MainWindow::on_generateButton_clicked() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> keyDist(1, 20000);
    std::uniform_int_distribution<int> charDist(0, 51);

    const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    auto generateRandomString = [&]() -> std::string {
        std::string randomString;
        for (int i = 0; i < 10; ++i) {
            randomString += alphabet[charDist(rng)];
        }
        return randomString;
    };

    for (int i = 0; i < 10000; ++i) {
        int randomKey = keyDist(rng);
        std::string randomData = generateRandomString();
        btree->insert(randomKey, randomData);
    }

    displayMessage("10 000 random nodes generated.");
}


void MainWindow::on_seeDataButton_clicked() {
    std::ostringstream os;
    if (btree) {
        btree->save(os);
    }
    QString data = QString::fromStdString(os.str());

    DataDialog dlg(this);
    dlg.setData(data);
    dlg.exec();
}

void MainWindow::on_deleteAllButton_clicked() {
    if (btree) {
        delete btree;
        btree = new BTree();
        displayMessage("All records have been deleted.");
    }
}
