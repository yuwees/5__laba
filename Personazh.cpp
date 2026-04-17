#include "Personazh.h"
#include "CraftDialog.h"
#include <QJsonArray>

QString Bronya::toString() const {
    return QString("%1/%2/%3").arg(helmet).arg(cuirass).arg(boots);
}

Personazh::Personazh(int c, const QString& n, int h, const Bronya& a)
    : code(c), name(n), health(h), armor(a) {}

QJsonObject Personazh::toJson() const {
    QJsonObject obj;
    obj["code"] = code;
    obj["name"] = name;
    obj["health"] = health;
    obj["helmet"] = armor.helmet;
    obj["cuirass"] = armor.cuirass;
    obj["boots"] = armor.boots;
    return obj;
}

MagP::MagP(int code, const QString& name, const QString& elem, int manaVal,
           int health, const Bronya& armor)
    : Personazh(code, name, health, armor), element(elem), mana(manaVal) {}

void MagP::Craft(QWidget* parent, int imageMode) {
    CraftDialog dlg(this, parent, imageMode);
    dlg.exec();
}

QJsonObject MagP::toJson() const {
    QJsonObject obj = Personazh::toJson();
    obj["type"] = "Mag";
    obj["element"] = element;
    obj["mana"] = mana;
    return obj;
}

VragP::VragP(int code, const QString& name, const QString& rar, int dmg,
             int health, const Bronya& armor)
    : Personazh(code, name, health, armor), rarity(rar), damage(dmg) {}

void VragP::Craft(QWidget* parent, int imageMode) {
    CraftDialog dlg(this, parent, imageMode);
    dlg.exec();
}

QJsonObject VragP::toJson() const {
    QJsonObject obj = Personazh::toJson();
    obj["type"] = "Vrag";
    obj["rarity"] = rarity;
    obj["damage"] = damage;
    return obj;
}