#ifndef WAVEMANAGER_H
#define WAVEMANAGER_H

#include <QObject>
#include <QGraphicsScene>
#include <QPointF>
#include "Constants.h"

class Player;
class Enemy;

class WaveManager : public QObject
{
    Q_OBJECT

public:
    explicit WaveManager(QGraphicsScene *scene, Player *player,
                         QObject *parent = nullptr);

    void setPlayer(Player *player) { m_player = player; }
    void setDifficultyParams(const DifficultyParams &p) { m_diff = p; }

    void startWave(int waveNumber);
    void update();
    bool isWaveComplete() const;

    int currentWave()            const { return m_wave; }
    int enemiesRemainingToSpawn() const { return m_enemiesRemainingToSpawn; }

signals:
    void enemySpawned(Enemy *enemy);
    void allEnemiesDefeated();

private:
    void spawnEnemy();
    QPointF randomEdgePosition() const;
    Enemy *createEnemyByDistribution();
    int enemiesForWave(int wave) const;
    int spawnIntervalForWave(int wave) const;

    QGraphicsScene *m_scene;
    Player         *m_player;
    int             m_wave;
    int             m_enemiesRemainingToSpawn;
    int             m_spawnInterval;
    qint64          m_lastSpawnTime;
    DifficultyParams m_diff;
};

#endif // WAVEMANAGER_H
