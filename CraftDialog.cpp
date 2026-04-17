#include "CraftDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QRandomGenerator>
#include <QDir>
#include <QMessageBox>
#include <QCoreApplication>

CraftDialog::CraftDialog(const Personazh* p, QWidget* parent, int imageMode)
    : QDialog(parent), person(p), currentImageMode(imageMode) {
    
    setWindowTitle("Карточка персонажа");
    setModal(true);
    setFixedSize(380, 420);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    imageLabel = new QLabel;
    imageLabel->setFixedSize(180, 180);
    imageLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("QLabel { background-color: #f5f5f5; }");
    
    QHBoxLayout* imageLayout = new QHBoxLayout;
    imageLayout->addStretch();
    imageLayout->addWidget(imageLabel);
    imageLayout->addStretch();
    
    QWidget* infoWidget = new QWidget;
    QVBoxLayout* infoLayout = new QVBoxLayout(infoWidget);
    infoLayout->setSpacing(8);
    
    if (auto mag = dynamic_cast<const MagP*>(person)) {
        QLabel* nameLabel = new QLabel(mag->name);
        QFont boldFont = nameLabel->font();
        boldFont.setBold(true);
        boldFont.setPointSize(14);
        nameLabel->setFont(boldFont);
        nameLabel->setAlignment(Qt::AlignCenter);
        
        QString elementLower = mag->element.toLower();
        QString elementColor;
        if (elementLower.contains("огонь")) elementColor = "#FF4500";
        else if (elementLower.contains("вода")) elementColor = "#1E90FF";
        else if (elementLower.contains("воздух")) elementColor = "#87CEEB";
        else if (elementLower.contains("земля")) elementColor = "#8B4513";
        else elementColor = "black";
        
        QLabel* elemLabel = new QLabel("Стихия: <span style='color: " + elementColor + "; font-weight: bold;'>" + mag->element + "</span>");
        elemLabel->setTextFormat(Qt::RichText);
        elemLabel->setAlignment(Qt::AlignCenter);
        
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
        
        infoLayout->addWidget(nameLabel);
        infoLayout->addWidget(elemLabel);
        infoLayout->addLayout(statsLayout);
        
    } else if (auto vrag = dynamic_cast<const VragP*>(person)) {
        QString rarityLower = vrag->rarity.toLower();
        QString nameColor;
        if (rarityLower.contains("легенда")) nameColor = "#FFD700";
        else if (rarityLower.contains("редкий")) nameColor = "#4169E1";
        else if (rarityLower.contains("обычный")) nameColor = "#32CD32";
        else nameColor = "black";
        
        QLabel* nameLabel = new QLabel("<span style='color: " + nameColor + ";'>" + vrag->name + "</span>");
        QFont boldFont = nameLabel->font();
        boldFont.setBold(true);
        boldFont.setPointSize(14);
        nameLabel->setFont(boldFont);
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setTextFormat(Qt::RichText);
        
        QLabel* hpLabel = new QLabel("ХП врага: <span style='color: #FF0000; font-weight: bold;'>" + QString::number(vrag->health) + "</span>");
        hpLabel->setTextFormat(Qt::RichText);
        hpLabel->setAlignment(Qt::AlignCenter);
        
        QLabel* armorLabel = new QLabel(vrag->armor.toString());
        armorLabel->setStyleSheet("QLabel { color: black; font-weight: bold; }");
        armorLabel->setAlignment(Qt::AlignCenter);
        
        infoLayout->addWidget(nameLabel);
        infoLayout->addWidget(hpLabel);
        infoLayout->addWidget(armorLabel);
    }
    
    QPushButton* printBtn = new QPushButton("Печать");
    QPushButton* cancelBtn = new QPushButton("Отмена");
    printBtn->setFixedWidth(100);
    cancelBtn->setFixedWidth(100);
    printBtn->setStyleSheet("QPushButton { background-color: #4169E1; color: white; font-weight: bold; padding: 8px 15px; border-radius: 4px; }");
    cancelBtn->setStyleSheet("QPushButton { background-color: #DC143C; color: white; font-weight: bold; padding: 8px 15px; border-radius: 4px; }");
    
    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(printBtn);
    btnLayout->addWidget(cancelBtn);
    btnLayout->addStretch();
    
    mainLayout->addLayout(imageLayout);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(infoWidget);
    mainLayout->addStretch();
    mainLayout->addLayout(btnLayout);
    
    connect(printBtn, &QPushButton::clicked, this, &CraftDialog::onPrint);
    connect(cancelBtn, &QPushButton::clicked, this, &CraftDialog::onCancel);
    
    updateImage();
}

void CraftDialog::updateImage() {
    switch (currentImageMode) {
        case 0: currentPixmap = loadStaticImage(); break;
        case 1: currentPixmap = loadRandomImage(); break;
        case 2: currentPixmap = loadUniqueImage(); break;
        default: currentPixmap = loadStaticImage();
    }
    imageLabel->setPixmap(currentPixmap.scaled(180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QPixmap CraftDialog::loadStaticImage() const {
    QString basePath = QCoreApplication::applicationDirPath() + "/";
    basePath += dynamic_cast<const MagP*>(person) ? "mag_res/mage_default.png" : "vrag_res/enemy_default.png";
    QPixmap pixmap(basePath);
    if (pixmap.isNull()) {
        pixmap = QPixmap(180, 180);
        pixmap.fill(dynamic_cast<const MagP*>(person) ? QColor(100, 150, 255) : QColor(255, 100, 100));
        QPainter painter(&pixmap);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        painter.drawText(pixmap.rect(), Qt::AlignCenter, dynamic_cast<const MagP*>(person) ? "MAGE" : "ENEMY");
    }
    return pixmap;
}

QPixmap CraftDialog::loadRandomImage() const {
    QString folderPath = QCoreApplication::applicationDirPath() + "/";
    folderPath += dynamic_cast<const MagP*>(person) ? "mag_res" : "vrag_res";
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
    basePath += dynamic_cast<const MagP*>(person) ? "mag_res/" : "vrag_res/";
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

void CraftDialog::onPrint() {
    emit printClicked();
    accept();
}

void CraftDialog::onCancel() {
    reject();
}
