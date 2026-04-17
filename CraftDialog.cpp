#include "CraftDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QRandomGenerator>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <QKeySequence>

CraftDialog::CraftDialog(Personazh* p, QWidget* parent, int imageMode)
    : QDialog(parent), person(p), currentImageMode(imageMode), customImagePath("") {
    
    setWindowTitle("Карточка персонажа");
    setModal(true);
    setFixedSize(400, 450);
    
    QShortcut* saveShortcut = new QShortcut(QKeySequence::Save, this);
    connect(saveShortcut, &QShortcut::activated, this, &CraftDialog::saveImage);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    QPushButton* chooseBtn = new QPushButton("Выбрать картинку");
    chooseBtn->setStyleSheet("QPushButton { background-color: #FF9800; color: white; font-weight: bold; padding: 8px; border-radius: 4px; }");
    mainLayout->addWidget(chooseBtn);
    
    imageLabel = new QLabel;
    imageLabel->setFixedSize(180, 180);
    imageLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("QLabel { background-color: #f5f5f5; }");
    
    QHBoxLayout* imageLayout = new QHBoxLayout;
    imageLayout->addStretch();
    imageLayout->addWidget(imageLabel);
    imageLayout->addStretch();
    mainLayout->addLayout(imageLayout);
    
    if (auto mag = dynamic_cast<MagP*>(person)) {
        QLabel* nameLabel = new QLabel(mag->name);
        QFont boldFont = nameLabel->font();
        boldFont.setBold(true);
        boldFont.setPointSize(14);
        nameLabel->setFont(boldFont);
        nameLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(nameLabel);
        
        QString elementColor;
        QString elemLower = mag->element.toLower();
        if (elemLower.contains("огонь")) elementColor = "#FF4500";
        else if (elemLower.contains("вода")) elementColor = "#1E90FF";
        else if (elemLower.contains("воздух")) elementColor = "#87CEEB";
        else if (elemLower.contains("земля")) elementColor = "#8B4513";
        else elementColor = "black";
        
        QLabel* elemLabel = new QLabel("Стихия: <span style='color: " + elementColor + "; font-weight: bold;'>" + mag->element + "</span>");
        elemLabel->setTextFormat(Qt::RichText);
        elemLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(elemLabel);
        
        QHBoxLayout* statsLayout = new QHBoxLayout();
        QLabel* hpLabel = new QLabel("ХП: <span style='color: #008000; font-weight: bold;'>" + QString::number(mag->health) + "</span>");
        hpLabel->setTextFormat(Qt::RichText);
        QLabel* manaL = new QLabel("Мана: <span style='color: #0000FF; font-weight: bold;'>" + QString::number(mag->mana) + "</span>");
        manaL->setTextFormat(Qt::RichText);
        statsLayout->addStretch();
        statsLayout->addWidget(hpLabel);
        statsLayout->addSpacing(40);
        statsLayout->addWidget(manaL);
        statsLayout->addStretch();
        mainLayout->addLayout(statsLayout);
        
    } else if (auto vrag = dynamic_cast<VragP*>(person)) {
        QString nameColor;
        QString rarLower = vrag->rarity.toLower();
        if (rarLower.contains("легенда")) nameColor = "#FFD700";
        else if (rarLower.contains("редкий")) nameColor = "#4169E1";
        else if (rarLower.contains("обычный")) nameColor = "#32CD32";
        else nameColor = "black";
        
        QLabel* nameLabel = new QLabel("<span style='color: " + nameColor + ";'>" + vrag->name + "</span>");
        QFont boldFont = nameLabel->font();
        boldFont.setBold(true);
        boldFont.setPointSize(14);
        nameLabel->setFont(boldFont);
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setTextFormat(Qt::RichText);
        mainLayout->addWidget(nameLabel);
        
        QLabel* hpLabel = new QLabel("ХП врага: <span style='color: #FF0000; font-weight: bold;'>" + QString::number(vrag->health) + "</span>");
        hpLabel->setTextFormat(Qt::RichText);
        hpLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(hpLabel);
        
        QLabel* armorLabel = new QLabel(vrag->armor.toString());
        armorLabel->setStyleSheet("QLabel { color: black; font-weight: bold; }");
        armorLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(armorLabel);
    }
    
    mainLayout->addStretch();
    
    QLabel* hintLabel = new QLabel("Ctrl+S — сохранить картинку");
    hintLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    hintLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(hintLabel);
    
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
    
    connect(chooseBtn, &QPushButton::clicked, this, &CraftDialog::chooseImage);
    connect(printBtn, &QPushButton::clicked, this, &CraftDialog::onPrint);
    connect(cancelBtn, &QPushButton::clicked, this, &CraftDialog::onCancel);
    
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
    imageLabel->setPixmap(currentPixmap.scaled(180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QPixmap CraftDialog::loadStaticImage() const {
    QString basePath = QCoreApplication::applicationDirPath() + "/";
    basePath += dynamic_cast<MagP*>(person) ? "mag_res/mage_default.png" : "vrag_res/enemy_default.png";
    QPixmap pixmap(basePath);
    if (pixmap.isNull()) {
        pixmap = QPixmap(180, 180);
        pixmap.fill(dynamic_cast<MagP*>(person) ? QColor(100, 150, 255) : QColor(255, 100, 100));
        QPainter painter(&pixmap);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 14, QFont::Bold));
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
