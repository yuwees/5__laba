#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <vector>
#include "Personazh.h"

QT_BEGIN_NAMESPACE
class QPushButton;
class QComboBox;
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
    void onTableRowClicked(int row, int column);
    void removePersonazh(Personazh* p);
    void editCell(int row, int column);
    void refreshTable();
    void toggleEditMode();  // ← новый слот

private:
    QTableWidget* table;
    QPushButton* btnLoad;
    QPushButton* btnLoadJSON;
    QPushButton* btnSaveJson;
    QPushButton* btnEdit;   // ← новая кнопка
    QComboBox* imageModeCombo;
    std::vector<Personazh*> persons;
    int previousHoveredRow;
    bool editMode;          // ← флаг режима редактирования

    void setupUI();
    void parseFile(const QString& filename);
    void parseJSON(const QString& filename);
    Personazh* createFromLine(const QString& line);
};

#endif // MAINWINDOW_H
