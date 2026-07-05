#include "TextureManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QRadialGradient>
#include <cmath>

TextureManager &TextureManager::instance()
{
    static TextureManager mgr;
    return mgr;
}

const QPixmap &TextureManager::texture(const QString &filename)
{
    if (m_cache.contains(filename))
        return m_cache[filename];

    // Search paths: project root first (user textures), exe dir second (defaults)
    QStringList searchPaths;
    searchPaths << QCoreApplication::applicationDirPath() + "/../pictures/" + filename
                << QCoreApplication::applicationDirPath() + "/../../pictures/" + filename
                << QCoreApplication::applicationDirPath() + "/pictures/" + filename
                << "pictures/" + filename;

    for (const auto &path : searchPaths) {
        if (QFileInfo::exists(path)) {
            QPixmap px;
            px.load(path);
            if (!px.isNull()) {
                m_cache[filename] = px;
                return m_cache[filename];
            }
        }
    }

    // Not found — insert an empty pixmap so we don't keep searching
    m_cache[filename] = QPixmap();
    return m_cache[filename];
}

void TextureManager::generateDefaultTextures()
{
    QString outDir = QCoreApplication::applicationDirPath() + "/pictures";
    QDir().mkpath(outDir);

    struct TexDef {
        QString filename;
        int     size;
        QColor  color1;
        QColor  color2;
    };

    QList<TexDef> defs = {
        { "player.png",          64, QColor(60, 180, 255), QColor(30, 80, 200)   },
        { "enemy_normal.png",    64, QColor(220, 60, 60),  QColor(140, 20, 20)   },
        { "enemy_fast.png",      64, QColor(220, 120, 40), QColor(160, 60, 10)   },
        { "enemy_tank.png",      64, QColor(140, 40, 140), QColor(70, 10, 70)    },
        { "bullet.png",          32, QColor(255, 220, 60), QColor(255, 160, 20)  },
        { "experience_orb.png",  32, QColor(100, 255, 100),QColor(20, 180, 20)   },
    };

    for (const auto &d : defs) {
        QString path = outDir + "/" + d.filename;
        if (QFileInfo::exists(path)) continue;  // don't overwrite user textures

        QPixmap px(d.size, d.size);
        px.fill(Qt::transparent);

        QPainter p(&px);
        p.setRenderHint(QPainter::Antialiasing);

        // Radial gradient for a 3D sphere look
        QRadialGradient grad(d.size * 0.35, d.size * 0.35, d.size * 0.55);
        grad.setColorAt(0.0, d.color1.lighter(160));
        grad.setColorAt(0.6, d.color1);
        grad.setColorAt(1.0, d.color2);

        p.setBrush(grad);
        p.setPen(QPen(d.color2, d.size * 0.04));
        p.drawEllipse(d.size * 0.08, d.size * 0.08,
                      d.size * 0.84, d.size * 0.84);

        // Small highlight
        p.setBrush(QColor(255, 255, 255, 100));
        p.setPen(Qt::NoPen);
        p.drawEllipse(d.size * 0.25, d.size * 0.18,
                      d.size * 0.25, d.size * 0.22);

        p.end();
        px.save(path, "PNG");
    }

    // ── Magic circle: ring texture ────────────────────────
    QString mcPath = outDir + "/magic_circle.png";
    if (!QFileInfo::exists(mcPath)) {
        int sz = 256;
        QPixmap px(sz, sz);
        px.fill(Qt::transparent);
        QPainter p(&px);
        p.setRenderHint(QPainter::Antialiasing);

        double margin = sz * 0.06;
        QRectF r(margin, margin, sz - 2 * margin, sz - 2 * margin);

        // Outer glow ring
        QPen outerPen(QColor(100, 200, 255, 80), sz * 0.04);
        p.setPen(outerPen);
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(r);

        // Inner bright ring
        QPen innerPen(QColor(150, 220, 255, 180), sz * 0.025);
        p.setPen(innerPen);
        p.drawEllipse(r.adjusted(sz * 0.02, sz * 0.02,
                                  -sz * 0.02, -sz * 0.02));

        // Rune dots at 4 corners
        p.setBrush(QColor(200, 240, 255, 200));
        p.setPen(Qt::NoPen);
        double cx = sz / 2.0, cy = sz / 2.0, rad = r.width() / 2.0;
        for (int i = 0; i < 4; i++) {
            double angle = i * 3.14159 / 2.0 + 0.785;  // 45° offset
            double dx = std::cos(angle) * rad * 0.88;
            double dy = std::sin(angle) * rad * 0.88;
            p.drawEllipse(QPointF(cx + dx, cy + dy), sz * 0.03, sz * 0.03);
        }

        p.end();
        px.save(mcPath, "PNG");
    }
}
