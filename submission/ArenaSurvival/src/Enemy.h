#ifndef ENEMY_H
#define ENEMY_H

#include <QGraphicsEllipseItem>
#include <QObject>
#include "Constants.h"

class Player;

class Enemy : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT

public:
    enum class EnemyType { Normal, Fast, Tank, Boss };

    explicit Enemy(Player *target, QGraphicsItem *parent = nullptr);
    virtual ~Enemy();

    virtual void advance(int phase) override;

    void takeDamage(int amount);
    bool isDead() const { return m_hp <= 0; }

    int   hp()         const { return m_hp; }
    int   maxHp()      const { return m_maxHp; }
    int   damageVal()  const { return m_damage; }
    double speed()     const { return m_speed; }
    int   scoreValue() const { return m_scoreValue; }
    int   xpValue()    const { return m_xpValue; }
    EnemyType enemyType() const { return m_type; }

    // For difficulty / Boss override
    void setHP(int hp);
    void setDamageVal(int dmg);
    void setSpeedVal(double spd);
    void setScoreValue(int val);
    void setXPValue(int val);
    void setScale(qreal scale);

    enum { Type = GameConstants::EnemyType };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

signals:
    void enemyDied(Enemy *enemy);

protected:
    Player   *m_target;
    int       m_hp;
    int       m_maxHp;
    double    m_speed;
    int       m_damage;
    int       m_scoreValue;
    int       m_xpValue;
    qreal     m_radius;
    QColor    m_color;
    EnemyType m_type;
};

// ── Concrete subtypes ─────────────────────────────────────────

class NormalEnemy : public Enemy
{
public:
    NormalEnemy(Player *target, int wave, QGraphicsItem *parent = nullptr);
};

class FastEnemy : public Enemy
{
public:
    FastEnemy(Player *target, int wave, QGraphicsItem *parent = nullptr);
};

class TankEnemy : public Enemy
{
public:
    TankEnemy(Player *target, int wave, QGraphicsItem *parent = nullptr);
};

#endif // ENEMY_H
