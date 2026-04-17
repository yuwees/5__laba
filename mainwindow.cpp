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
#include <QTextCodec>
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
    imageModeCombo->addItems({"Статичные", "Случайные", "Уникальные"});
    imageModeCombo->setMinimumWidth(120);
    
    topLayout->addWidget(btnLoad);
    topLayout->addWidget(btnLoadJSON);
    topLayout->addWidget(btnSaveJson);
    topLayout->addSpacing(20);
    topLayout->addWidget(modeLabel);
    topLayout->addWidget(imageModeCombo);
    topLayout->addStretch();

    // Таблица: 5 столбцов (Имя, Стихия/Редкость, Мана/Урон, ХП, Броня)
    table = new QTableWidget(0, 5);
    table->setHorizontalHeaderLabels({"Имя", "Стихия/Редкость", "Мана/Урон", "ХП", "Броня"});
    
    table->setColumnWidth(0, 120);  // Имя
    table->setColumnWidth(1, 130);  // Стихия/Редкость
    table->setColumnWidth(2, 100);  // Мана/Урон
    table->setColumnWidth(3, 80);   // ХП
    table->setColumnWidth(4, 150);  // Броня
    
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table->setAlternatingRowColors(false);
    table->setStyleSheet(
        "QTableWidget { gridline-color: #a0a0a0; }"
        "QTableWidget::item { border: 1px solid #a0a0a0; padding: 6px; }"
        "QTableWidget::item:hover { background-color: #d0a0ff; }"
        "QTableWidget::item:selected { background-color: #d0a0ff; color: black; }"
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
    connect(table, &QTableWidget::cellEntered, this, &MainWindow::onTableRowSelected);
    connect(table, &QTableWidget::cellChanged, this, &MainWindow::editCell);

    resize(850, 500);
    setWindowTitle("NPC Manager");
    setMinimumSize(750, 400);
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
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    QByteArray data = file.readAll();
    file.close();
    
    // Удаляем BOM, если есть
    if (data.startsWith("\xEF\xBB\xBF")) {
        data.remove(0, 3);
    }
    
    // Пробуем UTF-8
    QString content = QString::fromUtf8(data);
    
    // Если есть нераспознанные символы — пробуем Windows-1251
    if (content.contains(QChar(0xFFFD))) {
        QTextCodec* codec = QTextCodec::codecForName("Windows-1251");
        if (codec) {
            content = codec->toUnicode(data);
        }
    }
    
    QStringList lines = content.split('\n', Qt::SkipEmptyParts);
    
    for (QString line : lines) {
        line = line.trimmed();
        line.remove('\r');
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
    
    int fourth = parts[3].toInt();
    int health = parts[4].toInt();
    Bronya armor(parts[5].toInt(), parts[6].toInt(), parts[7].toInt());

    QString lowerThird = third.toLower();
    if (lowerThird.contains("огонь") || lowerThird.contains("вода") || 
        lowerThird.contains("земля") || lowerThird.contains("воздух") ||
        lowerThird.contains("молния") || lowerThird.contains("лёд")) {
        return new MagP(code, name, third, fourth, health, armor);
    } else {
        return new VragP(code, name, third, fourth, health, armor);
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
            persons.push_back(new MagP(code, name, element, mana, health, armor));
        } else {
            QString rarity = obj["rarity"].toString();
            int damage = obj["damage"].toInt();
            persons.push_back(new VragP(code, name, rarity, damage, health, armor));
        }
    }
    refreshTable();
}

void MainWindow::refreshTable() {
    table->blockSignals(true);
    table->setRowCount(static_cast<int>(persons.size()));
    for (size_t i = 0; i < persons.size(); ++i) {
        Personazh* p = persons[i];
        table->setItem(static_cast<int>(i), 0, new QTableWidgetItem(p->name));
        table->setItem(static_cast<int>(i), 1, new QTableWidgetItem(p->getSpecial1()));
        table->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::number(p->getSpecial2())));
        table->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(p->health)));
        table->setItem(static_cast<int>(i), 4, new QTableWidgetItem(p->armor.toString()));
    }
    table->blockSignals(false);
}

void MainWindow::onTableRowSelected(int row, int /*column*/) {
    // Выделяем всю строку при наведении
    table->selectRow(row);
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
        case 0: p->name = newValue; break;
        case 3: if (ok) p->health = intValue; break;
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
