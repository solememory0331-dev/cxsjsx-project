#ifndef BULLET_H
#define BULLET_H

#include <QGraphicsEllipseItem>
#include <QVector2D>
#include <QPointF>
#include "Constants.h"

class Bullet : public QGraphicsEllipseItem
{
public:
    Bullet(QPointF startPos, QPointF targetPos, int damage,
           QGraphicsItem *parent = nullptr);

    void advance(int phase) override;

    int  damage()    const { return m_damage; }
    bool isExpired() const;
    bool isDead()    const { return m_dead; }
    void setDead(bool dead) { m_dead = dead; }

    enum { Type = GameConstants::BulletType };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

private:
    QVector2D m_direction;
    double    m_speed;
    int       m_damage;
    qint64    m_createdAt;
    QPointF   m_startPos;
    double    m_maxDistance;
    qreal     m_radius;
    bool      m_dead;
};

#endif // BULLET_H
