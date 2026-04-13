#ifndef CRAFTDIALOG_H
#define CRAFTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include "Personazh.h"

class CraftDialog : public QDialog {
    Q_OBJECT
public:
    explicit CraftDialog(const Personazh* p, QWidget* parent = nullptr, int imageMode = 0);
    void setImageMode(int mode);

signals:
    void printClicked();

private slots:
    void onPrint();
    void onCancel();
    void saveImage();
    void changeImageMode(int index);
    void chooseCustomImage();

private:
    const Personazh* person;
    int currentImageMode;
    QLabel* imageLabel;
    QComboBox* imageModeCombo;
    QPushButton* saveImageBtn;
    QPushButton* chooseImageBtn;
    QPixmap currentPixmap;
    QString customImagePath;

    void updateImage();
    QPixmap loadStaticImage() const;
    QPixmap loadRandomImage() const;
    QPixmap loadUniqueImage() const;
    QPixmap loadCustomImage() const;
};

#endif