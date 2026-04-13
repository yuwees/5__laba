#ifndef CRAFTDIALOG_H
#define CRAFTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include "Personazh.h"

class CraftDialog : public QDialog {
    Q_OBJECT
public:
    explicit CraftDialog(const Personazh* p, QWidget* parent = nullptr);
    
signals:
    void printClicked();
    
private slots:
    void onPrint();
    void onCancel();
    void saveImage();
    
private:
    const Personazh* person;
    QLabel* imageLabel;
    QPixmap currentPixmap;
    
    void updateImage();
    QPixmap loadStaticImage() const;
};

#endif
