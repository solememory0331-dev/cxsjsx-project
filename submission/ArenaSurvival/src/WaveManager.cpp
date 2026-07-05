#include "WaveManager.h"
#include "Enemy.h"
#include "Player.h"
#include "Constants.h"

#include <QRandomGenerator>
#include <QDateTime>

WaveManager::WaveManager(QGraphicsScene *scene, Player *player,
                         QObject *parent)
    : QObject(parent)
    , m_scene(scene)
    , m_player(player)
    , m_wave(0)
    , m_enemiesRemainingToSpawn(0)
    , m_spawnInterval(GameConstants::BASE_SPAWN_INTERVAL)
    , m_lastSpawnTime(0)
    , m_diff(difficultyParams(Difficulty::Normal))
{
}

void WaveManager::startWave(int waveNumber)
{
    m_wave = waveNumber;
    m_enemiesRemainingToSpawn = enemiesForWave(waveNumber);
    m_spawnInterval = spawnIntervalForWave(waveNumber);
    m_lastSpawnTime = QDateTime::currentMSecsSinceEpoch();

    // Boss every BOSS_WAVE_INTERVAL waves
    if (waveNumber % GameConstants::BOSS_WAVE_INTERVAL == 0) {
        TankEnemy *boss = new TankEnemy(m_player, waveNumber);
        boss->setHP(static_cast<int>(
            GameConstants::ENEMY_BASE_HP
            * (1.0 + (waveNumber - 1) * GameConstants::HP_SCALE_PER_WAVE)
            * GameConstants::TANK_ENEMY_HP_MUL * 3.0
            * m_diff.enemyHpMult));
        boss->setDamageVal(static_cast<int>(
            GameConstants::ENEMY_BASE_DAMAGE * 2 * m_diff.enemyDamageMult));
        boss->setSpeedVal(boss->speed() * m_diff.enemySpeedMult);
        boss->setScoreValue(GameConstants::ENEMY_SCORE_VALUE * 5);
        boss->setXPValue(GameConstants::XP_PER_KILL * 5);
        // Boss gets bigger
        boss->QGraphicsItem::setScale(1.8);

        QPointF bossPos = randomEdgePosition();
        boss->setPos(bossPos);
        m_scene->addItem(boss);
        m_enemiesRemainingToSpawn--;
        emit enemySpawned(boss);
    }
}

void WaveManager::update()
{
    if (m_enemiesRemainingToSpawn <= 0) return;

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastSpawnTime >= m_spawnInterval) {
        spawnEnemy();
        m_lastSpawnTime = now;
    }
}

bool WaveManager::isWaveComplete() const
{
    return m_enemiesRemainingToSpawn <= 0;
}

void WaveManager::spawnEnemy()
{
    Enemy *enemy = createEnemyByDistribution();
    enemy->setPos(randomEdgePosition());

    // Apply difficulty multipliers
    enemy->setHP(static_cast<int>(enemy->hp() * m_diff.enemyHpMult));
    enemy->setDamageVal(static_cast<int>(enemy->damageVal() * m_diff.enemyDamageMult));
    enemy->setSpeedVal(enemy->speed() * m_diff.enemySpeedMult);

    m_scene->addItem(enemy);
    m_enemiesRemainingToSpawn--;
    emit enemySpawned(enemy);
}

QPointF WaveManager::randomEdgePosition() const
{
    double W = GameConstants::SCENE_WIDTH;
    double H = GameConstants::SCENE_HEIGHT;
    double M = GameConstants::SPAWN_MARGIN;

    auto *rng = QRandomGenerator::global();
    int edge = rng->bounded(4);
    double x, y;

    switch (edge) {
    case 0: // Top
        x = rng->bounded(W);
        y = -M;
        break;
    case 1: // Right
        x = W + M;
        y = rng->bounded(H);
        break;
    case 2: // Bottom
        x = rng->bounded(W);
        y = H + M;
        break;
    case 3: // Left
    default:
        x = -M;
        y = rng->bounded(H);
        break;
    }
    return QPointF(x, y);
}

Enemy *WaveManager::createEnemyByDistribution()
{
    auto *rng = QRandomGenerator::global();
    int roll = rng->bounded(100);

    if (m_wave >= 5 && roll < 10) {
        return new TankEnemy(m_player, m_wave);
    }

    if (m_wave >= 3) {
        int fastChance = qMin(10 + m_wave * 5, 40);
        if (roll < fastChance) {
            return new FastEnemy(m_player, m_wave);
        }
    }

    return new NormalEnemy(m_player, m_wave);
}

int WaveManager::enemiesForWave(int wave) const
{
    int base = GameConstants::BASE_ENEMIES_PER_WAVE
               + wave * GameConstants::EXTRA_ENEMIES_PER_WAVE;
    return qMax(1, static_cast<int>(base * m_diff.enemyCountMult));
}

int WaveManager::spawnIntervalForWave(int wave) const
{
    int interval = GameConstants::BASE_SPAWN_INTERVAL - (wave - 1) * 50;
    return qMax(interval, GameConstants::MIN_SPAWN_INTERVAL);
}
