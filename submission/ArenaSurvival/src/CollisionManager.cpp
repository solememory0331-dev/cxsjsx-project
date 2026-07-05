#include "CollisionManager.h"
#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "ExperienceOrb.h"

#include <QGraphicsScene>

void CollisionManager::processCollisions(
    QGraphicsScene *scene,
    Player *player,
    QList<Bullet*> &bullets,
    QList<Enemy*> &enemies,
    QList<ExperienceOrb*> &orbs,
    int &scoreOut)
{
    handleBulletEnemy(scene, bullets, enemies, scoreOut);
    handleEnemyPlayer(scene, player, enemies);
    handlePlayerOrb(scene, player, orbs);
}

void CollisionManager::handleBulletEnemy(
    QGraphicsScene *scene,
    QList<Bullet*> &bullets,
    QList<Enemy*> &enemies,
    int &scoreOut)
{
    for (auto *bullet : bullets) {
        if (bullet->isDead() || bullet->isExpired())
            continue;

        QList<QGraphicsItem*> hits = scene->collidingItems(
            bullet, Qt::IntersectsItemShape);

        for (auto *hit : hits) {
            if (hit->type() == GameConstants::EnemyType) {
                Enemy *enemy = static_cast<Enemy*>(hit);
                if (enemy->isDead()) continue;

                enemy->takeDamage(bullet->damage());
                bullet->setDead(true);

                // Score only added when enemy actually dies (handled in Game::cleanupDeadItems)
                break;  // one bullet = one hit
            }
        }
    }
}

void CollisionManager::handleEnemyPlayer(
    QGraphicsScene *scene,
    Player *player,
    QList<Enemy*> &enemies)
{
    if (player->isInvincible()) return;

    QList<QGraphicsItem*> hits = scene->collidingItems(
        player, Qt::IntersectsItemShape);

    for (auto *hit : hits) {
        if (hit->type() == GameConstants::EnemyType) {
            Enemy *enemy = static_cast<Enemy*>(hit);
            if (enemy->isDead()) continue;

            player->takeDamage(enemy->damageVal());
            // takeDamage already triggers invincibility
            break;  // one collision triggers invincibility
        }
    }
}

void CollisionManager::handlePlayerOrb(
    QGraphicsScene *scene,
    Player *player,
    QList<ExperienceOrb*> &orbs)
{
    QList<QGraphicsItem*> hits = scene->collidingItems(
        player, Qt::IntersectsItemShape);

    for (auto *hit : hits) {
        if (hit->type() == GameConstants::ExperienceOrbType) {
            ExperienceOrb *orb = static_cast<ExperienceOrb*>(hit);
            if (orb->isDead()) continue;

            player->addXP(orb->xpValue());
            orb->setDead(true);
            // Collect all touching orbs in one frame
        }
    }
}
