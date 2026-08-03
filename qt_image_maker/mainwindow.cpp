#include "mainwindow.h"
#include "spritesheetview.h"
#include "colorquantizer.h"
#include "bmpwriter.h"

#include <QAction>
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
#include <QStatusBar>
#include <QToolBar>
#include <QUrl>

MainWindow::MainWindow(QWidget *pParent)
    : QMainWindow(pParent)
    , m_pSpriteView(nullptr)
    , m_pFileMenu(nullptr)
    , m_pExportBmpMenu(nullptr)
    , m_pOpenAction(nullptr)
    , m_pExportPngAction(nullptr)
    , m_pExportBmpTrueColorAction(nullptr)
    , m_pExportBmp256Action(nullptr)
    , m_pExportBmp16Action(nullptr)
    , m_pTileInfoLabel(nullptr)
{
    resize(1300, 850);
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

    m_pFileMenu->addSeparator();

    m_pExportPngAction = m_pFileMenu->addAction(QStringLiteral("PNG로 내보내기..."), this, &MainWindow::OnExportPng);

    m_pExportBmpMenu = m_pFileMenu->addMenu(QStringLiteral("BMP로 내보내기"));
    m_pExportBmpTrueColorAction = m_pExportBmpMenu->addAction(QStringLiteral("트루컬러(24비트)..."), this, &MainWindow::OnExportBmpTrueColor);
    m_pExportBmp256Action = m_pExportBmpMenu->addAction(QStringLiteral("256색..."), this, &MainWindow::OnExportBmp256);
    m_pExportBmp16Action = m_pExportBmpMenu->addAction(QStringLiteral("16색..."), this, &MainWindow::OnExportBmp16);

    m_pFileMenu->addSeparator();
    m_pFileMenu->addAction(QStringLiteral("종료"), this, &QWidget::close);
}

void MainWindow::InitToolBar()
{
    QToolBar *pToolBar = new QToolBar(QStringLiteral("Main"), this);
    pToolBar->setMovable(false);

    pToolBar->addAction(m_pOpenAction);
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

QImage MainWindow::BuildFlattenedStripImage() const
{
    QImage imageStrip = m_oModel.BuildStripImage();

    QImage imageFlat(imageStrip.size(), QImage::Format_RGB32);
    imageFlat.fill(Qt::white);

    QPainter painter(&imageFlat);
    painter.drawImage(0, 0, imageStrip);
    painter.end();

    return imageFlat;
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

    statusBar()->showMessage(QStringLiteral("PNG로 저장했습니다: ") + strPath, 5000);
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

    statusBar()->showMessage(QStringLiteral("BMP(트루컬러)로 저장했습니다: ") + strPath, 5000);
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

    if (!imageIndexed.save(strPath, "BMP"))
    {
        QMessageBox::warning(this, QStringLiteral("저장 실패"), QStringLiteral("BMP 파일 저장에 실패했습니다."));
        return;
    }

    statusBar()->showMessage(QStringLiteral("BMP(256색)로 저장했습니다: ") + strPath, 5000);
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

    statusBar()->showMessage(QStringLiteral("BMP(16색)로 저장했습니다: ") + strPath, 5000);
}

void MainWindow::OnSpriteOrderChanged()
{
    m_oModel.SetTiles(m_pSpriteView->CurrentOrder());
}
