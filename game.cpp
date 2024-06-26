#include "Game.h"

using namespace std;

Game::Game(QWidget *parent) : QWidget(parent)
{
    int seed = QDateTime::currentMSecsSinceEpoch(); //seeding randomgenerator for troop spawning
    randomGenerator = new QRandomGenerator(seed);

    level = new Levels; //creating an instance of level
    layout = new QGridLayout(this);
    scene = new QGraphicsScene();
    view = new QGraphicsView(scene);
    // view->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    money = new playerMoney;

    shopWindow = new Shop1(this, money->getCurrentMoney());

    layout->deleteLater();


    setLayout(layout);
    //setting background
    QPixmap backgroundPixmap(":/images/Background.png"); // background image empty field
    if (!backgroundPixmap.isNull())
    {
        view->setBackgroundBrush(backgroundPixmap);
    }

    //layout
    setLayout(layout);
    view->setStyleSheet("background: transparent; border: 0px");
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(view);
    view->show();


    //START BUTTON
    startButton = new QPushButton("Start Level " + QString::number(level->currLevel));
    startButton->setStyleSheet("font-size: 20px;");
    connect(startButton, SIGNAL(clicked()), this, SLOT(handleStartButton()));

    // SHOP BUTTON
    shopButton = new QPushButton("Shop");
    shopButton->setStyleSheet("font-size: 20px;");
    connect(shopButton, SIGNAL(clicked()), this, SLOT(showShopWindow()));
    connect(shopWindow, &Shop1::moneyChanged, this, &Game::updateMoney);
    connect(shopWindow, &Shop1::imagehealthChanges, this, &Game::updateimagehealth);

    //SETTINGS BUTTON
    soundSettingsButton = new QPushButton("Sound Settings");
    soundSettingsButton->setStyleSheet("font-size: 20px;");
    connect(soundSettingsButton, SIGNAL(clicked()), this, SLOT(handleSoundSettingsButton()));

    // HOW TO PLAY BUTTON
    helpButton = new QPushButton("How To Play?");
    helpButton->setStyleSheet("font-size: 20px;");
    connect(helpButton, SIGNAL(clicked()), this, SLOT(handleHelpButton()));

    // QUIT BUTTON
    quitButton = new QPushButton("Quit");
    quitButton->setStyleSheet("font-size: 20px;");
    connect(quitButton, SIGNAL(clicked()), this, SLOT(handleQuitButton()));

    fence= new Fence1("");


    gameStarted = false;
   startLevel(); //sets scene and view and clan design and timertext

    //-----------------------------------------------//

    //TIMER LABEL
    timerRect = QRectF(10, 10, 100, 40); // Position and size of the timer rectangle
    timerText = new QGraphicsTextItem();
    timerText->setPos(timerRect.topLeft());
    timerText->setPlainText("0:00");
    timerText->setDefaultTextColor(Qt::white);
    timerText->setFont(QFont("Arial", 20));
    timerText->setOpacity(0.8);
    timerText->setPos(0,0);
    scene->addItem(timerText);
    timerText->hide();

    //MoneyBar will increase by 5 after killing one troop and increase by 20 when a level is completed
    moneyBar= new QProgressBar(this);
    moneyBar->setTextVisible(false);
    moneyBar->setGeometry(12,52,60,20);
    moneyBar->setRange(0,1000);

    moneyBar->setValue(money->getCurrentMoney());
    moneyLabel = new QLabel(this);
    moneyLabel->setGeometry(80,52,60,20);
    moneyLabel->setText(QString::number(money->getCurrentMoney()));

    moneyBar->hide();
    moneyLabel->hide();

    backgroundMusicPlayer.setAudioOutput(&audiooutput);
    backgroundMusicPlayer.setSource(QUrl("qrc:/Sound files/fm-freemusic-inspiring-optimistic-upbeat-energetic-guitar-rhythm(chosic.com).mp3"));
    backgroundMusicPlayer.play();


   // backgroundMusicPlayer->setVolume(50);

    bulletSoundEffect = new QSoundEffect();
    bulletSoundEffect->setSource(QUrl("qrc:/sounds/bullet_sound.wav"));
    bulletSoundEffect->setVolume(50);

    ////////////////////////
    // Booster* booster = new Booster();
    // booster->setPos(50,50);
    // scene->addItem(booster);


}
void Game::updateMoney(int newMoney)
{

    money->setMoney(newMoney);

    moneyBar->setValue(newMoney);
    moneyLabel->setText(QString::number(newMoney));
}

void Game::updateimagehealth(QString type, QPixmap image, int health2)
{

    // imageFence=image;
    if(type=="Fence"){
       imageFence=image;
    }
    else if( type== "Cannon"){
      imageCannon=image;
    }
    else if(type=="Townhall"){
        imageTownhall=image;
    }

}



void Game::startLevel()
{
    QGraphicsProxyWidget* startBtn = scene->addWidget(startButton);
    QGraphicsProxyWidget* shopBtn = scene->addWidget(shopButton);

    startBtn->setPos(240,400);
    shopBtn->setPos(240,450);
    startButton->setText("Start Level " + QString::number(level->currLevel)); //to update start button text
    startButton->setEnabled(true);
    startButton->show();

    shopButton->setEnabled(true);
    shopButton->show();

    gameStarted = false;
    townHallDestroyed = false;


    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTimer())); //game timer

    m_timer = new QTimer(this); //troop movement timer
    connect(m_timer,SIGNAL(timeout()),this,SLOT (moveTroops()));

    spawnTimer = new QTimer(); //spawning troops timer
    connect(spawnTimer, SIGNAL(timeout()),this, SLOT (formTroops()));

    boosterTimer = new QTimer(); //spawning boosters (health markers)
    connect(boosterTimer, SIGNAL(timeout()), this, SLOT(spawnBoosters()));

    clanDesign = level->getDesignVector(); //sends a vector for the clan design based on the current level

    scene->setSceneRect(0, 0, clanDesign[0].size() * 50, clanDesign.size() * 50); //set scene and view
    view->setFixedSize(clanDesign[0].size() * 50, clanDesign.size() * 50);



}

void Game::displayClanDesign()
{
    const int tileSize = 50;
    const int fenceSize = tileSize ;
    const int castleSize = tileSize;
    const int cannonSize = tileSize;

    qDebug()<<"inside displayClanDesign";

    for (int i = 0; i < clanDesign.size(); i++)
    {
        for (int j = 0; j < clanDesign[i].size(); j++)
        {
            int element = clanDesign[i][j]; //element at the current position

            if (element == 1) // castle
            {
                if(imageTownhall.isNull()){
                townhall = new Townhall();
                townhall->setPos(j * castleSize, i * castleSize);
                scene->addItem(townhall);}
                else
                {
                    townhall = new Townhall();
                    townhall->setPix(imageTownhall);
                    townhall->setPos(j * castleSize, i * castleSize);
                    scene->addItem(townhall);

                }
            }
            else if (element == 2) // cannon
            {
                if(imageCannon.isNull()){
                 cannon = new Cannon();
                cannon->setPos(j * cannonSize, i * cannonSize);
                 scene->addItem(cannon);
                       }
                       else
                {
                    cannon = new Cannon();
                    cannon->setPos(j * cannonSize, i * cannonSize);
                    scene->addItem(cannon);
                    cannon->setPix(imageCannon);
                }

            }
            else if (element == 3) // fence
            {
                //find out whether fence is horizontal or vertical
                if(imageFence.isNull()) {
                if (clanDesign[i-1][j] == 3 || clanDesign[i+1][j] == 3)
                  fence = new Fence1("vertical");
                else
                   fence = new Fence1("horizontal");
                 fence->setPos(j * fenceSize, i * fenceSize);
                scene->addItem(fence);
                }
            else
                {
                    if (clanDesign[i-1][j] == 3 || clanDesign[i+1][j] == 3)
                        fence = new Fence1("vertical");
                    else
                     fence = new Fence1("horizontal");
                    fence->setPix(imageFence);
                    fence->setPos(j * fenceSize, i * fenceSize);
                    scene->addItem(fence);
                }
        }

    }
    }}

void Game::handleStartButton()
{
    if (!gameStarted)
    {
        startButton->setEnabled(false);
        gameStarted = true;
        startButton->hide();
        shopButton->hide();
        startGame();
    }
}

void Game::startGame()
{
    workerCount = 0;
    killCount = 0;
    startButton->hide();


    timerText->show();
    moneyBar->show();
    moneyLabel->show();

    resetTimer();
    timer->start(1000);
    boosterTimer->start(30000);
    displayClanDesign();
    moneyBar->setValue(money->getCurrentMoney()); //update money bar
    moneyLabel->setText(QString::number(money->getCurrentMoney()));

    pause = new pauseButton;
    QGraphicsProxyWidget * pauseBtn = scene->addWidget(pause);
    pauseBtn->setPos(scene->width() - pause->width(), 0);
    connect(pause, SIGNAL(clicked()), this, SLOT(handlePauseButton()));

    backgroundMusicPlayer.play();
    if (level->currLevel == 1) //enemies spawn more frequently as level increases
        spawnTimer->start(4000);

    else if (level->currLevel == 2) {
        spawnTimer->start(3000);
    }

    else if (level->currLevel == 3) {
        spawnTimer->start(2000);
    }
    else if (level->currLevel == 4) {
        spawnTimer->start(1000);
    }
    else if (level->currLevel == 5) {
        spawnTimer->start(500);
    }


}


void Game::formTroops()
{
    int randomX;
    int randomY;

    do{
        randomX = randomGenerator->bounded(0, clanDesign[0].size()-1); //generating a random X and Y within scene dimensions
        randomY = randomGenerator->bounded(0, clanDesign.size()-1);
         // qDebug()<< randomX << " " << randomY;
    } while (clanDesign[randomY][randomX] != 0); //keeps generating until position is a 0

    qDebug() << "randomX: " << randomX << " randomY: " << randomY;
    Troop* troop = new Troop(level->currLevel * 300, level->currLevel * 100);
    scene->addItem(troop);
    troop->setPos(randomY  *50, randomX *50);

    if (level->currLevel == 1) //enemies move faster as level increases
        m_timer->start(30);

    else if (level->currLevel == 2)
        m_timer->start(25);

    else if (level->currLevel == 3)
        m_timer->start(20);

    else if (level->currLevel == 4)
        m_timer->start(15);

    else if (level->currLevel == 5)
        m_timer->start(10);

}

void Game::moveTroops()
{

    QList<QGraphicsItem*> items = scene->items();
    foreach (QGraphicsItem* item, items) {
        Troop* troop = dynamic_cast<Troop*>(item);
        if (troop) {
            Townhall* nearestTownhall = findNearestTownhall(troop->pos()); //troops move towards the castle/townhall
             if (nearestTownhall) {
                QPointF direction = nearestTownhall->pos()- troop->pos();
                direction /= QVector2D(direction).length();
                qreal dx = direction.x() * troop->speed;
                qreal dy = direction.y() * troop->speed;
                troop->setPos(troop->x() + dx, troop->y() + dy);

          checkCollisions(troop); //to destroy obstacles between troop and the castle/townhall
            }
        }
    }
}

Townhall* Game::findNearestTownhall(const QPointF& position)
{
    qreal minDistanceSquared = numeric_limits<qreal>::max();
    Townhall* nearestTownhall = nullptr;
    foreach (QGraphicsItem* item, scene->items()) {
        Townhall* townhall = dynamic_cast<Townhall*>(item);
        if (townhall) {
            qreal distanceSquared = QPointF(townhall->x() - position.x(), townhall->y() - position.y()).manhattanLength();
            if (distanceSquared < minDistanceSquared) {
                minDistanceSquared = distanceSquared;
                nearestTownhall = townhall;
            }
        }
    }
    return nearestTownhall;

}


void Game::checkCollisions(Troop* troop)
{
    QList<QGraphicsItem*> collidingItems = troop->collidingItems();


    foreach (QGraphicsItem* collidingItem, collidingItems)
    {
        if (typeid(*collidingItem) == typeid(Fence1))
        {



            Fence1* fence = dynamic_cast<Fence1*>(collidingItem);
            if (fence)
                    {
                        if (fence->fenceType == "vertical")
                        {
                            if (troop->x() > fence->x())
                                troop->setPos(troop->x() + 30, troop->y());
                            else
                                troop->setPos(troop->x() - 30, troop->y());
                        }
                        else
                        {
                            if (troop->y() > fence->y())
                                troop->setPos(troop->x(), troop->y() + 30);
                            else
                                troop->setPos(troop->x(), troop->y() - 30);
                        }
            qDebug() << "Collision with Fence1";
            fence->markForRemoval();
            fence->fenceHealth->reduceHealth(troop->troopPower);
            if (fence && typeid(*collidingItem) == typeid(Fence1))
            {
            if (fence->fenceHealth->getHealth() <= 0)
            {
            itemsToRemove.append(collidingItem);
             }
            }
            }
        }
     else  if (typeid(*collidingItem) == typeid(Bullet))
        {

            troop->troopHealth->reduceHealth(cannon->currPower);
            troop->knockBack(collidingItem->pos().x(), collidingItem->pos().y());

            if (troop->troopHealth->getHealth() <= 0)
            {
                itemsToRemove.append(collidingItem);
                itemsToRemove.append(troop);
                killCount++;

                if (killCount == 20)
                {
                    cannon->powerUp();
                    killCount = 0;
                }
            }}
        else if (typeid(*collidingItem) == typeid(Cannon))
        {
            Cannon* cannon = dynamic_cast<Cannon*>(collidingItem);
            qreal dx = troop->x() - collidingItem->x();
            qreal dy = troop->y() - collidingItem->y();

            if (qAbs(dx) > qAbs(dy))
            {
                if (dx < 0)
                    troop->setPos(troop->x() - 10, troop->y());
                else
                    troop->setPos(troop->x() + 10, troop->y());
            }
            else
            {
                if (dy < 0)
                    troop->setPos(troop->x(), troop->y() + 10);
                else
                    troop->setPos(troop->x(), troop->y() - 10);
            }
        }
        else if (typeid(*collidingItem) == typeid(Townhall))
        {
                Townhall* townhall = dynamic_cast<Townhall*>(collidingItem);

            if (townhall)
            {
            qreal dx = troop->x() - collidingItem->x();
            qreal dy = troop->y() - collidingItem->y();

            if (qAbs(dx) > qAbs(dy))
            {
                if (dx < 0)
                    troop->setPos(townhall->x() - 80, townhall->y());
                else
                    troop->setPos(townhall->x() + 80, townhall->y());
            }
            else
            {
                if (dy < 0)
                    troop->setPos(townhall->x(), townhall->y() - 80);
                else
                    troop->setPos(townhall->x(), townhall->y() + 80);
            }

            if (typeid(*collidingItem) == typeid(Townhall))
            {
                townhall->townhallHealth->reduceHealth(troop->troopPower);
                qDebug()<<townhall->townhallHealth->getHealth();
                if (townhall->townhallHealth->getHealth() <= 0)
                {
                itemsToRemove.append(collidingItem);
                townHallDestroyed = true;
                count++;
                }

            } }}
    }
   QTimer::singleShot(0, this, &Game::removeMarkedItems);

}
void Game::removeMarkedItems()
{
    foreach (QGraphicsItem* item, itemsToRemove)
    {
        scene->removeItem(item);
        delete item;
    }
    itemsToRemove.clear();
}

void Game::updateTimer()
{
    QTime currentTime = QTime::fromString(timerText->toPlainText(), "m:ss");

    currentTime = currentTime.addSecs(1);
    timerText->setPlainText(currentTime.toString("m:ss"));

    if (currentTime.minute() == 1 )
    {
        timer->stop();
        m_timer->stop();
        spawnTimer->stop();
        boosterTimer->stop();
        foreach (QGraphicsItem* item, scene->items())
        {
            if (typeid(*item) == typeid(Workers))
            {
                Workers* w = dynamic_cast<Workers*>(item);
                w->workerTimer->stop();
            }
        }
        foreach (QGraphicsItem* item, scene->items())
        {
            if (typeid(*item) == typeid(Workers))
            {
                Workers* w = dynamic_cast<Workers*>(item);
                w->dance();

            }
        }
        QMessageBox::information(this, "Player wins!", "Level " + QString::number(level->currLevel) + " completed succesfully!"); //!!HANDLE THE BUTTONS
        level->nextLevel(); //increment current level
        money->increaseMoney(100);
        foreach (QGraphicsItem* items, scene->items())
        {
            //clear clan design to prepare for next level
            if (typeid(*items) == typeid(Fence1) || typeid(*items) == typeid(Workers) || typeid(*items) == typeid(Troop) || typeid(*items) == typeid(Cannon) || typeid(*items) == typeid(Townhall) || typeid(*items) == typeid(Booster))
                delete items;
        }

        resetTimer();
        pause->hide();
        startLevel(); //starts next level
    }

    else if(townHallDestroyed == true){
        timer->stop();
        m_timer->stop();
        spawnTimer->stop();
        boosterTimer->stop();

        foreach (QGraphicsItem* item, scene->items())
        {
            if (typeid(*item) == typeid(Workers))
            {
                Workers* w = dynamic_cast<Workers*>(item);
                w->workerTimer->stop();
            }
        }

        foreach (QGraphicsItem* item, scene->items())
        {
            if (typeid(*item) == typeid(Troop))
            {
                Troop* t = dynamic_cast<Troop*>(item);
                t->dance();

            }
        }

        QMessageBox::information(this, "Game Over", "Game Over"); //!!HANDLE THE BUTTONS!!
         level->nextLevel(); //increment current level
        foreach (QGraphicsItem* items, scene->items())
        {
            //clear clan design to prepare for next level
            if (typeid(*items) == typeid(Fence1) || typeid(*items) == typeid(Workers) || typeid(*items) == typeid(Troop) || typeid(*items) == typeid(Cannon) || typeid(*items) == typeid(Townhall))
                delete items;
        }


        resetTimer();
        pause->hide();
        startLevel(); //restarts the level
    }

}

void Game::resetTimer()
{
    timerText->setPlainText("0:00");
}



void Game::mousePressEvent(QMouseEvent *event) //release bullet when player clicks left
{
    if (event->button() == Qt::LeftButton)
    {
        if (!gameStarted)
        {
            return;
        }

        // QPointF targetPos = event->pos();
        QPoint targetPos = QCursor::pos();
        qreal dx = targetPos.x() - cannon->pos().x();
        qreal dy = targetPos.y() - cannon->pos().y();
        qreal length = qSqrt(dx * dx + dy * dy);
        qreal directionX = dx / length;
        qreal directionY = dy / length;

        // qDebug() << "Direction X:" << directionX << "Direction Y:" << directionY;

        Bullet *bullet = new Bullet(targetPos.x(), targetPos.y(), cannon->pos().x() , cannon->pos().y());
        bullet->setDirection(directionX, directionY);

        bullet->setPos(cannon->pos().x() + 50, cannon->pos().y() + 50); //bullet comes out of cannon

        scene->addItem(bullet);

        qDebug() << "cannon power: " << cannon->power;
    }
}




void Game::showShopWindow()
{
    shopWindow->show();
}


void Game::handleSoundSettingsButton()
{
    QDialog* soundSettingsDialog = new QDialog(this);
    soundSettingsDialog->setWindowTitle("Sound Settings");

    QVBoxLayout* layout = new QVBoxLayout(soundSettingsDialog);

    QLabel* volumeLabel = new QLabel("Volume");
    QSlider* volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(int( 100));

    QPushButton* applyButton = new QPushButton("Apply");

    layout->addWidget(volumeLabel);
    layout->addWidget(volumeSlider);
    layout->addWidget(applyButton);

    connect(applyButton, &QPushButton::clicked, soundSettingsDialog, &QDialog::accept);
    connect(volumeSlider, &QSlider::valueChanged, this, [this](int value) {
        qreal volume = qreal(value) / 100.0;
        audiooutput.setVolume(volume);
    });

    if (soundSettingsDialog->exec() == QDialog::Accepted)
    {
        int volume = volumeSlider->value();
        QMessageBox::information(this, "Sound Settings", "Volume changed to " + QString::number(volume));
    }

    delete soundSettingsDialog;
    if (gamePaused)
       showPauseMenu();
}

void Game::handlePauseButton()
{
    if (!gamePaused)
    {
        qDebug()<< "pause";
        gamePaused = true;
        timer->stop();
        m_timer->stop();
        spawnTimer->stop();
        boosterTimer->stop();
        foreach (QGraphicsItem* item, scene->items())
        {
            if (typeid(*item) == typeid(Workers))
            {
                Workers* w = dynamic_cast<Workers*>(item);
                w->workerTimer->stop();
            }
        }

        showPauseMenu();

    }
    else
    {
        qDebug()<< "play";
        gamePaused = false;
        timer->start();
        m_timer->start();
        spawnTimer->start();
        boosterTimer->start();
        foreach (QGraphicsItem* item, scene->items())
        {
            if (typeid(*item) == typeid(Workers))
            {
                Workers* w = dynamic_cast<Workers*>(item);
                w->workerTimer->start();
            }
        }

    }
}

// void Game::handleQuitButton()
// {
//     /*QMessageBox* confirmQuit = new QMessageBox;
//     confirmQuit->setWindowFlags(Qt::CustomizeWindowHint);
//     confirmQuit->setText("Are you sure you would like to quit?");
//     confirmQuit->setStyleSheet("background-color: rgb(237, 230, 180); color: black;");

//     QPushButton* cancel = new QPushButton("Cancel");
//     confirmQuit->addButton(cancel, QMessageBox::ActionRole);
//     QPushButton* quit = new QPushButton("Quit");
//     confirmQuit->addButton(quit, QMessageBox::ActionRole);
//     confirmQuit->exec();

//     if (confirmQuit->clickedButton() == cancel)
//     {
//         delete confirmQuit;
//     }
//     else
//         QCoreApplication::quit()*/;
// }

void Game::handleHelpButton()
{
    QMessageBox* helpWindow = new QMessageBox;
    helpWindow->setWindowFlags(Qt::CustomizeWindowHint);
    //////////////////////////////////////////////////////////
    helpWindow->setText("(add instructions on how to play)");
    helpWindow->setStyleSheet("background-color: rgb(237, 230, 180); color: black;");

    QPushButton* OK = new QPushButton("OK");
    helpWindow->addButton(OK, QMessageBox::RejectRole);
    helpWindow->exec();

    if (gamePaused)
        showPauseMenu();

}

void Game::showPauseMenu()
{
    //still not done yet
    QMessageBox* menu = new QMessageBox;
    menu->setWindowFlags(Qt::CustomizeWindowHint);
    menu->setText("Game Options");
    menu->setStyleSheet("background-color: rgb(237, 230, 180); color: black;");

    QPushButton* resume = new QPushButton("Resume");
    menu->addButton(resume, QMessageBox::ActionRole);
    QPushButton* sound = new QPushButton("Sound Settings");
    menu->addButton(sound, QMessageBox::ActionRole);
    QPushButton* help = new QPushButton("Help");
    menu->addButton(help, QMessageBox::ActionRole);
    QPushButton* leave = new QPushButton("Back to Main Menu");
    menu->addButton(leave, QMessageBox::ActionRole);
    menu->exec();
    if (menu->clickedButton() == resume)
    {
        handlePauseButton(); //will resume
        delete menu;
    }
    if (menu->clickedButton() == sound)
        handleSoundSettingsButton();
    if (menu->clickedButton() == help)
        handleHelpButton();
    if (menu->clickedButton() == leave)
    {
        QMessageBox* confirmLeave = new QMessageBox;
        confirmLeave->setWindowFlags(Qt::CustomizeWindowHint);
        confirmLeave->setText("Are you sure you would like to return to main menu?\nYour progress in this level will not be saved");
        confirmLeave->setStyleSheet("background-color: rgb(237, 230, 180); color: black;");

        QPushButton* cancel = new QPushButton("Cancel");
        confirmLeave->addButton(cancel, QMessageBox::RejectRole);
        QPushButton* yes = new QPushButton("Leave");
        confirmLeave->addButton(yes, QMessageBox::ActionRole);
        confirmLeave->exec();

        if (confirmLeave->clickedButton() == yes)
        {
            foreach (QGraphicsItem* items, scene->items())
            {
                //clear clan design
                if (typeid(*items) == typeid(Fence1) || typeid(*items) == typeid(Workers) || typeid(*items) == typeid(Troop) || typeid(*items) == typeid(Cannon) || typeid(*items) == typeid(Townhall))
                    delete items;
            }
            resetTimer();
            pause->hide();
            startLevel(); //restarts the level
            delete confirmLeave;
        }
        else
        {
            showPauseMenu();
            delete confirmLeave;
        }

    }
}

void Game::spawnBoosters()
{
    qDebug() << "in spawnboosters";
    int spawn = randomGenerator->bounded(0,2);
    qDebug() << spawn;
    if (spawn == 0)
        return;
    else
    {
        int randomX;
        int randomY;

        do{
                randomX = randomGenerator->bounded(0, clanDesign[0].size()-1); //generating a random X and Y within scene dimensions
                randomY = randomGenerator->bounded(0, clanDesign.size()-1);
                qDebug()<< randomX << " " << randomY;
        } while (!(clanDesign[randomY][randomX] == 0 || clanDesign[randomY][randomX] == 4) ); //keeps generating until position is a 0 or 4

        Booster* booster = new Booster;
        scene->addItem(booster);
        booster->setPos(randomY * 50, randomX * 50);
    }
}




