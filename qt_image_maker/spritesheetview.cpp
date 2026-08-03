#include "spritesheetview.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QScrollArea>
#include <QPainter>
#include <QPixmap>
#include <QIcon>
#include <QVariant>
#include <QPushButton>

SpriteSheetView::SpriteSheetView(QWidget *pParent)
    : QWidget(pParent)
    , m_pListWidget(nullptr)
    , m_pStripPreviewLabel(nullptr)
    , m_pStripScrollArea(nullptr)
    , m_pZoomLabel(nullptr)
    , m_pMoveUpButton(nullptr)
    , m_pMoveDownButton(nullptr)
    , m_nTileSize(0)
{
    InitUi();
}

void SpriteSheetView::InitUi()
{
    // 타일 순서 재정렬 리스트: 우측에 세로로 쌓아서 보여주고, 창 높이에 맞춰 스스로 늘어나도록 구성.
    m_pListWidget = new QListWidget(this);
    m_pListWidget->setViewMode(QListView::IconMode);
    m_pListWidget->setFlow(QListView::TopToBottom);
    m_pListWidget->setWrapping(false);
    m_pListWidget->setMovement(QListView::Snap);
    m_pListWidget->setDragDropMode(QAbstractItemView::InternalMove);
    m_pListWidget->setDefaultDropAction(Qt::MoveAction);
    m_pListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pListWidget->setResizeMode(QListView::Adjust);
    m_pListWidget->setSpacing(2);
    m_pListWidget->setMinimumWidth(70);
    m_pListWidget->setMaximumWidth(90);

    connect(m_pListWidget->model(), &QAbstractItemModel::rowsMoved, this, &SpriteSheetView::OnRowsMoved);
    connect(m_pListWidget, &QListWidget::currentItemChanged, this, &SpriteSheetView::OnCurrentItemChanged);

    m_pStripPreviewLabel = new QLabel(this);
    m_pStripPreviewLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_pStripPreviewLabel->setStyleSheet(QStringLiteral("background-color: #303030;"));

    m_pStripScrollArea = new QScrollArea(this);
    m_pStripScrollArea->setWidget(m_pStripPreviewLabel);
    m_pStripScrollArea->setWidgetResizable(false);
    m_pStripScrollArea->setFixedHeight(180);
    m_pStripScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pStripScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_pZoomLabel = new QLabel(this);
    m_pZoomLabel->setAlignment(Qt::AlignCenter);
    m_pZoomLabel->setStyleSheet(QStringLiteral("background-color: #303030;"));

    QLabel *pStripTitle = new QLabel(QStringLiteral("스트립 미리보기 (현재 순서)"), this);
    QLabel *pZoomTitle = new QLabel(QStringLiteral("선택 타일 확대"), this);
    QLabel *pListTitle = new QLabel(QStringLiteral("타일 순서 (드래그로 변경)"), this);

    QVBoxLayout *pLeftLayout = new QVBoxLayout();
    pLeftLayout->addWidget(pZoomTitle);
    pLeftLayout->addWidget(m_pZoomLabel);
    pLeftLayout->addStretch(1);

    QWidget *pLeftPanel = new QWidget(this);
    pLeftPanel->setLayout(pLeftLayout);

    m_pMoveUpButton = new QPushButton(QStringLiteral("▲"), this);
    m_pMoveDownButton = new QPushButton(QStringLiteral("▼"), this);
    m_pMoveUpButton->setToolTip(QStringLiteral("선택한 타일을 한 칸 위로 이동"));
    m_pMoveDownButton->setToolTip(QStringLiteral("선택한 타일을 한 칸 아래로 이동"));
    m_pMoveUpButton->setFixedHeight(24);
    m_pMoveDownButton->setFixedHeight(24);

    connect(m_pMoveUpButton, &QPushButton::clicked, this, &SpriteSheetView::OnMoveUpClicked);
    connect(m_pMoveDownButton, &QPushButton::clicked, this, &SpriteSheetView::OnMoveDownClicked);

    QHBoxLayout *pMoveButtonsLayout = new QHBoxLayout();
    pMoveButtonsLayout->addWidget(m_pMoveUpButton);
    pMoveButtonsLayout->addWidget(m_pMoveDownButton);

    QVBoxLayout *pRightLayout = new QVBoxLayout();
    pRightLayout->addWidget(pListTitle);
    pRightLayout->addLayout(pMoveButtonsLayout);
    pRightLayout->addWidget(m_pListWidget, 1);

    QWidget *pRightPanel = new QWidget(this);
    pRightPanel->setLayout(pRightLayout);
    pRightPanel->setMinimumWidth(70);
    pRightPanel->setMaximumWidth(90);

    QHBoxLayout *pMainRowLayout = new QHBoxLayout();
    pMainRowLayout->addWidget(pLeftPanel, 1);
    pMainRowLayout->addWidget(pRightPanel);

    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addWidget(pStripTitle);
    pMainLayout->addWidget(m_pStripScrollArea);
    pMainLayout->addLayout(pMainRowLayout, 1);

    setLayout(pMainLayout);
}

void SpriteSheetView::Populate(const QVector<QImage> &vecTiles, int nTileSize)
{
    m_nTileSize = nTileSize;

    m_pListWidget->clear();

    const int nIconEdge = 20;
    m_pListWidget->setIconSize(QSize(nIconEdge, nIconEdge));

    for (int i = 0; i < vecTiles.size(); ++i)
    {
        QImage imageScaled = vecTiles.at(i).scaled(nIconEdge, nIconEdge, Qt::KeepAspectRatio, Qt::FastTransformation);
        QListWidgetItem *pItem = new QListWidgetItem(QIcon(QPixmap::fromImage(imageScaled)), QString::number(i));
        pItem->setData(Qt::UserRole, QVariant::fromValue(vecTiles.at(i)));
        pItem->setTextAlignment(Qt::AlignHCenter);
        m_pListWidget->addItem(pItem);
    }

    if (m_pListWidget->count() > 0)
    {
        m_pListWidget->setCurrentRow(0);
    }

    UpdateStripPreview();
}

QVector<QImage> SpriteSheetView::CurrentOrder() const
{
    QVector<QImage> vecResult;
    vecResult.reserve(m_pListWidget->count());

    for (int i = 0; i < m_pListWidget->count(); ++i)
    {
        QListWidgetItem *pItem = m_pListWidget->item(i);
        vecResult.append(pItem->data(Qt::UserRole).value<QImage>());
    }

    return vecResult;
}

void SpriteSheetView::Clear()
{
    m_pListWidget->clear();
    m_pStripPreviewLabel->clear();
    m_pStripPreviewLabel->setFixedSize(1, 1);
    m_pZoomLabel->clear();
    m_pZoomLabel->setFixedSize(1, 1);
    m_nTileSize = 0;
}

void SpriteSheetView::OnRowsMoved()
{
    HandleReorderChanged();
}

void SpriteSheetView::OnMoveUpClicked()
{
    MoveCurrentRow(-1);
}

void SpriteSheetView::OnMoveDownClicked()
{
    MoveCurrentRow(1);
}

void SpriteSheetView::MoveCurrentRow(int nOffset)
{
    int nCurrentRow = m_pListWidget->currentRow();
    if (nCurrentRow < 0)
    {
        return;
    }

    int nNewRow = nCurrentRow + nOffset;
    if (nNewRow < 0 || nNewRow >= m_pListWidget->count())
    {
        return;
    }

    QListWidgetItem *pItem = m_pListWidget->takeItem(nCurrentRow);
    m_pListWidget->insertItem(nNewRow, pItem);
    m_pListWidget->setCurrentItem(pItem);

    HandleReorderChanged();
}

void SpriteSheetView::HandleReorderChanged()
{
    RenumberItems();
    UpdateStripPreview();
    emit OrderChanged();
}

void SpriteSheetView::OnCurrentItemChanged(QListWidgetItem *pCurrent, QListWidgetItem *pPrevious)
{
    Q_UNUSED(pPrevious);

    if (pCurrent == nullptr)
    {
        m_pZoomLabel->clear();
        return;
    }

    QImage imageTile = pCurrent->data(Qt::UserRole).value<QImage>();
    if (imageTile.isNull())
    {
        m_pZoomLabel->clear();
        return;
    }

    int nZoom = (m_nTileSize <= 16) ? 8 : 2;
    QImage imageZoomed = imageTile.scaled(imageTile.width() * nZoom, imageTile.height() * nZoom, Qt::KeepAspectRatio, Qt::FastTransformation);

    m_pZoomLabel->setPixmap(QPixmap::fromImage(imageZoomed));
    m_pZoomLabel->setFixedSize(imageZoomed.size());
}

void SpriteSheetView::RenumberItems()
{
    for (int i = 0; i < m_pListWidget->count(); ++i)
    {
        m_pListWidget->item(i)->setText(QString::number(i));
    }
}

void SpriteSheetView::UpdateStripPreview()
{
    QVector<QImage> vecTiles = CurrentOrder();

    if (vecTiles.isEmpty() || m_nTileSize <= 0)
    {
        m_pStripPreviewLabel->clear();
        m_pStripPreviewLabel->setFixedSize(1, 1);
        return;
    }

    int nWidth = m_nTileSize * vecTiles.size();
    QImage imageStrip(nWidth, m_nTileSize, QImage::Format_ARGB32);
    imageStrip.fill(Qt::transparent);

    QPainter painter(&imageStrip);
    for (int i = 0; i < vecTiles.size(); ++i)
    {
        painter.drawImage(i * m_nTileSize, 0, vecTiles.at(i));
    }
    painter.end();

    int nZoom = (m_nTileSize <= 16) ? 4 : 2;
    QImage imageZoomed = imageStrip.scaled(imageStrip.width() * nZoom, imageStrip.height() * nZoom, Qt::KeepAspectRatio, Qt::FastTransformation);

    m_pStripPreviewLabel->setPixmap(QPixmap::fromImage(imageZoomed));
    m_pStripPreviewLabel->setFixedSize(imageZoomed.size());
}
