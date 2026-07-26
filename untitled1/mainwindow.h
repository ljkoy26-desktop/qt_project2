#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QToolBar>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *pParent = nullptr);
    ~MainWindow();

private slots:
    void OnNewSqlTool();
    void OnCascadeWindows();
    void OnTileWindows();

private:
    void InitMdiArea();
    void InitMenuBar();
    void InitToolBar();
    void InitStatusBar();
    QMdiSubWindow* CreateSqlToolChild();

private:
    Ui::MainWindow *ui;

    QMdiArea    *m_pMdiArea;
    QToolBar    *m_pMainToolBar;

    QMenu       *m_pFileMenu;
    QMenu       *m_pEditMenu;
    QMenu       *m_pActionMenu;
    QMenu       *m_pOptionMenu;
    QMenu       *m_pDbaMenu;
    QMenu       *m_pToolsMenu;
    QMenu       *m_pWindowMenu;
    QMenu       *m_pHelpMenu;

    QLabel      *m_pStatusMessageLabel;
    QLabel      *m_pAutoCommitLabel;
    QLabel      *m_pVersionLabel;

    int          m_nSqlToolSeq;
};
#endif // MAINWINDOW_H
