#ifndef EXPERIENCEORB_H
#define EXPERIENCEORB_H

#include <QGraphicsEllipseItem>
#include "Constants.h"

class Player;

class ExperienceOrb : public QGraphicsEllipseItem
{
public:
    ExperienceOrb(QPointF position, int xpValue, Player *player,
                  QGraphicsItem *parent = nullptr);

    void advance(int phase) override;

    int  xpValue()  const { return m_xpValue; }
    bool isDead()   const { return m_dead; }
    void setDead(bool dead) { m_dead = dead; }

    enum { Type = GameConstants::ExperienceOrbType };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

private:
    int     m_xpValue;
    Player *m_player;
    double  m_magnetRange;
    double  m_speed;
    qreal   m_radius;
    bool    m_dead;
};

#endif // EXPERIENCEORB_H
