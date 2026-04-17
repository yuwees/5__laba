#ifndef CRAFTDIALOG_H
#define CRAFTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QShortcut>
#include "Personazh.h"

class CraftDialog : public QDialog {
    Q_OBJECT
public:
    explicit CraftDialog(Personazh* p, QWidget* parent = nullptr, int imageMode = 0);
    
signals:
    void printClicked();
    void dataChanged();
    
private slots:
    void onPrint();
    void onCancel();
    void chooseImage();
    
private:
    Personazh* person;
    int currentImageMode;
    QLabel* imageLabel;
    QPixmap currentPixmap;
    QString customImagePath;
    
    void updateImage();
    QPixmap loadStaticImage() const;
    QPixmap loadRandomImage() const;
    QPixmap loadUniqueImage() const;
    QPixmap loadCustomImage() const;
};

#endif
