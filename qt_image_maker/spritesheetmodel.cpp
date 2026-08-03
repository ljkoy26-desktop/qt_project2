#include "spritesheetmodel.h"

#include <QPainter>

SpriteSheetModel::SpriteSheetModel()
    : m_nTileSize(0)
{
}

bool SpriteSheetModel::LoadFromFile(const QString &strFilePath, QString *pStrError)
{
    QImage image;
    if (!image.load(strFilePath))
    {
        if (pStrError != nullptr)
        {
            *pStrError = QStringLiteral("이미지 파일을 열 수 없습니다: %1").arg(strFilePath);
        }
        return false;
    }

    int nTileSize = 0;
    if (image.height() == 16)
    {
        nTileSize = 16;
    }
    else if (image.height() == 64)
    {
        nTileSize = 64;
    }
    else
    {
        if (pStrError != nullptr)
        {
            *pStrError = QStringLiteral("지원하는 타일 세로 크기는 16 또는 64뿐입니다. (현재 이미지 세로: %1)").arg(image.height());
        }
        return false;
    }

    if ((image.width() % nTileSize) != 0)
    {
        if (pStrError != nullptr)
        {
            *pStrError = QStringLiteral("이미지 가로 크기(%1)가 타일 크기(%2)의 배수가 아닙니다.").arg(image.width()).arg(nTileSize);
        }
        return false;
    }

    QImage imageArgb = image.convertToFormat(QImage::Format_ARGB32);

    int nCount = imageArgb.width() / nTileSize;

    QVector<QImage> vecTiles;
    vecTiles.reserve(nCount);

    for (int i = 0; i < nCount; ++i)
    {
        QRect rectTile(i * nTileSize, 0, nTileSize, nTileSize);
        vecTiles.append(imageArgb.copy(rectTile));
    }

    m_vecTiles = vecTiles;
    m_nTileSize = nTileSize;
    m_strSourcePath = strFilePath;

    return true;
}

QImage SpriteSheetModel::BuildStripImage() const
{
    if (m_vecTiles.isEmpty())
    {
        return QImage();
    }

    int nWidth = m_nTileSize * m_vecTiles.size();

    QImage imageResult(nWidth, m_nTileSize, QImage::Format_ARGB32);
    imageResult.fill(Qt::transparent);

    QPainter painter(&imageResult);
    for (int i = 0; i < m_vecTiles.size(); ++i)
    {
        painter.drawImage(i * m_nTileSize, 0, m_vecTiles.at(i));
    }
    painter.end();

    return imageResult;
}

void SpriteSheetModel::SetTiles(const QVector<QImage> &vecTiles)
{
    m_vecTiles = vecTiles;
}

const QVector<QImage>& SpriteSheetModel::Tiles() const
{
    return m_vecTiles;
}

int SpriteSheetModel::TileSize() const
{
    return m_nTileSize;
}

int SpriteSheetModel::TileCount() const
{
    return m_vecTiles.size();
}

bool SpriteSheetModel::IsEmpty() const
{
    return m_vecTiles.isEmpty();
}

const QString& SpriteSheetModel::SourcePath() const
{
    return m_strSourcePath;
}

void SpriteSheetModel::Clear()
{
    m_vecTiles.clear();
    m_nTileSize = 0;
    m_strSourcePath.clear();
}
