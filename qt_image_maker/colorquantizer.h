#ifndef COLORQUANTIZER_H
#define COLORQUANTIZER_H

#include <QImage>
#include <QVector>

// 미디언컷(median-cut) 알고리즘으로 이미지에서 최적 팔레트를 자동 계산하는 유틸리티.
// 16색/256색 인덱스 BMP로 내보낼 때 사용한다.
struct QuantizeResult
{
    QVector<QRgb> vecPalette;       // 계산된 팔레트 (최대 nMaxColors개)
    QVector<int>  vecPixelIndices;  // width*height, 픽셀별 팔레트 인덱스 (row-major)
};

class ColorQuantizer
{
public:
    // image를 nMaxColors개 이하의 팔레트로 양자화한다.
    static QuantizeResult Quantize(const QImage &image, int nMaxColors);
};

#endif // COLORQUANTIZER_H
