#include "bmpwriter.h"

#include <QDataStream>
#include <QFile>

namespace
{
    const int kPaletteEntryCount = 16;
}

bool BmpWriter::WriteIndexed4Bpp(const QString &strFilePath,
                                  int nWidth,
                                  int nHeight,
                                  const QVector<int> &vecPixelIndices,
                                  const QVector<QRgb> &vecPalette,
                                  QString *pStrError)
{
    if (nWidth <= 0 || nHeight <= 0 || vecPixelIndices.size() != (nWidth * nHeight))
    {
        if (pStrError != nullptr)
        {
            *pStrError = QStringLiteral("BMP 저장 실패: 픽셀 데이터 크기가 올바르지 않습니다.");
        }
        return false;
    }

    int nRowBytesUnpadded = (nWidth + 1) / 2;
    int nRowBytes = (nRowBytesUnpadded + 3) & ~3;
    int nPixelDataSize = nRowBytes * nHeight;
    int nPaletteBytes = kPaletteEntryCount * 4;
    int nHeaderSize = 14 + 40;
    int nPixelDataOffset = nHeaderSize + nPaletteBytes;
    int nFileSize = nPixelDataOffset + nPixelDataSize;

    QFile file(strFilePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        if (pStrError != nullptr)
        {
            *pStrError = QStringLiteral("BMP 파일을 생성할 수 없습니다: %1").arg(strFilePath);
        }
        return false;
    }

    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);

    // BITMAPFILEHEADER (14 bytes)
    out.writeRawData("BM", 2);
    out << (quint32)nFileSize;
    out << (quint32)0;
    out << (quint32)nPixelDataOffset;

    // BITMAPINFOHEADER (40 bytes)
    out << (quint32)40;
    out << (qint32)nWidth;
    out << (qint32)nHeight;
    out << (quint16)1;
    out << (quint16)4;
    out << (quint32)0;
    out << (quint32)nPixelDataSize;
    out << (qint32)2835;
    out << (qint32)2835;
    out << (quint32)kPaletteEntryCount;
    out << (quint32)0;

    // 팔레트 (Blue, Green, Red, Reserved) x 16
    for (int i = 0; i < kPaletteEntryCount; ++i)
    {
        QRgb rgbColor = (i < vecPalette.size()) ? vecPalette.at(i) : qRgb(0, 0, 0);
        out << (quint8)qBlue(rgbColor) << (quint8)qGreen(rgbColor) << (quint8)qRed(rgbColor) << (quint8)0;
    }

    // 픽셀 데이터: 2픽셀/바이트, bottom-up
    QByteArray baRow(nRowBytes, (char)0);
    for (int y = nHeight - 1; y >= 0; --y)
    {
        baRow.fill((char)0);
        for (int x = 0; x < nWidth; ++x)
        {
            int nIndex = vecPixelIndices.at(y * nWidth + x) & 0x0F;
            int nByteIndex = x / 2;
            if ((x % 2) == 0)
            {
                baRow[nByteIndex] = (char)((baRow[nByteIndex] & 0x0F) | (nIndex << 4));
            }
            else
            {
                baRow[nByteIndex] = (char)((baRow[nByteIndex] & 0xF0) | nIndex);
            }
        }
        out.writeRawData(baRow.constData(), nRowBytes);
    }

    file.close();

    return true;
}
