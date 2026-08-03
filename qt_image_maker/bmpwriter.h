#ifndef BMPWRITER_H
#define BMPWRITER_H

#include <QImage>
#include <QString>
#include <QVector>

// Qt의 QImage::save()는 4bpp(16색) BMP 쓰기를 지원하지 않으므로,
// 그림판의 "16색 비트맵" 저장 옵션과 호환되는 표준 4bpp 인덱스 BMP를 직접 작성하는 유틸리티.
class BmpWriter
{
public:
    // vecPixelIndices: width*height 크기, row-major(위→아래), 값 범위 0~15
    // vecPalette: 팔레트 색상(최대 16개, 부족하면 검정으로 채움)
    static bool WriteIndexed4Bpp(const QString &strFilePath,
                                  int nWidth,
                                  int nHeight,
                                  const QVector<int> &vecPixelIndices,
                                  const QVector<QRgb> &vecPalette,
                                  QString *pStrError);
};

#endif // BMPWRITER_H
