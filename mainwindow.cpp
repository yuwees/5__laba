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
#include <QTimer>
#include <QCursor>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), previousHoveredRow(-1), editMode(false) {
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
    btnEdit = new QPushButton("Редактировать");
    
    btnLoad->setMinimumWidth(120);
    btnLoadJSON->setMinimumWidth(130);
    btnSaveJson->setMinimumWidth(130);
    btnEdit->setMinimumWidth(120);
    
    btnLoad->setStyleSheet("QPushButton { background-color: #87CEEB; color: black; font-weight: bold; padding: 8px 12px; border-radius: 4px; }");
    btnLoadJSON->setStyleSheet("QPushButton { background-color: #87CEEB; color: black; font-weight: bold; padding: 8px 12px; border-radius: 4px; }");
    btnSaveJson->setStyleSheet("QPushButton { background-color: #3CB371; color: white; font-weight: bold; padding: 8px 12px; border-radius: 4px; }");
    btnEdit->setStyleSheet("QPushButton { background-color: #FF9800; color: white; font-weight: bold; padding: 8px 12px; border-radius: 4px; }");
    
    QLabel* modeLabel = new QLabel("Режим картинок:");
    modeLabel->setStyleSheet("QLabel { font-weight: bold; }");
    
    imageModeCombo = new QComboBox();
    imageModeCombo->addItems({"Статичные", "Случайные", "Уникальные"});
    imageModeCombo->setMinimumWidth(120);
    
    topLayout->addWidget(btnLoad);
    topLayout->addWidget(btnLoadJSON);
    topLayout->addWidget(btnSaveJson);
    topLayout->addWidget(btnEdit);
    topLayout->addSpacing(20);
    topLayout->addWidget(modeLabel);
    topLayout->addWidget(imageModeCombo);
    topLayout->addStretch();

    table = new QTableWidget(0, 5);
    table->setHorizontalHeaderLabels({"Имя", "Стихия/Редкость", "Мана/Урон", "ХП", "Броня"});
    
    table->setColumnWidth(0, 120);
    table->setColumnWidth(1, 130);
    table->setColumnWidth(2, 100);
    table->setColumnWidth(3, 80);
    table->setColumnWidth(4, 150);
    
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table->setAlternatingRowColors(false);
    
    table->setStyleSheet(
        "QTableWidget { gridline-color: #a0a0a0; outline: none; }"
        "QTableWidget::item { border: 1px solid #a0a0a0; padding: 6px; }"
        "QTableWidget::item:selected { background-color: #c896ff; color: black; outline: none; }"
        "QTableWidget::item:focus { outline: none; }"
        "QHeaderView::section { border: 1px solid #a0a0a0; background-color: #e8e8e8; padding: 8px; font-weight: bold; }"
    );
    table->setFocusPolicy(Qt::NoFocus);
    
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);  // ← изначально редактирование выключено
    table->setMouseTracking(true);
    table->viewport()->setMouseTracking(true);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(table);
    setCentralWidget(central);

    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadFile);
    connect(btnLoadJSON, &QPushButton::clicked, this, &MainWindow::loadJSON);
    connect(btnSaveJson, &QPushButton::clicked, this, &MainWindow::saveToJson);
    connect(btnEdit, &QPushButton::clicked, this, &MainWindow::toggleEditMode);
    
    connect(table, &QTableWidget::cellEntered, this, [this](int row) {
        table->selectRow(row);
        previousHoveredRow = row;
    });
    
    connect(table, &QTableWidget::cellClicked, this, &MainWindow::onTableRowClicked);
    connect(table, &QTableWidget::cellChanged, this, &MainWindow::editCell);

    QTimer* hoverTimer = new QTimer(this);
    connect(hoverTimer, &QTimer::timeout, this, [this]() {
        QPoint pos = table->viewport()->mapFromGlobal(QCursor::pos());
        QRect viewportRect = table->viewport()->rect();
        if (!viewportRect.contains(pos)) {
            table->clearSelection();
            previousHoveredRow = -1;
        }
    });
    hoverTimer->start(50);

    resize(950, 500);
    setWindowTitle("NPC Manager");
    setMinimumSize(850, 400);
}

void MainWindow::toggleEditMode() {
    editMode = !editMode;
    
    if (editMode) {
        table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
        btnEdit->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px 12px; border-radius: 4px; }");
        btnEdit->setText("Завершить редактирование");
    } else {
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        btnEdit->setStyleSheet("QPushButton { background-color: #FF9800; color: white; font-weight: bold; padding: 8px 12px; border-radius: 4px; }");
        btnEdit->setText("Редактировать");
    }
}

void MainWindow::loadFile() {
    QString filename = QFileDialog::getOpenFileName(this, "Выберите TXT файл", "", "Text files (*.txt)");
    if (filename.isEmpty()) return;
    
    for (auto p : persons) delete p;
    persons.clear();
    parseFile(filename);
    refreshTable();
}

void MainWindow::parseFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    QByteArray data = file.readAll();
    file.close();
    
    if (data.startsWith("\xEF\xBB\xBF")) {
        data.remove(0, 3);
    }
    
    QString content = QString::fromUtf8(data);
    
    if (content.contains(QChar(0xFFFD)) || content.isEmpty()) {
        content = QString::fromLocal8Bit(data);
    }
    
    QStringList lines = content.split('\n', Qt::SkipEmptyParts);
    
    for (QString line : lines) {
        line = line.trimmed();
        line.remove('\r');
        if (line.isEmpty()) continue;
        
        Personazh* p = createFromLine(line);
        if (p) persons.push_back(p);
    }
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
    refreshTable();
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
}

void MainWindow::refreshTable() {
    table->blockSignals(true);
    table->setRowCount(static_cast<int>(persons.size()));
    for (size_t i = 0; i < persons.size(); ++i) {
        Personazh* p = persons[i];
        
        QTableWidgetItem* item0 = new QTableWidgetItem(p->name);
        item0->setFlags(item0->flags() | Qt::ItemIsEditable);
        table->setItem(static_cast<int>(i), 0, item0);
        
        QTableWidgetItem* item1 = new QTableWidgetItem(p->getSpecial1());
        item1->setFlags(item1->flags() | Qt::ItemIsEditable);
        table->setItem(static_cast<int>(i), 1, item1);
        
        QTableWidgetItem* item2 = new QTableWidgetItem(QString::number(p->getSpecial2()));
        item2->setFlags(item2->flags() | Qt::ItemIsEditable);
        table->setItem(static_cast<int>(i), 2, item2);
        
        QTableWidgetItem* item3 = new QTableWidgetItem(QString::number(p->health));
        item3->setFlags(item3->flags() | Qt::ItemIsEditable);
        table->setItem(static_cast<int>(i), 3, item3);
        
        QTableWidgetItem* item4 = new QTableWidgetItem(p->armor.toString());
        item4->setFlags(item4->flags() | Qt::ItemIsEditable);
        table->setItem(static_cast<int>(i), 4, item4);
    }
    table->blockSignals(false);
}

void MainWindow::onTableRowClicked(int row, int /*column*/) {
    table->clearSelection();
    
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
        case 0: p->name = newValue; break;
        case 1:
            if (auto mag = dynamic_cast<MagP*>(p)) mag->element = newValue;
            else if (auto vrag = dynamic_cast<VragP*>(p)) vrag->rarity = newValue;
            break;
        case 2:
            if (ok) {
                if (auto mag = dynamic_cast<MagP*>(p)) mag->mana = intValue;
                else if (auto vrag = dynamic_cast<VragP*>(p)) vrag->damage = intValue;
            }
            break;
        case 3: if (ok) p->health = intValue; break;
        case 4: {
            QStringList parts = newValue.split('/');
            if (parts.size() == 3) {
                p->armor.helmet = parts[0].trimmed().toInt();
                p->armor.cuirass = parts[1].trimmed().toInt();
                p->armor.boots = parts[2].trimmed().toInt();
            }
            break;
        }
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
