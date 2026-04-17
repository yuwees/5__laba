#include "CraftDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPainter>
#include <QRandomGenerator>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <QKeySequence>
#include <QLineEdit>

CraftDialog::CraftDialog(Personazh* p, QWidget* parent, int imageMode)
    : QDialog(parent), person(p), currentImageMode(imageMode), customImagePath("") {
    
    setWindowTitle("Карточка персонажа");
    setModal(true);
    setFixedSize(450, 600);
    
    QShortcut* saveShortcut = new QShortcut(QKeySequence::Save, this);
    connect(saveShortcut, &QShortcut::activated, this, &CraftDialog::saveImage);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Кнопка выбора картинки
    QPushButton* chooseBtn = new QPushButton("Выбрать картинку");
    chooseBtn->setStyleSheet("QPushButton { background-color: #FF9800; color: white; font-weight: bold; padding: 8px; border-radius: 4px; }");
    mainLayout->addWidget(chooseBtn);
    
    // Изображение
    imageLabel = new QLabel;
    imageLabel->setFixedSize(150, 150);
    imageLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("QLabel { background-color: #f5f5f5; }");
    
    QHBoxLayout* imageLayout = new QHBoxLayout;
    imageLayout->addStretch();
    imageLayout->addWidget(imageLabel);
    imageLayout->addStretch();
    mainLayout->addLayout(imageLayout);
    
    // Форма редактирования
    QGroupBox* editGroup = new QGroupBox("Редактирование данных");
    QFormLayout* formLayout = new QFormLayout(editGroup);
    
    QLineEdit* nameEdit = new QLineEdit(person->name);
    formLayout->addRow("Имя:", nameEdit);
    
    QLineEdit* special1Edit = nullptr;
    QLineEdit* special2Edit = nullptr;
    
    if (auto mag = dynamic_cast<MagP*>(person)) {
        special1Edit = new QLineEdit(mag->element);
        formLayout->addRow("Стихия:", special1Edit);
        
        special2Edit = new QLineEdit(QString::number(mag->mana));
        formLayout->addRow("Мана:", special2Edit);
    } else if (auto vrag = dynamic_cast<VragP*>(person)) {
        special1Edit = new QLineEdit(vrag->rarity);
        formLayout->addRow("Редкость:", special1Edit);
        
        special2Edit = new QLineEdit(QString::number(vrag->damage));
        formLayout->addRow("Урон:", special2Edit);
    }
    
    QLineEdit* healthEdit = new QLineEdit(QString::number(person->health));
    formLayout->addRow("ХП:", healthEdit);
    
    QLineEdit* helmetEdit = new QLineEdit(QString::number(person->armor.helmet));
    formLayout->addRow("Шлем:", helmetEdit);
    
    QLineEdit* cuirassEdit = new QLineEdit(QString::number(person->armor.cuirass));
    formLayout->addRow("Кираса:", cuirassEdit);
    
    QLineEdit* bootsEdit = new QLineEdit(QString::number(person->armor.boots));
    formLayout->addRow("Сапоги:", bootsEdit);
    
    mainLayout->addWidget(editGroup);
    
    // Кнопки
    QPushButton* saveAllBtn = new QPushButton("Сохранить всё");
    QPushButton* saveImageBtn = new QPushButton("Сохранить картинку");
    
    saveAllBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px; border-radius: 4px; }");
    saveImageBtn->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; padding: 8px; border-radius: 4px; }");
    
    QHBoxLayout* saveLayout = new QHBoxLayout;
    saveLayout->addWidget(saveAllBtn);
    saveLayout->addWidget(saveImageBtn);
    mainLayout->addLayout(saveLayout);
    
    // Подсказка
    QLabel* hintLabel = new QLabel("Ctrl+S — сохранить картинку");
    hintLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    hintLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(hintLabel);
    
    // Кнопки Печать и Отмена
    QPushButton* printBtn = new QPushButton("Печать");
    QPushButton* cancelBtn = new QPushButton("Отмена");
    printBtn->setFixedWidth(100);
    cancelBtn->setFixedWidth(100);
    printBtn->setStyleSheet("QPushButton { background-color: #4169E1; color: white; font-weight: bold; padding: 8px; border-radius: 4px; }");
    cancelBtn->setStyleSheet("QPushButton { background-color: #DC143C; color: white; font-weight: bold; padding: 8px; border-radius: 4px; }");
    
    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(printBtn);
    btnLayout->addWidget(cancelBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);
    
    // Подключение сигналов
    connect(chooseBtn, &QPushButton::clicked, this, &CraftDialog::chooseImage);
    connect(saveImageBtn, &QPushButton::clicked, this, &CraftDialog::saveImage);
    connect(printBtn, &QPushButton::clicked, this, &CraftDialog::onPrint);
    connect(cancelBtn, &QPushButton::clicked, this, &CraftDialog::onCancel);
    
    // Сохранение ВСЕХ данных
    connect(saveAllBtn, &QPushButton::clicked, this, [=]() {
        // Сохраняем общие поля
        person->name = nameEdit->text();
        person->health = healthEdit->text().toInt();
        person->armor.helmet = helmetEdit->text().toInt();
        person->armor.cuirass = cuirassEdit->text().toInt();
        person->armor.boots = bootsEdit->text().toInt();
        
        // Сохраняем специфичные поля
        if (auto mag = dynamic_cast<MagP*>(person)) {
            mag->element = special1Edit->text();
            mag->mana = special2Edit->text().toInt();
        } else if (auto vrag = dynamic_cast<VragP*>(person)) {
            vrag->rarity = special1Edit->text();
            vrag->damage = special2Edit->text().toInt();
        }
        
        emit dataChanged();
        QMessageBox::information(this, "Успех", "Все данные сохранены!");
    });
    
    updateImage();
}

void CraftDialog::chooseImage() {
    QString fileName = QFileDialog::getOpenFileName(this, "Выбрать картинку", "", "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!fileName.isEmpty()) {
        customImagePath = fileName;
        currentImageMode = 3;
        updateImage();
    }
}

void CraftDialog::updateImage() {
    switch (currentImageMode) {
        case 0: currentPixmap = loadStaticImage(); break;
        case 1: currentPixmap = loadRandomImage(); break;
        case 2: currentPixmap = loadUniqueImage(); break;
        case 3: currentPixmap = loadCustomImage(); break;
        default: currentPixmap = loadStaticImage();
    }
    imageLabel->setPixmap(currentPixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QPixmap CraftDialog::loadStaticImage() const {
    QString basePath = QCoreApplication::applicationDirPath() + "/";
    basePath += dynamic_cast<MagP*>(person) ? "mag_res/mage_default.png" : "vrag_res/enemy_default.png";
    QPixmap pixmap(basePath);
    if (pixmap.isNull()) {
        pixmap = QPixmap(150, 150);
        pixmap.fill(dynamic_cast<MagP*>(person) ? QColor(100, 150, 255) : QColor(255, 100, 100));
        QPainter painter(&pixmap);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(pixmap.rect(), Qt::AlignCenter, dynamic_cast<MagP*>(person) ? "MAGE" : "ENEMY");
    }
    return pixmap;
}

QPixmap CraftDialog::loadRandomImage() const {
    QString folderPath = QCoreApplication::applicationDirPath() + "/";
    folderPath += dynamic_cast<MagP*>(person) ? "mag_res" : "vrag_res";
    QDir dir(folderPath);
    QStringList filters = {"*.png", "*.jpg", "*.jpeg", "*.bmp"};
    QStringList files = dir.entryList(filters, QDir::Files);
    if (!files.isEmpty()) {
        return QPixmap(dir.filePath(files[QRandomGenerator::global()->bounded(files.size())]));
    }
    return loadStaticImage();
}

QPixmap CraftDialog::loadUniqueImage() const {
    QString basePath = QCoreApplication::applicationDirPath() + "/";
    basePath += dynamic_cast<MagP*>(person) ? "mag_res/" : "vrag_res/";
    QStringList possibleNames;
    possibleNames << person->name.toLower().replace(" ", "_") + ".png"
                  << QString::number(person->code) + ".png"
                  << person->getSpecial1().toLower() + ".png";
    for (const QString& name : possibleNames) {
        QPixmap pixmap(basePath + name);
        if (!pixmap.isNull()) return pixmap;
    }
    return loadStaticImage();
}

QPixmap CraftDialog::loadCustomImage() const {
    if (!customImagePath.isEmpty()) {
        QPixmap pixmap(customImagePath);
        if (!pixmap.isNull()) return pixmap;
    }
    return loadStaticImage();
}

void CraftDialog::saveImage() {
    if (currentPixmap.isNull()) {
        QMessageBox::warning(this, "Ошибка", "Нет изображения для сохранения");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить изображение", 
                                                     person->name + ".png",
                                                     "Images (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty() && currentPixmap.save(fileName)) {
        QMessageBox::information(this, "Успех", "Изображение сохранено");
    }
}

void CraftDialog::onPrint() {
    emit printClicked();
    accept();
}

void CraftDialog::onCancel() {
    reject();
}
