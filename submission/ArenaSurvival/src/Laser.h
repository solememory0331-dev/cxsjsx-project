#ifndef LASER_H
#define LASER_H

#include <QGraphicsLineItem>
#include <QPointF>
#include <QList>
#include "Constants.h"

class Enemy;

class Laser : public QGraphicsLineItem
{
public:
    Laser(QPointF startPos, QPointF aimPoint, int damage, double beamWidth,
          QGraphicsItem *parent = nullptr);

    /// Call after adding to scene — checks enemies along the beam
    void checkHit(const QList<Enemy*> &enemies);

    int  damage()   const { return m_damage; }
    bool isDead()    const { return m_dead; }
    bool isExpired() const;
    void setDead(bool dead) { m_dead = dead; }

    enum { Type = GameConstants::LaserType };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

private:
    int    m_damage;
    double m_beamWidth;
    qint64 m_createdAt;
    bool   m_dead;

    /// Point-to-segment distance helper
    static double pointToSegmentDist(QPointF p, QPointF a, QPointF b);
};

#endif // LASER_H
