#ifndef MAGICCIRCLE_H
#define MAGICCIRCLE_H

#include <QGraphicsEllipseItem>
#include <QList>
#include "Constants.h"

class Player;
class Enemy;

class MagicCircle : public QGraphicsEllipseItem
{
public:
    MagicCircle(Player *player, QGraphicsItem *parent = nullptr);

    void updatePosition();
    void checkDamage(const QList<Enemy*> &enemies);

    double radius() const { return m_radius; }
    int    damage() const { return m_damage; }

    void increaseRadius(double amount);
    void increaseDamage(int amount);

    bool isDead() const { return m_dead; }

    enum { Type = GameConstants::MagicCircleType };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

private:
    Player *m_player;
    double  m_radius;
    int     m_damage;
    qint64  m_lastDamageTime;
    bool    m_dead;
};

#endif // MAGICCIRCLE_H
