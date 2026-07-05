#ifndef COLLISIONMANAGER_H
#define COLLISIONMANAGER_H

#include <QList>

class QGraphicsScene;
class Player;
class Bullet;
class Enemy;
class ExperienceOrb;

class CollisionManager
{
public:
    static void processCollisions(
        QGraphicsScene *scene,
        Player *player,
        QList<Bullet*> &bullets,
        QList<Enemy*> &enemies,
        QList<ExperienceOrb*> &orbs,
        int &scoreOut);

private:
    static void handleBulletEnemy(QGraphicsScene *scene,
                                  QList<Bullet*> &bullets,
                                  QList<Enemy*> &enemies,
                                  int &scoreOut);
    static void handleEnemyPlayer(QGraphicsScene *scene,
                                  Player *player,
                                  QList<Enemy*> &enemies);
    static void handlePlayerOrb(QGraphicsScene *scene,
                                Player *player,
                                QList<ExperienceOrb*> &orbs);
};

#endif // COLLISIONMANAGER_H
