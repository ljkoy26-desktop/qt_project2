#include "mainwindow.h"
#include "spritesheetview.h"
#include "colorquantizer.h"
#include "bmpwriter.h"

#include <QAction>
#include <QDesktopServices>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPushButton>
#include <QSize>
#include <QStatusBar>
#include <QToolBar>
#include <QUrl>

MainWindow::MainWindow(QWidget *pParent)
    : QMainWindow(pParent)
    , m_pSpriteView(nullptr)
    , m_pFileMenu(nullptr)
    , m_pExportBmpMenu(nullptr)
    , m_pExportTilesMenu(nullptr)
    , m_pOpenAction(nullptr)
    , m_pMergeFolderAction(nullptr)
    , m_pExportPngAction(nullptr)
    , m_pExportBmpTrueColorAction(nullptr)
    , m_pExportBmp256Action(nullptr)
    , m_pExportBmp16Action(nullptr)
    , m_pExportTilesPngAction(nullptr)
    , m_pExportTilesBmpTrueColorAction(nullptr)
    , m_pExportTilesBmp256Action(nullptr)
    , m_pExportTilesBmp16Action(nullptr)
    , m_pTileInfoLabel(nullptr)
{
    // 스트립 미리보기가 기본값인 20개씩 x 5줄 격자를 스크롤 없이 보여줄 수 있도록 초기 창 크기를 확보.
    resize(1450, 1100);
    setWindowTitle(QStringLiteral("스프라이트시트 뷰어/편집기"));
    setAcceptDrops(true);

    InitCentralWidget();
    InitMenuBar();
    InitToolBar();
    InitStatusBar();
}

MainWindow::~MainWindow()
{
}

void MainWindow::InitCentralWidget()
{
    m_pSpriteView = new SpriteSheetView(this);
    setCentralWidget(m_pSpriteView);

    connect(m_pSpriteView, &SpriteSheetView::OrderChanged, this, &MainWindow::OnSpriteOrderChanged);
}

void MainWindow::InitMenuBar()
{
    m_pFileMenu = menuBar()->addMenu(QStringLiteral("파일"));

    m_pOpenAction = m_pFileMenu->addAction(QStringLiteral("이미지 열기..."), this, &MainWindow::OnOpenImage);
    m_pOpenAction->setShortcut(QKeySequence::Open);

    m_pMergeFolderAction = m_pFileMenu->addAction(QStringLiteral("폴더에서 타일 합치기..."), this, &MainWindow::OnMergeFromFolder);

    m_pFileMenu->addSeparator();

    m_pExportPngAction = m_pFileMenu->addAction(QStringLiteral("PNG로 내보내기..."), this, &MainWindow::OnExportPng);

    m_pExportBmpMenu = m_pFileMenu->addMenu(QStringLiteral("BMP로 내보내기"));
    m_pExportBmpTrueColorAction = m_pExportBmpMenu->addAction(QStringLiteral("트루컬러(24비트)..."), this, &MainWindow::OnExportBmpTrueColor);
    m_pExportBmp256Action = m_pExportBmpMenu->addAction(QStringLiteral("256색..."), this, &MainWindow::OnExportBmp256);
    m_pExportBmp16Action = m_pExportBmpMenu->addAction(QStringLiteral("16색..."), this, &MainWindow::OnExportBmp16);

    m_pFileMenu->addSeparator();

    m_pExportTilesMenu = m_pFileMenu->addMenu(QStringLiteral("타일 개별 이미지로 내보내기"));
    m_pExportTilesPngAction = m_pExportTilesMenu->addAction(QStringLiteral("PNG..."), this, &MainWindow::OnExportTilesPng);
    m_pExportTilesBmpTrueColorAction = m_pExportTilesMenu->addAction(QStringLiteral("BMP 트루컬러(24비트)..."), this, &MainWindow::OnExportTilesBmpTrueColor);
    m_pExportTilesBmp256Action = m_pExportTilesMenu->addAction(QStringLiteral("BMP 256색..."), this, &MainWindow::OnExportTilesBmp256);
    m_pExportTilesBmp16Action = m_pExportTilesMenu->addAction(QStringLiteral("BMP 16색..."), this, &MainWindow::OnExportTilesBmp16);

    m_pFileMenu->addSeparator();
    m_pFileMenu->addAction(QStringLiteral("종료"), this, &QWidget::close);
}

void MainWindow::InitToolBar()
{
    QToolBar *pToolBar = new QToolBar(QStringLiteral("Main"), this);
    pToolBar->setMovable(false);

    pToolBar->addAction(m_pOpenAction);
    pToolBar->addAction(m_pMergeFolderAction);
    pToolBar->addSeparator();
    pToolBar->addAction(m_pExportPngAction);
    pToolBar->addAction(m_pExportBmpTrueColorAction);
    pToolBar->addAction(m_pExportBmp256Action);
    pToolBar->addAction(m_pExportBmp16Action);

    addToolBar(Qt::TopToolBarArea, pToolBar);
}

void MainWindow::InitStatusBar()
{
    m_pTileInfoLabel = new QLabel(QStringLiteral("이미지를 열어주세요. (창으로 파일을 드래그해도 됩니다)"), this);
    statusBar()->addWidget(m_pTileInfoLabel, 1);
}

void MainWindow::UpdateStatusLabel()
{
    if (m_oModel.IsEmpty())
    {
        m_pTileInfoLabel->setText(QStringLiteral("이미지를 열어주세요. (창으로 파일을 드래그해도 됩니다)"));
        return;
    }

    m_pTileInfoLabel->setText(QString(QStringLiteral("타일 크기: %1x%2, 개수: %3, 원본: %4"))
        .arg(m_oModel.TileSize())
        .arg(m_oModel.TileSize())
        .arg(m_oModel.TileCount())
        .arg(m_oModel.SourcePath()));
}

bool MainWindow::EnsureNotEmpty()
{
    if (m_oModel.IsEmpty())
    {
        QMessageBox::warning(this, QStringLiteral("알림"), QStringLiteral("먼저 이미지를 불러오세요."));
        return false;
    }

    return true;
}

void MainWindow::ShowCompletionDialog(const QString &strFolderPath, int nImageCount, const QSize &oFinalSize)
{
    QMessageBox oMsgBox(this);
    oMsgBox.setWindowTitle(QStringLiteral("작업 완료"));
    oMsgBox.setIcon(QMessageBox::Information);
    oMsgBox.setText(QString(QStringLiteral("작업이 완료되었습니다.\n\n이미지 개수: %1개\n최종 크기: %2x%3"))
        .arg(nImageCount)
        .arg(oFinalSize.width())
        .arg(oFinalSize.height()));

    QPushButton *pOpenFolderButton = oMsgBox.addButton(QStringLiteral("해당 폴더 열기"), QMessageBox::ActionRole);
    QPushButton *pOkButton = oMsgBox.addButton(QStringLiteral("확인"), QMessageBox::AcceptRole);
    oMsgBox.setDefaultButton(pOkButton);

    oMsgBox.exec();

    if (oMsgBox.clickedButton() == pOpenFolderButton)
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(strFolderPath));
    }
}

QImage MainWindow::FlattenToWhite(const QImage &imageSource) const
{
    QImage imageFlat(imageSource.size(), QImage::Format_RGB32);
    imageFlat.fill(Qt::white);

    QPainter painter(&imageFlat);
    painter.drawImage(0, 0, imageSource);
    painter.end();

    return imageFlat;
}

QImage MainWindow::BuildFlattenedStripImage() const
{
    return FlattenToWhite(m_oModel.BuildStripImage());
}

QImage MainWindow::BuildIndexedImage(const QImage &imageFlat, const QuantizeResult &oQuantized) const
{
    QImage imageIndexed(imageFlat.width(), imageFlat.height(), QImage::Format_Indexed8);
    imageIndexed.setColorCount(oQuantized.vecPalette.size());
    for (int i = 0; i < oQuantized.vecPalette.size(); ++i)
    {
        imageIndexed.setColor(i, oQuantized.vecPalette.at(i));
    }

    for (int y = 0; y < imageFlat.height(); ++y)
    {
        uchar *pLine = imageIndexed.scanLine(y);
        for (int x = 0; x < imageFlat.width(); ++x)
        {
            pLine[x] = (uchar)oQuantized.vecPixelIndices.at(y * imageFlat.width() + x);
        }
    }

    return imageIndexed;
}

void MainWindow::OnOpenImage()
{
    QString strPath = QFileDialog::getOpenFileName(this, QStringLiteral("이미지 열기"), QString(), QStringLiteral("이미지 파일 (*.png *.bmp)"));
    if (strPath.isEmpty())
    {
        return;
    }

    LoadImageFile(strPath);
}

void MainWindow::LoadImageFile(const QString &strPath)
{
    QString strError;
    if (!m_oModel.LoadFromFile(strPath, &strError))
    {
        QMessageBox::warning(this, QStringLiteral("열기 실패"), strError);
        return;
    }

    m_pSpriteView->Populate(m_oModel.Tiles(), m_oModel.TileSize());
    UpdateStatusLabel();
}

void MainWindow::OnMergeFromFolder()
{
    QString strFolderPath = QFileDialog::getExistingDirectory(this, QStringLiteral("타일 이미지 폴더 선택"));
    if (strFolderPath.isEmpty())
    {
        return;
    }

    QString strError;
    if (!m_oModel.LoadFromFolder(strFolderPath, &strError))
    {
        QMessageBox::warning(this, QStringLiteral("합치기 실패"), strError);
        return;
    }

    m_pSpriteView->Populate(m_oModel.Tiles(), m_oModel.TileSize());
    UpdateStatusLabel();

    QSize oFinalSize(m_oModel.TileSize() * m_oModel.TileCount(), m_oModel.TileSize());
    ShowCompletionDialog(strFolderPath, m_oModel.TileCount(), oFinalSize);
}

namespace
{
    // 드롭된 URL 목록 중 확장자가 png/bmp인 첫 로컬 파일 경로를 찾는다. 없으면 빈 문자열.
    QString FindFirstImageFilePath(const QList<QUrl> &listUrls)
    {
        for (const QUrl &oUrl : listUrls)
        {
            if (!oUrl.isLocalFile())
            {
                continue;
            }

            QString strPath = oUrl.toLocalFile();
            QString strSuffix = QFileInfo(strPath).suffix().toLower();
            if (strSuffix == QStringLiteral("png") || strSuffix == QStringLiteral("bmp"))
            {
                return strPath;
            }
        }

        return QString();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *pEvent)
{
    if (pEvent->mimeData()->hasUrls() && !FindFirstImageFilePath(pEvent->mimeData()->urls()).isEmpty())
    {
        pEvent->acceptProposedAction();
        return;
    }

    pEvent->ignore();
}

void MainWindow::dropEvent(QDropEvent *pEvent)
{
    QString strPath = pEvent->mimeData()->hasUrls() ? FindFirstImageFilePath(pEvent->mimeData()->urls()) : QString();
    if (strPath.isEmpty())
    {
        pEvent->ignore();
        return;
    }

    pEvent->acceptProposedAction();

    if (!m_oModel.IsEmpty())
    {
        QImage imageProbe;
        if (imageProbe.load(strPath) && imageProbe.height() == m_oModel.TileSize())
        {
            QMessageBox::StandardButton eAnswer = QMessageBox::question(this, QStringLiteral("이미지 이어붙이기"),
                QString(QStringLiteral("이미 열려 있는 이미지와 타일 크기(%1x%1)가 같습니다.\n뒤에 이어붙일까요?"))
                    .arg(m_oModel.TileSize()),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

            if (eAnswer == QMessageBox::Yes)
            {
                int nOldCount = m_oModel.TileCount();

                QString strError;
                if (!m_oModel.AppendFromFile(strPath, &strError))
                {
                    QMessageBox::warning(this, QStringLiteral("이어붙이기 실패"), strError);
                    return;
                }

                // 화면에 표시된 목록(제외 상태 포함)은 그대로 두고, 새로 추가된 타일만 뒤에 붙인다.
                m_pSpriteView->AppendTiles(m_oModel.Tiles().mid(nOldCount));
                UpdateStatusLabel();
                return;
            }
        }
    }

    LoadImageFile(strPath);
}

void MainWindow::OnExportPng()
{
    if (!EnsureNotEmpty())
    {
        return;
    }

    QString strPath = QFileDialog::getSaveFileName(this, QStringLiteral("PNG로 내보내기"), QString(), QStringLiteral("PNG 파일 (*.png)"));
    if (strPath.isEmpty())
    {
        return;
    }

    QImage imageStrip = m_oModel.BuildStripImage();
    if (!imageStrip.save(strPath, "PNG"))
    {
        QMessageBox::warning(this, QStringLiteral("저장 실패"), QStringLiteral("PNG 파일 저장에 실패했습니다."));
        return;
    }

    ShowCompletionDialog(QFileInfo(strPath).absolutePath(), m_oModel.TileCount(), imageStrip.size());
}

void MainWindow::OnExportBmpTrueColor()
{
    if (!EnsureNotEmpty())
    {
        return;
    }

    QString strPath = QFileDialog::getSaveFileName(this, QStringLiteral("BMP로 내보내기 (트루컬러)"), QString(), QStringLiteral("BMP 파일 (*.bmp)"));
    if (strPath.isEmpty())
    {
        return;
    }

    QImage imageFlat = BuildFlattenedStripImage().convertToFormat(QImage::Format_RGB888);
    if (!imageFlat.save(strPath, "BMP"))
    {
        QMessageBox::warning(this, QStringLiteral("저장 실패"), QStringLiteral("BMP 파일 저장에 실패했습니다."));
        return;
    }

    ShowCompletionDialog(QFileInfo(strPath).absolutePath(), m_oModel.TileCount(), imageFlat.size());
}

void MainWindow::OnExportBmp256()
{
    if (!EnsureNotEmpty())
    {
        return;
    }

    QString strPath = QFileDialog::getSaveFileName(this, QStringLiteral("BMP로 내보내기 (256색)"), QString(), QStringLiteral("BMP 파일 (*.bmp)"));
    if (strPath.isEmpty())
    {
        return;
    }

    QImage imageFlat = BuildFlattenedStripImage();
    QuantizeResult oQuantized = ColorQuantizer::Quantize(imageFlat, 256);
    QImage imageIndexed = BuildIndexedImage(imageFlat, oQuantized);

    if (!imageIndexed.save(strPath, "BMP"))
    {
        QMessageBox::warning(this, QStringLiteral("저장 실패"), QStringLiteral("BMP 파일 저장에 실패했습니다."));
        return;
    }

    ShowCompletionDialog(QFileInfo(strPath).absolutePath(), m_oModel.TileCount(), imageIndexed.size());
}

void MainWindow::OnExportBmp16()
{
    if (!EnsureNotEmpty())
    {
        return;
    }

    QString strPath = QFileDialog::getSaveFileName(this, QStringLiteral("BMP로 내보내기 (16색)"), QString(), QStringLiteral("BMP 파일 (*.bmp)"));
    if (strPath.isEmpty())
    {
        return;
    }

    QImage imageFlat = BuildFlattenedStripImage();
    QuantizeResult oQuantized = ColorQuantizer::Quantize(imageFlat, 16);

    QString strError;
    if (!BmpWriter::WriteIndexed4Bpp(strPath, imageFlat.width(), imageFlat.height(), oQuantized.vecPixelIndices, oQuantized.vecPalette, &strError))
    {
        QMessageBox::warning(this, QStringLiteral("저장 실패"), strError);
        return;
    }

    ShowCompletionDialog(QFileInfo(strPath).absolutePath(), m_oModel.TileCount(), imageFlat.size());
}

void MainWindow::OnSpriteOrderChanged()
{
    m_oModel.SetTiles(m_pSpriteView->CurrentOrder());
}

bool MainWindow::SaveTileAsPng(const QImage &imageTile, const QString &strFilePath, QString *pStrError)
{
    if (!imageTile.save(strFilePath, "PNG"))
    {
        *pStrError = QStringLiteral("PNG 저장 실패: %1").arg(strFilePath);
        return false;
    }

    return true;
}

bool MainWindow::SaveTileAsBmpTrueColor(const QImage &imageTile, const QString &strFilePath, QString *pStrError)
{
    QImage imageRgb888 = FlattenToWhite(imageTile).convertToFormat(QImage::Format_RGB888);
    if (!imageRgb888.save(strFilePath, "BMP"))
    {
        *pStrError = QStringLiteral("BMP 저장 실패: %1").arg(strFilePath);
        return false;
    }

    return true;
}

bool MainWindow::SaveTileAsBmp256(const QImage &imageTile, const QString &strFilePath, QString *pStrError)
{
    QImage imageFlat = FlattenToWhite(imageTile);
    QuantizeResult oQuantized = ColorQuantizer::Quantize(imageFlat, 256);
    QImage imageIndexed = BuildIndexedImage(imageFlat, oQuantized);

    if (!imageIndexed.save(strFilePath, "BMP"))
    {
        *pStrError = QStringLiteral("BMP 저장 실패: %1").arg(strFilePath);
        return false;
    }

    return true;
}

bool MainWindow::SaveTileAsBmp16(const QImage &imageTile, const QString &strFilePath, QString *pStrError)
{
    QImage imageFlat = FlattenToWhite(imageTile);
    QuantizeResult oQuantized = ColorQuantizer::Quantize(imageFlat, 16);

    return BmpWriter::WriteIndexed4Bpp(strFilePath, imageFlat.width(), imageFlat.height(), oQuantized.vecPixelIndices, oQuantized.vecPalette, pStrError);
}

void MainWindow::ExportTilesToFolder(TileSaveFn pSaveFn, const QString &strExtension)
{
    if (!EnsureNotEmpty())
    {
        return;
    }

    QString strFolderPath = QFileDialog::getExistingDirectory(this, QStringLiteral("타일을 저장할 폴더 선택"));
    if (strFolderPath.isEmpty())
    {
        return;
    }

    const QVector<QImage> &vecTiles = m_oModel.Tiles();
    QDir oDir(strFolderPath);

    int nSuccessCount = 0;
    QString strError;

    for (int i = 0; i < vecTiles.size(); ++i)
    {
        QString strFilePath = oDir.filePath(QString(QStringLiteral("%1.%2")).arg(i + 1).arg(strExtension));

        if ((this->*pSaveFn)(vecTiles.at(i), strFilePath, &strError))
        {
            ++nSuccessCount;
        }
    }

    if (nSuccessCount == vecTiles.size())
    {
        ShowCompletionDialog(strFolderPath, nSuccessCount, QSize(m_oModel.TileSize(), m_oModel.TileSize()));
    }
    else
    {
        QMessageBox::warning(this, QStringLiteral("일부 저장 실패"),
            QString(QStringLiteral("%1 / %2개 타일만 저장되었습니다.\n마지막 오류: %3")).arg(nSuccessCount).arg(vecTiles.size()).arg(strError));
    }
}

void MainWindow::OnExportTilesPng()
{
    ExportTilesToFolder(&MainWindow::SaveTileAsPng, QStringLiteral("png"));
}

void MainWindow::OnExportTilesBmpTrueColor()
{
    ExportTilesToFolder(&MainWindow::SaveTileAsBmpTrueColor, QStringLiteral("bmp"));
}

void MainWindow::OnExportTilesBmp256()
{
    ExportTilesToFolder(&MainWindow::SaveTileAsBmp256, QStringLiteral("bmp"));
}

void MainWindow::OnExportTilesBmp16()
{
    ExportTilesToFolder(&MainWindow::SaveTileAsBmp16, QStringLiteral("bmp"));
}
