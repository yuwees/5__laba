#include "MainWindow.h"
#include "CraftDialog.h"
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QHeaderView>
#include <QLineEdit>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUI();
}

MainWindow::~MainWindow() {
    for (auto p : persons) delete p;
}

void MainWindow::setupUI() {
    QWidget* central = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Верхняя панель с кнопками
    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->setSpacing(10);

    btnLoad = new QPushButton("Загрузить TXT");
    btnLoadJSON = new QPushButton("Загрузить JSON");
    btnSaveJson = new QPushButton("Сохранить JSON");

    btnLoad->setMinimumWidth(120);
    btnLoadJSON->setMinimumWidth(130);
    btnSaveJson->setMinimumWidth(130);

    btnLoad->setStyleSheet("QPushButton { background-color: #87CEEB; color: black; font-weight: bold; padding: 8px 12px; border-radius: 4px; }");
    btnLoadJSON->setStyleSheet("QPushButton { background-color: #87CEEB; color: black; font-weight: bold; padding: 8px 12px; border-radius: 4px; }");
    btnSaveJson->setStyleSheet("QPushButton { background-color: #3CB371; color: white; font-weight: bold; padding: 8px 12px; border-radius: 4px; }");

    QLabel* modeLabel = new QLabel("Режим картинок:");
    modeLabel->setStyleSheet("QLabel { font-weight: bold; }");

    imageModeCombo = new QComboBox();
    imageModeCombo->addItems({"Статичные картинки", "Рандом из папок", "Уникальные"});
    imageModeCombo->setMinimumWidth(160);

    topLayout->addWidget(btnLoad);
    topLayout->addWidget(btnLoadJSON);
    topLayout->addWidget(btnSaveJson);
    topLayout->addSpacing(20);
    topLayout->addWidget(modeLabel);
    topLayout->addWidget(imageModeCombo);
    topLayout->addStretch();

    // Таблица (8 столбцов, без XP)
    table = new QTableWidget(0, 8);
    table->setHorizontalHeaderLabels({"Код", "Имя", "Стихия/Редкость", "Мана/Урон", "ХП", "Шлем", "Кираса", "Сапоги"});

    table->setColumnWidth(0, 50);   // Код
    table->setColumnWidth(1, 100);  // Имя
    table->setColumnWidth(2, 130);  // Стихия/Редкость
    table->setColumnWidth(3, 90);   // Мана/Урон
    table->setColumnWidth(4, 70);   // ХП
    table->setColumnWidth(5, 70);   // Шлем
    table->setColumnWidth(6, 80);   // Кираса
    table->setColumnWidth(7, 80);   // Сапоги

    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table->setAlternatingRowColors(true);
    table->setStyleSheet(
        "QTableWidget { gridline-color: #a0a0a0; }"
        "QTableWidget::item { border: 1px solid #a0a0a0; padding: 6px; }"
        "QHeaderView::section { border: 1px solid #a0a0a0; background-color: #e8e8e8; padding: 8px; font-weight: bold; }"
        );
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(table);
    setCentralWidget(central);

    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadFile);
    connect(btnLoadJSON, &QPushButton::clicked, this, &MainWindow::loadJSON);
    connect(btnSaveJson, &QPushButton::clicked, this, &MainWindow::saveToJson);
    connect(table, &QTableWidget::cellDoubleClicked, this, &MainWindow::onTableDoubleClicked);
    connect(table, &QTableWidget::cellChanged, this, &MainWindow::editCell);

    resize(1000, 500);
    setWindowTitle("NPC Manager");
    setMinimumSize(900, 400);
}

void MainWindow::loadFile() {
    QString filename = QFileDialog::getOpenFileName(this, "Выберите TXT файл", "", "Text files (*.txt)");
    if (filename.isEmpty()) return;

    for (auto p : persons) delete p;
    persons.clear();

    parseFile(filename);
}

void MainWindow::parseFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        Personazh* p = createFromLine(line);
        if (p) persons.push_back(p);
    }
    refreshTable();
}

Personazh* MainWindow::createFromLine(const QString& line) {
    QStringList parts = line.split(',', Qt::SkipEmptyParts);
    if (parts.size() < 8) return nullptr;

    for (QString& part : parts) part = part.trimmed();

    int code = parts[0].toInt();
    QString name = parts[1];
    QString third = parts[2];
    if (third.toLower() == "огонь") third = "Огонь";
    else if (third.toLower() == "вода") third = "Вода";
    else if (third.toLower() == "воздух") third = "Воздух";
    else if (third.toLower() == "земля") third = "Земля";

    int fourth = parts[3].toInt();   // мана или урон
    int health = parts[4].toInt();   // ХП
    Bronya armor(parts[5].toInt(), parts[6].toInt(), parts[7].toInt());

    QString lowerThird = third.toLower();
    if (lowerThird.contains("огонь") || lowerThird.contains("вода") ||
        lowerThird.contains("земля") || lowerThird.contains("воздух") ||
        lowerThird.contains("молния") || lowerThird.contains("лёд")) {
        return new MagP(code, name, third, fourth, health, armor, 0, 0, 0, 0);
    } else {
        return new VragP(code, name, third, fourth, health, armor, 0, 0, 0, 0);
    }
}

void MainWindow::loadJSON() {
    QString filename = QFileDialog::getOpenFileName(this, "Выберите JSON файл", "", "JSON files (*.json)");
    if (filename.isEmpty()) return;

    for (auto p : persons) delete p;
    persons.clear();

    parseJSON(filename);
}

void MainWindow::parseJSON(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть JSON файл");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        QMessageBox::warning(this, "Ошибка", "JSON должен быть массивом");
        return;
    }

    QJsonArray arr = doc.array();
    for (const QJsonValue& val : arr) {
        QJsonObject obj = val.toObject();

        int code = obj["code"].toInt();
        QString name = obj["name"].toString();
        int health = obj["health"].toInt();
        Bronya armor(obj["helmet"].toInt(), obj["cuirass"].toInt(), obj["boots"].toInt());

        QString type = obj["type"].toString();

        if (type == "Mag") {
            QString element = obj["element"].toString();
            if (element.toLower() == "огонь") element = "Огонь";
            else if (element.toLower() == "вода") element = "Вода";
            else if (element.toLower() == "воздух") element = "Воздух";
            else if (element.toLower() == "земля") element = "Земля";
            int mana = obj["mana"].toInt();
            persons.push_back(new MagP(code, name, element, mana, health, armor, 0, 0, 0, 0));
        } else {
            QString rarity = obj["rarity"].toString();
            int damage = obj["damage"].toInt();
            persons.push_back(new VragP(code, name, rarity, damage, health, armor, 0, 0, 0, 0));
        }
    }
    refreshTable();
}

void MainWindow::refreshTable() {
    table->blockSignals(true);
    table->setRowCount(static_cast<int>(persons.size()));
    for (size_t i = 0; i < persons.size(); ++i) {
        Personazh* p = persons[i];
        table->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(p->code)));
        table->setItem(static_cast<int>(i), 1, new QTableWidgetItem(p->name));
        table->setItem(static_cast<int>(i), 2, new QTableWidgetItem(p->getSpecial1()));
        table->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(p->getSpecial2())));
        table->setItem(static_cast<int>(i), 4, new QTableWidgetItem(QString::number(p->health)));
        table->setItem(static_cast<int>(i), 5, new QTableWidgetItem(QString::number(p->armor.helmet)));
        table->setItem(static_cast<int>(i), 6, new QTableWidgetItem(QString::number(p->armor.cuirass)));
        table->setItem(static_cast<int>(i), 7, new QTableWidgetItem(QString::number(p->armor.boots)));
    }
    table->blockSignals(false);
}

void MainWindow::onTableDoubleClicked(int row, int /*column*/) {
    if (row >= 0 && row < static_cast<int>(persons.size())) {
        Personazh* selectedPerson = persons[static_cast<size_t>(row)];
        CraftDialog* dlg = new CraftDialog(selectedPerson, this, imageModeCombo->currentIndex());
        dlg->setAttribute(Qt::WA_DeleteOnClose);

        connect(dlg, &CraftDialog::printClicked, this, [this, selectedPerson]() {
            removePersonazh(selectedPerson);
        });

        dlg->show();
    }
}

void MainWindow::editCell(int row, int column) {
    if (row < 0 || row >= static_cast<int>(persons.size())) return;

    QTableWidgetItem* item = table->item(row, column);
    if (!item) return;

    Personazh* p = persons[static_cast<size_t>(row)];
    QString newValue = item->text();
    bool ok;
    int intValue = newValue.toInt(&ok);

    switch (column) {
    case 0: if (ok) p->code = intValue; break;
    case 1: p->name = newValue; break;
    case 4: if (ok) p->health = intValue; break;
    case 5: if (ok) p->armor.helmet = intValue; break;
    case 6: if (ok) p->armor.cuirass = intValue; break;
    case 7: if (ok) p->armor.boots = intValue; break;
    }
}

void MainWindow::removePersonazh(Personazh* p) {
    auto it = std::find(persons.begin(), persons.end(), p);
    if (it != persons.end()) {
        delete *it;
        persons.erase(it);
        refreshTable();
    }
}

void MainWindow::saveToJson() {
    QString filename = QFileDialog::getSaveFileName(this, "Сохранить JSON", "", "JSON (*.json)");
    if (filename.isEmpty()) return;

    QJsonArray arr;
    for (auto p : persons) {
        arr.append(p->toJson());
    }

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(arr).toJson());
        QMessageBox::information(this, "Успех", "JSON файл сохранён");
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл");
    }
}