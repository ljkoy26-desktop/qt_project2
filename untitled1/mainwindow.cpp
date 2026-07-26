#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QStyle>
#include <QAction>
#include <QTextEdit>

MainWindow::MainWindow(QWidget *pParent)
    : QMainWindow(pParent)
    , ui(new Ui::MainWindow)
    , m_pMdiArea(nullptr)
    , m_pMainToolBar(nullptr)
    , m_pFileMenu(nullptr)
    , m_pEditMenu(nullptr)
    , m_pActionMenu(nullptr)
    , m_pOptionMenu(nullptr)
    , m_pDbaMenu(nullptr)
    , m_pToolsMenu(nullptr)
    , m_pWindowMenu(nullptr)
    , m_pHelpMenu(nullptr)
    , m_pStatusMessageLabel(nullptr)
    , m_pAutoCommitLabel(nullptr)
    , m_pVersionLabel(nullptr)
    , m_nSqlToolSeq(0)
{
    ui->setupUi(this);

    resize(1024, 768);
    setWindowTitle(QStringLiteral("Orange 8.0"));

    InitMdiArea();
    InitMenuBar();
    InitToolBar();
    InitStatusBar();

    OnNewSqlTool();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitMdiArea()
{
    m_pMdiArea = new QMdiArea(this);
    m_pMdiArea->setViewMode(QMdiArea::SubWindowView);
    m_pMdiArea->setDocumentMode(false);

    setCentralWidget(m_pMdiArea);
}

void MainWindow::InitMenuBar()
{
    m_pFileMenu = ui->menubar->addMenu(QStringLiteral("File"));
    m_pFileMenu->addAction(QStringLiteral("New"), this, &MainWindow::OnNewSqlTool);
    m_pFileMenu->addSeparator();
    m_pFileMenu->addAction(QStringLiteral("Exit"), this, &QWidget::close);

    m_pEditMenu = ui->menubar->addMenu(QStringLiteral("Edit"));
    m_pActionMenu = ui->menubar->addMenu(QStringLiteral("Action"));
    m_pOptionMenu = ui->menubar->addMenu(QStringLiteral("Option"));
    m_pDbaMenu = ui->menubar->addMenu(QStringLiteral("DBA"));
    m_pToolsMenu = ui->menubar->addMenu(QStringLiteral("Tools"));

    m_pWindowMenu = ui->menubar->addMenu(QStringLiteral("Window"));
    m_pWindowMenu->addAction(QStringLiteral("Cascade"), this, &MainWindow::OnCascadeWindows);
    m_pWindowMenu->addAction(QStringLiteral("Tile"), this, &MainWindow::OnTileWindows);
    m_pWindowMenu->addSeparator();
    m_pWindowMenu->addAction(QStringLiteral("Close All"), m_pMdiArea, &QMdiArea::closeAllSubWindows);

    m_pHelpMenu = ui->menubar->addMenu(QStringLiteral("Help"));
}

void MainWindow::InitToolBar()
{
    m_pMainToolBar = new QToolBar(QStringLiteral("Main"), this);
    m_pMainToolBar->setMovable(false);
    m_pMainToolBar->setIconSize(QSize(24, 24));

    QAction *pNewAction = m_pMainToolBar->addAction(style()->standardIcon(QStyle::SP_FileIcon), QStringLiteral("SQL Tool"));
    connect(pNewAction, &QAction::triggered, this, &MainWindow::OnNewSqlTool);

    m_pMainToolBar->addSeparator();

    QAction *pOpenAction = m_pMainToolBar->addAction(style()->standardIcon(QStyle::SP_DialogOpenButton), QStringLiteral("Open"));
    QAction *pSaveAction = m_pMainToolBar->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), QStringLiteral("Save"));

    Q_UNUSED(pOpenAction);
    Q_UNUSED(pSaveAction);

    addToolBar(Qt::TopToolBarArea, m_pMainToolBar);
}

void MainWindow::InitStatusBar()
{
    m_pStatusMessageLabel = new QLabel(QStringLiteral("Ready"), this);
    m_pAutoCommitLabel = new QLabel(QStringLiteral("AutoCommit is On"), this);
    m_pVersionLabel = new QLabel(QStringLiteral("8.0.1.150"), this);

    ui->statusbar->addWidget(m_pStatusMessageLabel, 1);
    ui->statusbar->addPermanentWidget(m_pAutoCommitLabel);
    ui->statusbar->addPermanentWidget(m_pVersionLabel);
}

QMdiSubWindow* MainWindow::CreateSqlToolChild()
{
    ++m_nSqlToolSeq;

    QTextEdit *pEditor = new QTextEdit();
    pEditor->setPlaceholderText(QStringLiteral("SQL 입력 영역 (추후 구현 예정)"));

    QMdiSubWindow *pSubWindow = m_pMdiArea->addSubWindow(pEditor);
    pSubWindow->setWindowTitle(QString(QStringLiteral("SQL Tool:Not Connected/SQL%1")).arg(m_nSqlToolSeq));
    pSubWindow->resize(600, 400);

    return pSubWindow;
}

void MainWindow::OnNewSqlTool()
{
    QMdiSubWindow *pSubWindow = CreateSqlToolChild();

    if (pSubWindow != nullptr)
    {
        pSubWindow->showMaximized();
    }
}

void MainWindow::OnCascadeWindows()
{
    if (m_pMdiArea != nullptr)
    {
        m_pMdiArea->cascadeSubWindows();
    }
}

void MainWindow::OnTileWindows()
{
    if (m_pMdiArea != nullptr)
    {
        m_pMdiArea->tileSubWindows();
    }
}
