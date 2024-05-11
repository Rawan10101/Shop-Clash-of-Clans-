#include "fence1.h"
#include "troop.h"
#include <QTimer>
#include <QList>
#include <QDebug>
#include <QPixmap>
#include<QObject>
#include<QGraphicsScene>
// #include<game.h>
Fence1::Fence1(QString type)
{
    QPixmap pixmap(":/images/Wall.png");
    pixmap = pixmap.scaledToWidth(50);
    pixmap = pixmap.scaledToHeight(50);
    this->setPixmap(pixmap);
    fenceHealth = new Health(healthValue);
    fenceType = type;
    remove = false;

    setFlag(QGraphicsItem::ItemIsSelectable, true);  // Enable item selection
}

qreal Fence1::healthValue = 500;

Fence1::~Fence1()
{
}


void Fence1::UpgradeButton()
{
    QPixmap newImage("C:/Users/HP/Downloads/Wall3.webp");
    newImage = newImage.scaledToWidth(50);
    newImage = newImage.scaledToHeight(50);
    this->setPixmap(newImage);

    int newHealth = 15;
    fenceHealth->setMaxHealth(newHealth);

    // emit fenceUpgraded(newImage, newHealth);
}
void Fence1::setPix(QPixmap image){
    this->setPixmap(image);
}
