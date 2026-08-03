#include "spritesheetmodel.h"

#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <algorithm>

namespace
{
    // 파일명을 자연 정렬(숫자 구간을 값으로 비교)로 비교한다. 예: "2.png" < "10.png"
    bool NaturalLessThan(const QString &strLeft, const QString &strRight)
    {
        int i = 0;
        int j = 0;

        while (i < strLeft.length() && j < strRight.length())
        {
            QChar chLeft = strLeft.at(i);
            QChar chRight = strRight.at(j);

            if (chLeft.isDigit() && chRight.isDigit())
            {
                int nStartI = i;
                while (i < strLeft.length() && strLeft.at(i).isDigit())
                {
                    ++i;
                }

                int nStartJ = j;
                while (j < strRight.length() && strRight.at(j).isDigit())
                {
                    ++j;
                }

                QString strNumLeft = strLeft.mid(nStartI, i - nStartI);
                QString strNumRight = strRight.mid(nStartJ, j - nStartJ);

                bool bOkLeft = false;
                bool bOkRight = false;
                qulonglong nValLeft = strNumLeft.toULongLong(&bOkLeft);
                qulonglong nValRight = strNumRight.toULongLong(&bOkRight);

                if (bOkLeft && bOkRight && nValLeft != nValRight)
                {
                    return nValLeft < nValRight;
                }
            }
            else
            {
                if (chLeft != chRight)
                {
                    return chLeft < chRight;
                }

                ++i;
                ++j;
            }
        }

        return (strLeft.length() - i) < (strRight.length() - j);
    }

    // ARGB32로 변환된 가로 1×N 스트립 이미지를 nTileSize 정사각형 타일들로 잘라낸다.
    QVector<QImage> SliceIntoTiles(const QImage &imageArgb, int nTileSize)
    {
        int nCount = imageArgb.width() / nTileSize;

        QVector<QImage> vecTiles;
        vecTiles.reserve(nCount);

        for (int i = 0; i < nCount; ++i)
        {
            QRect rectTile(i * nTileSize, 0, nTileSize, nTileSize);
            vecTiles.append(imageArgb.copy(rectTile));
        }

        return vecTiles;
    }
}

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

    m_vecTiles = SliceIntoTiles(imageArgb, nTileSize);
    m_nTileSize = nTileSize;
    m_strSourcePath = strFilePath;

    return true;
}

bool SpriteSheetModel::AppendFromFile(const QString &strFilePath, QString *pStrError)
{
    if (m_nTileSize == 0)
    {
        if (pStrError != nullptr)
        {
            *pStrError = QStringLiteral("이어붙일 기존 이미지가 없습니다.");
        }
        return false;
    }

    QImage image;
    if (!image.load(strFilePath))
    {
        if (pStrError != nullptr)
        {
            *pStrError = QStringLiteral("이미지 파일을 열 수 없습니다: %1").arg(strFilePath);
        }
        return false;
    }

    if (image.height() != m_nTileSize)
    {
        if (pStrError != nullptr)
        {
            *pStrError = QStringLiteral("기존 타일 크기(%1)와 세로 크기가 다릅니다. (현재 이미지 세로: %2)").arg(m_nTileSize).arg(image.height());
        }
        return false;
    }

    if ((image.width() % m_nTileSize) != 0)
    {
        if (pStrError != nullptr)
        {
            *pStrError = QStringLiteral("이미지 가로 크기(%1)가 타일 크기(%2)의 배수가 아닙니다.").arg(image.width()).arg(m_nTileSize);
        }
        return false;
    }

    QImage imageArgb = image.convertToFormat(QImage::Format_ARGB32);
    m_vecTiles += SliceIntoTiles(imageArgb, m_nTileSize);

    return true;
}

bool SpriteSheetModel::LoadFromFolder(const QString &strFolderPath, QString *pStrError)
{
    QDir oDir(strFolderPath);
    if (!oDir.exists())
    {
        if (pStrError != nullptr)
        {
            *pStrError = QStringLiteral("폴더를 찾을 수 없습니다: %1").arg(strFolderPath);
        }
        return false;
    }

    QStringList listFilters;
    listFilters << QStringLiteral("*.png") << QStringLiteral("*.bmp");

    QFileInfoList listFiles = oDir.entryInfoList(listFilters, QDir::Files);
    if (listFiles.isEmpty())
    {
        if (pStrError != nullptr)
        {
            *pStrError = QStringLiteral("폴더에 png/bmp 이미지가 없습니다: %1").arg(strFolderPath);
        }
        return false;
    }

    std::sort(listFiles.begin(), listFiles.end(),
        [](const QFileInfo &oLeft, const QFileInfo &oRight)
        {
            return NaturalLessThan(oLeft.fileName(), oRight.fileName());
        });

    QVector<QImage> vecTiles;
    vecTiles.reserve(listFiles.size());

    int nTileSize = 0;

    for (const QFileInfo &oFileInfo : listFiles)
    {
        QImage image;
        if (!image.load(oFileInfo.absoluteFilePath()))
        {
            if (pStrError != nullptr)
            {
                *pStrError = QStringLiteral("이미지를 읽을 수 없습니다: %1").arg(oFileInfo.fileName());
            }
            return false;
        }

        if (image.width() != image.height())
        {
            if (pStrError != nullptr)
            {
                *pStrError = QStringLiteral("정사각형 타일 이미지가 아닙니다: %1 (%2x%3)").arg(oFileInfo.fileName()).arg(image.width()).arg(image.height());
            }
            return false;
        }

        if (nTileSize == 0)
        {
            nTileSize = image.width();
            if (nTileSize != 16 && nTileSize != 64)
            {
                if (pStrError != nullptr)
                {
                    *pStrError = QStringLiteral("지원하는 타일 크기는 16x16 또는 64x64뿐입니다. (%1: %2x%2)").arg(oFileInfo.fileName()).arg(nTileSize);
                }
                return false;
            }
        }
        else if (image.width() != nTileSize)
        {
            if (pStrError != nullptr)
            {
                *pStrError = QStringLiteral("이미지 크기가 서로 다릅니다: %1 (%2x%2, 앞서 읽은 크기: %3x%3)").arg(oFileInfo.fileName()).arg(image.width()).arg(nTileSize);
            }
            return false;
        }

        vecTiles.append(image.convertToFormat(QImage::Format_ARGB32));
    }

    m_vecTiles = vecTiles;
    m_nTileSize = nTileSize;
    m_strSourcePath = strFolderPath;

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
