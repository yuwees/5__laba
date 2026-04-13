#include "CraftDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPainter>
#include <QRandomGenerator>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>

CraftDialog::CraftDialog(const Personazh* p, QWidget* parent, int imageMode)
    : QDialog(parent), person(p), currentImageMode(imageMode), customImagePath("") {

    setWindowTitle("Карточка персонажа");
    setModal(true);
    setFixedSize(420, 530);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // Верхняя панель
    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->setSpacing(8);

    QLabel* modeLabel = new QLabel("Режим:");
    modeLabel->setStyleSheet("QLabel { font-weight: bold; }");
    topLayout->addWidget(modeLabel);

    imageModeCombo = new QComboBox();
    imageModeCombo->addItems({"Статичная", "Случайная", "Уникальная", "Выбрать"});
    imageModeCombo->setCurrentIndex(currentImageMode);
    imageModeCombo->setMinimumWidth(100);
    topLayout->addWidget(imageModeCombo);

    chooseImageBtn = new QPushButton("Выбрать");
    chooseImageBtn->setMinimumWidth(80);
    topLayout->addWidget(chooseImageBtn);

    saveImageBtn = new QPushButton("Сохранить");
    saveImageBtn->setMinimumWidth(90);
    topLayout->addWidget(saveImageBtn);
    topLayout->addStretch();

    // Изображение по центру
    imageLabel = new QLabel;
    imageLabel->setFixedSize(200, 200);
    imageLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("QLabel { background-color: #f5f5f5; }");

    QHBoxLayout* imageLayout = new QHBoxLayout;
    imageLayout->addStretch();
    imageLayout->addWidget(imageLabel);
    imageLayout->addStretch();

    // Информация
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
        nameLabel->setStyleSheet("QLabel { color: black; }");

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
        QLabel* hpLabel = new QLabel("ХП: <span style='color: #008000; font-weight: bold; font-size: 14px;'>" + QString::number(mag->health) + "</span>");
        hpLabel->setTextFormat(Qt::RichText);

        QLabel* manaL = new QLabel("Мана: <span style='color: #0000FF; font-weight: bold; font-size: 14px;'>" + QString::number(mag->mana) + "</span>");
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

        QLabel* hpLabel = new QLabel("ХП врага: <span style='color: #FF0000; font-weight: bold; font-size: 14px;'>" + QString::number(vrag->health) + "</span>");
        hpLabel->setTextFormat(Qt::RichText);
        hpLabel->setAlignment(Qt::AlignCenter);

        QLabel* armorLabel = new QLabel(vrag->armor.toString());
        armorLabel->setStyleSheet("QLabel { color: black; font-weight: bold; font-size: 14px; }");
        armorLabel->setAlignment(Qt::AlignCenter);

        infoLayout->addWidget(nameLabel);
        infoLayout->addWidget(hpLabel);
        infoLayout->addWidget(armorLabel);
    }

    // Кнопки
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

    mainLayout->addLayout(topLayout);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(imageLayout);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(infoWidget);
    mainLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    connect(printBtn, &QPushButton::clicked, this, &CraftDialog::onPrint);
    connect(cancelBtn, &QPushButton::clicked, this, &CraftDialog::onCancel);
    connect(saveImageBtn, &QPushButton::clicked, this, &CraftDialog::saveImage);
    connect(chooseImageBtn, &QPushButton::clicked, this, &CraftDialog::chooseCustomImage);
    connect(imageModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CraftDialog::changeImageMode);

    updateImage();
}

void CraftDialog::setImageMode(int mode) {
    currentImageMode = mode;
    imageModeCombo->setCurrentIndex(mode);
    updateImage();
}

void CraftDialog::changeImageMode(int index) {
    currentImageMode = index;
    customImagePath = "";
    updateImage();
}

void CraftDialog::chooseCustomImage() {
    QString fileName = QFileDialog::getOpenFileName(this, "Выбрать картинку", "", "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!fileName.isEmpty()) {
        customImagePath = fileName;
        currentImageMode = 3;
        imageModeCombo->setCurrentIndex(3);
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
    imageLabel->setPixmap(currentPixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QPixmap CraftDialog::loadStaticImage() const {
    QString basePath = QCoreApplication::applicationDirPath() + "/";
    basePath += dynamic_cast<const MagP*>(person) ? "mag_res/mage_default.png" : "vrag_res/enemy_default.png";
    QPixmap pixmap(basePath);
    if (pixmap.isNull()) {
        pixmap = QPixmap(200, 200);
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

    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::lightGray);
    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 11));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, person->name);
    return pixmap;
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
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить изображение", person->name + ".png", "Images (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty() && currentPixmap.save(fileName)) {
        QMessageBox::information(this, "Успех", "Изображение сохранено");
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить изображение");
    }
}

void CraftDialog::onPrint() {
    emit printClicked();
    accept();
}

void CraftDialog::onCancel() {
    reject();
}