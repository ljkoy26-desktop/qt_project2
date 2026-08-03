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

    // strFilePath 이미지를 현재 로드된 타일 시퀀스 뒤에 이어붙인다.
    // 기존 타일 크기와 세로 크기가 일치해야 하며, 기존에 로드된 이미지가 없으면 실패한다.
    // 실패 시 false를 반환하고 pStrError에 사유를 채운다.
    bool AppendFromFile(const QString &strFilePath, QString *pStrError);

    // strFolderPath 폴더 안의 png/bmp 이미지들을 파일명 자연 정렬(1,2,...,10) 순으로 읽어
    // 하나의 타일 시퀀스로 합친다. 모든 이미지는 정사각형이며 크기가 동일해야 하고,
    // 지원 타일 크기는 16x16 또는 64x64뿐이다. 실패 시 false를 반환하고 pStrError에 사유를 채운다.
    bool LoadFromFolder(const QString &strFolderPath, QString *pStrError);

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
