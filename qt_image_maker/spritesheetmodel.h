#ifndef SPRITESHEETMODEL_H
#define SPRITESHEETMODEL_H

#include <QImage>
#include <QString>
#include <QVector>

// 가로 1×N 타일 스트립(예: 16x16 타일 10개 = 160x16) 형태의
// 스프라이트시트를 불러오고, 타일 단위로 잘라 보관하며, 재조립하는 데이터 모델.
// Qt 위젯에 의존하지 않는다.
class SpriteSheetModel
{
public:
    SpriteSheetModel();

    // strFilePath 이미지를 읽어 타일로 분할한다.
    // 지원 타일 세로 크기는 16 또는 64뿐이며, 가로는 타일 크기의 배수여야 한다.
    // 실패 시 false를 반환하고 pStrError에 사유를 채운다.
    bool LoadFromFile(const QString &strFilePath, QString *pStrError);

    // 현재 보관 중인 타일들을 순서대로 가로로 이어붙여 하나의 이미지로 재조립한다.
    QImage BuildStripImage() const;

    void SetTiles(const QVector<QImage> &vecTiles);

    const QVector<QImage>& Tiles() const;
    int TileSize() const;
    int TileCount() const;
    bool IsEmpty() const;
    const QString& SourcePath() const;

    void Clear();

private:
    QVector<QImage> m_vecTiles;
    int             m_nTileSize;
    QString         m_strSourcePath;
};

#endif // SPRITESHEETMODEL_H
