#include "colorquantizer.h"

#include <QHash>
#include <algorithm>

namespace
{
    struct ColorCount
    {
        QRgb   rgbColor;
        qint64 nCount;
    };

    struct Bucket
    {
        QVector<ColorCount> vecColors;
        qint64               nTotalCount;
    };

    // 버킷 내에서 R/G/B 중 값의 범위가 가장 큰 채널을 찾는다. (0=R, 1=G, 2=B)
    int WidestChannel(const Bucket &bucket)
    {
        int nMinR = 255;
        int nMaxR = 0;
        int nMinG = 255;
        int nMaxG = 0;
        int nMinB = 255;
        int nMaxB = 0;

        for (const ColorCount &oColorCount : bucket.vecColors)
        {
            int nR = qRed(oColorCount.rgbColor);
            int nG = qGreen(oColorCount.rgbColor);
            int nB = qBlue(oColorCount.rgbColor);

            nMinR = qMin(nMinR, nR);
            nMaxR = qMax(nMaxR, nR);
            nMinG = qMin(nMinG, nG);
            nMaxG = qMax(nMaxG, nG);
            nMinB = qMin(nMinB, nB);
            nMaxB = qMax(nMaxB, nB);
        }

        int nRangeR = nMaxR - nMinR;
        int nRangeG = nMaxG - nMinG;
        int nRangeB = nMaxB - nMinB;

        if (nRangeR >= nRangeG && nRangeR >= nRangeB)
        {
            return 0;
        }
        else if (nRangeG >= nRangeB)
        {
            return 1;
        }

        return 2;
    }

    void SortBucketByChannel(Bucket &bucket, int nChannel)
    {
        std::sort(bucket.vecColors.begin(), bucket.vecColors.end(),
            [nChannel](const ColorCount &oLeft, const ColorCount &oRight)
            {
                int nLeftValue = 0;
                int nRightValue = 0;

                if (nChannel == 0)
                {
                    nLeftValue = qRed(oLeft.rgbColor);
                    nRightValue = qRed(oRight.rgbColor);
                }
                else if (nChannel == 1)
                {
                    nLeftValue = qGreen(oLeft.rgbColor);
                    nRightValue = qGreen(oRight.rgbColor);
                }
                else
                {
                    nLeftValue = qBlue(oLeft.rgbColor);
                    nRightValue = qBlue(oRight.rgbColor);
                }

                return nLeftValue < nRightValue;
            });
    }

    QRgb AverageColor(const QVector<ColorCount> &vecColors)
    {
        quint64 nSumR = 0;
        quint64 nSumG = 0;
        quint64 nSumB = 0;
        quint64 nTotal = 0;

        for (const ColorCount &oColorCount : vecColors)
        {
            nSumR += (quint64)qRed(oColorCount.rgbColor) * (quint64)oColorCount.nCount;
            nSumG += (quint64)qGreen(oColorCount.rgbColor) * (quint64)oColorCount.nCount;
            nSumB += (quint64)qBlue(oColorCount.rgbColor) * (quint64)oColorCount.nCount;
            nTotal += (quint64)oColorCount.nCount;
        }

        if (nTotal == 0)
        {
            return qRgb(0, 0, 0);
        }

        return qRgb((int)(nSumR / nTotal), (int)(nSumG / nTotal), (int)(nSumB / nTotal));
    }
}

QuantizeResult ColorQuantizer::Quantize(const QImage &image, int nMaxColors)
{
    QuantizeResult oResult;

    if (image.isNull() || nMaxColors <= 0)
    {
        return oResult;
    }

    QImage imageArgb = image.convertToFormat(QImage::Format_ARGB32);
    int nWidth = imageArgb.width();
    int nHeight = imageArgb.height();

    QHash<QRgb, qint64> mapHistogram;
    mapHistogram.reserve(4096);

    for (int y = 0; y < nHeight; ++y)
    {
        const QRgb *pLine = reinterpret_cast<const QRgb*>(imageArgb.constScanLine(y));
        for (int x = 0; x < nWidth; ++x)
        {
            mapHistogram[pLine[x]] += 1;
        }
    }

    QVector<Bucket> vecBuckets;

    if (mapHistogram.size() <= nMaxColors)
    {
        // 고유 색상 수가 이미 목표 이하이면 색상별로 독립 버킷을 만들어 그대로 팔레트에 반영한다.
        for (auto it = mapHistogram.constBegin(); it != mapHistogram.constEnd(); ++it)
        {
            Bucket oSingle;
            ColorCount oColorCount;
            oColorCount.rgbColor = it.key();
            oColorCount.nCount = it.value();
            oSingle.vecColors.append(oColorCount);
            oSingle.nTotalCount = it.value();
            vecBuckets.append(oSingle);
        }
    }
    else
    {
        Bucket oInitial;
        oInitial.nTotalCount = 0;
        for (auto it = mapHistogram.constBegin(); it != mapHistogram.constEnd(); ++it)
        {
            ColorCount oColorCount;
            oColorCount.rgbColor = it.key();
            oColorCount.nCount = it.value();
            oInitial.vecColors.append(oColorCount);
            oInitial.nTotalCount += it.value();
        }
        vecBuckets.append(oInitial);

        bool bCanSplit = true;
        while (vecBuckets.size() < nMaxColors && bCanSplit)
        {
            // 분할 가능한(고유색 2개 이상) 버킷 중 총 픽셀 수가 가장 큰 버킷을 분할 대상으로 선택
            int nTargetIndex = -1;
            qint64 nBestCount = -1;
            for (int i = 0; i < vecBuckets.size(); ++i)
            {
                if (vecBuckets.at(i).vecColors.size() > 1 && vecBuckets.at(i).nTotalCount > nBestCount)
                {
                    nBestCount = vecBuckets.at(i).nTotalCount;
                    nTargetIndex = i;
                }
            }

            if (nTargetIndex < 0)
            {
                bCanSplit = false;
                continue;
            }

            Bucket oTarget = vecBuckets.at(nTargetIndex);
            int nChannel = WidestChannel(oTarget);
            SortBucketByChannel(oTarget, nChannel);

            qint64 nHalf = oTarget.nTotalCount / 2;
            qint64 nAccum = 0;
            int nSplitAt = 1;
            for (int i = 0; i < oTarget.vecColors.size(); ++i)
            {
                nAccum += oTarget.vecColors.at(i).nCount;
                if (nAccum >= nHalf)
                {
                    nSplitAt = i + 1;
                    break;
                }
            }
            nSplitAt = qBound(1, nSplitAt, oTarget.vecColors.size() - 1);

            Bucket oLeft;
            Bucket oRight;
            oLeft.nTotalCount = 0;
            oRight.nTotalCount = 0;

            for (int i = 0; i < nSplitAt; ++i)
            {
                oLeft.vecColors.append(oTarget.vecColors.at(i));
                oLeft.nTotalCount += oTarget.vecColors.at(i).nCount;
            }
            for (int i = nSplitAt; i < oTarget.vecColors.size(); ++i)
            {
                oRight.vecColors.append(oTarget.vecColors.at(i));
                oRight.nTotalCount += oTarget.vecColors.at(i).nCount;
            }

            vecBuckets[nTargetIndex] = oLeft;
            vecBuckets.append(oRight);
        }
    }

    QHash<QRgb, int> mapColorToIndex;
    mapColorToIndex.reserve(mapHistogram.size());

    oResult.vecPalette.reserve(vecBuckets.size());
    for (int i = 0; i < vecBuckets.size(); ++i)
    {
        QRgb rgbAverage = AverageColor(vecBuckets.at(i).vecColors);
        oResult.vecPalette.append(rgbAverage);

        for (const ColorCount &oColorCount : vecBuckets.at(i).vecColors)
        {
            mapColorToIndex[oColorCount.rgbColor] = i;
        }
    }

    oResult.vecPixelIndices.resize(nWidth * nHeight);
    for (int y = 0; y < nHeight; ++y)
    {
        const QRgb *pLine = reinterpret_cast<const QRgb*>(imageArgb.constScanLine(y));
        for (int x = 0; x < nWidth; ++x)
        {
            oResult.vecPixelIndices[y * nWidth + x] = mapColorToIndex.value(pLine[x], 0);
        }
    }

    return oResult;
}
