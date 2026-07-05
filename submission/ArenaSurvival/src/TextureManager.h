#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <QPixmap>
#include <QHash>
#include <QString>

/// Singleton that loads & caches textures from the pictures/ folder.
/// Falls back to an empty pixmap if the file doesn't exist —
/// callers should check isValid() and draw the original shape instead.
class TextureManager
{
public:
    static TextureManager &instance();

    /// Load (or return cached) texture.  Returns invalid QPixmap if missing.
    const QPixmap &texture(const QString &filename);

    /// Generate default placeholder textures if they don't exist
    void generateDefaultTextures();

private:
    TextureManager() = default;
    QHash<QString, QPixmap> m_cache;
};

#endif // TEXTUREMANAGER_H
