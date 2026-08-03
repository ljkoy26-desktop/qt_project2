#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "spritesheetmodel.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QLabel;
class QDragEnterEvent;
class QDropEvent;
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
    void OnExportPng();
    void OnExportBmpTrueColor();
    void OnExportBmp256();
    void OnExportBmp16();
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
    QImage BuildFlattenedStripImage() const;
    void UpdateStatusLabel();
    void LoadImageFile(const QString &strPath);

private:
    SpriteSheetModel m_oModel;
    SpriteSheetView  *m_pSpriteView;

    QMenu   *m_pFileMenu;
    QMenu   *m_pExportBmpMenu;

    QAction *m_pOpenAction;
    QAction *m_pExportPngAction;
    QAction *m_pExportBmpTrueColorAction;
    QAction *m_pExportBmp256Action;
    QAction *m_pExportBmp16Action;

    QLabel  *m_pTileInfoLabel;
};

#endif // MAINWINDOW_H
