#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <vector>
#include "Personazh.h"

QT_BEGIN_NAMESPACE
class QPushButton;
class QComboBox;
class QLabel;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadFile();
    void loadJSON();
    void saveToJson();
    void onTableDoubleClicked(int row, int column);
    void removePersonazh(Personazh* p);
    void editCell(int row, int column);

private:
    QTableWidget* table;
    QPushButton* btnLoad;
    QPushButton* btnLoadJSON;
    QPushButton* btnSaveJson;
    QComboBox* imageModeCombo;
    std::vector<Personazh*> persons;

    void setupUI();
    void parseFile(const QString& filename);
    void parseJSON(const QString& filename);
    void refreshTable();
    Personazh* createFromLine(const QString& line);
};

#endif // MAINWINDOW_H