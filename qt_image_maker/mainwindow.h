#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "spritesheetmodel.h"
#include "colorquantizer.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QLabel;
class QDragEnterEvent;
class QDropEvent;
class QSize;
QT_END_NAMESPACE

class SpriteSheetView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *pParent = nullptr);
    ~MainWindow();

private slots:
    void OnOpenImage();
    void OnMergeFromFolder();
    void OnExportPng();
    void OnExportBmpTrueColor();
    void OnExportBmp256();
    void OnExportBmp16();
    void OnExportTilesPng();
    void OnExportTilesBmpTrueColor();
    void OnExportTilesBmp256();
    void OnExportTilesBmp16();
    void OnSpriteOrderChanged();

protected:
    void dragEnterEvent(QDragEnterEvent *pEvent) override;
    void dropEvent(QDropEvent *pEvent) override;

private:
    void InitMenuBar();
    void InitToolBar();
    void InitCentralWidget();
    void InitStatusBar();

    bool EnsureNotEmpty();
    QImage FlattenToWhite(const QImage &imageSource) const;
    QImage BuildIndexedImage(const QImage &imageFlat, const QuantizeResult &oQuantized) const;
    QImage BuildFlattenedStripImage() const;
    void UpdateStatusLabel();
    void LoadImageFile(const QString &strPath);

    // 저장/내보내기/합치기 작업 공통 완료 안내. "해당 폴더 열기" / "확인" 버튼 제공.
    void ShowCompletionDialog(const QString &strFolderPath, int nImageCount, const QSize &oFinalSize);

    bool SaveTileAsPng(const QImage &imageTile, const QString &strFilePath, QString *pStrError);
    bool SaveTileAsBmpTrueColor(const QImage &imageTile, const QString &strFilePath, QString *pStrError);
    bool SaveTileAsBmp256(const QImage &imageTile, const QString &strFilePath, QString *pStrError);
    bool SaveTileAsBmp16(const QImage &imageTile, const QString &strFilePath, QString *pStrError);

    using TileSaveFn = bool (MainWindow::*)(const QImage &, const QString &, QString *);
    void ExportTilesToFolder(TileSaveFn pSaveFn, const QString &strExtension);

private:
    SpriteSheetModel m_oModel;
    SpriteSheetView  *m_pSpriteView;

    QMenu   *m_pFileMenu;
    QMenu   *m_pExportBmpMenu;
    QMenu   *m_pExportTilesMenu;

    QAction *m_pOpenAction;
    QAction *m_pMergeFolderAction;
    QAction *m_pExportPngAction;
    QAction *m_pExportBmpTrueColorAction;
    QAction *m_pExportBmp256Action;
    QAction *m_pExportBmp16Action;
    QAction *m_pExportTilesPngAction;
    QAction *m_pExportTilesBmpTrueColorAction;
    QAction *m_pExportTilesBmp256Action;
    QAction *m_pExportTilesBmp16Action;

    QLabel  *m_pTileInfoLabel;
};

#endif // MAINWINDOW_H
