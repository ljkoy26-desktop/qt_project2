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

namespace
{
    // 타일 이미지(Qt::UserRole)와 별개로, "내보내기 대상에서 제외" 여부를 저장하는 데이터 롤.
    const int kExcludedRole = Qt::UserRole + 1;
}

SpriteSheetView::SpriteSheetView(QWidget *pParent)
    : QWidget(pParent)
    , m_pListWidget(nullptr)
    , m_pStripPreviewLabel(nullptr)
    , m_pStripScrollArea(nullptr)
    , m_pZoomLabel(nullptr)
    , m_pMoveUpButton(nullptr)
    , m_pMoveDownButton(nullptr)
    , m_pToggleExcludeButton(nullptr)
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

    m_pToggleExcludeButton = new QPushButton(QStringLiteral("제외"), this);
    m_pToggleExcludeButton->setToolTip(QStringLiteral("선택한 타일을 내보내기 대상에서 제외/복귀"));
    m_pToggleExcludeButton->setFixedHeight(24);
    m_pToggleExcludeButton->setEnabled(false);

    connect(m_pToggleExcludeButton, &QPushButton::clicked, this, &SpriteSheetView::OnToggleExcludeClicked);

    QVBoxLayout *pRightLayout = new QVBoxLayout();
    pRightLayout->addWidget(pListTitle);
    pRightLayout->addLayout(pMoveButtonsLayout);
    pRightLayout->addWidget(m_pToggleExcludeButton);
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
        m_pListWidget->addItem(CreateTileItem(vecTiles.at(i), i));
    }

    if (m_pListWidget->count() > 0)
    {
        m_pListWidget->setCurrentRow(0);
    }

    UpdateStripPreview();
}

void SpriteSheetView::AppendTiles(const QVector<QImage> &vecNewTiles)
{
    int nStartRow = m_pListWidget->count();

    for (int i = 0; i < vecNewTiles.size(); ++i)
    {
        m_pListWidget->addItem(CreateTileItem(vecNewTiles.at(i), nStartRow + i));
    }

    UpdateStripPreview();
}

QListWidgetItem* SpriteSheetView::CreateTileItem(const QImage &imageTile, int nRow)
{
    QListWidgetItem *pItem = new QListWidgetItem(BuildTileIcon(imageTile), QString::number(nRow));
    pItem->setData(Qt::UserRole, QVariant::fromValue(imageTile));
    pItem->setData(kExcludedRole, false);
    pItem->setTextAlignment(Qt::AlignHCenter);
    return pItem;
}

QIcon SpriteSheetView::BuildTileIcon(const QImage &imageTile) const
{
    int nIconEdge = m_pListWidget->iconSize().width();
    QImage imageScaled = imageTile.scaled(nIconEdge, nIconEdge, Qt::KeepAspectRatio, Qt::FastTransformation);
    return QIcon(QPixmap::fromImage(imageScaled));
}

bool SpriteSheetView::IsItemExcluded(QListWidgetItem *pItem) const
{
    return pItem->data(kExcludedRole).toBool();
}

void SpriteSheetView::SetItemExcluded(QListWidgetItem *pItem, bool bExcluded)
{
    pItem->setData(kExcludedRole, bExcluded);

    if (bExcluded)
    {
        QPixmap oPixmap = pItem->icon().pixmap(m_pListWidget->iconSize());
        QPixmap oDimmed(oPixmap.size());
        oDimmed.fill(Qt::transparent);

        QPainter painter(&oDimmed);
        painter.setOpacity(0.3);
        painter.drawPixmap(0, 0, oPixmap);
        painter.end();

        pItem->setIcon(QIcon(oDimmed));
        pItem->setBackground(QColor(90, 40, 40));
    }
    else
    {
        QImage imageTile = pItem->data(Qt::UserRole).value<QImage>();
        pItem->setIcon(BuildTileIcon(imageTile));
        pItem->setBackground(QBrush());
    }
}

QVector<QImage> SpriteSheetView::CurrentOrder() const
{
    QVector<QImage> vecResult;
    vecResult.reserve(m_pListWidget->count());

    for (int i = 0; i < m_pListWidget->count(); ++i)
    {
        QListWidgetItem *pItem = m_pListWidget->item(i);
        if (IsItemExcluded(pItem))
        {
            continue;
        }

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
    m_pToggleExcludeButton->setEnabled(false);
    m_pToggleExcludeButton->setText(QStringLiteral("제외"));
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

void SpriteSheetView::OnToggleExcludeClicked()
{
    QListWidgetItem *pCurrent = m_pListWidget->currentItem();
    if (pCurrent == nullptr)
    {
        return;
    }

    bool bNowExcluded = !IsItemExcluded(pCurrent);
    SetItemExcluded(pCurrent, bNowExcluded);
    m_pToggleExcludeButton->setText(bNowExcluded ? QStringLiteral("복귀") : QStringLiteral("제외"));

    HandleReorderChanged();
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
        m_pToggleExcludeButton->setEnabled(false);
        m_pToggleExcludeButton->setText(QStringLiteral("제외"));
        return;
    }

    m_pToggleExcludeButton->setEnabled(true);
    m_pToggleExcludeButton->setText(IsItemExcluded(pCurrent) ? QStringLiteral("복귀") : QStringLiteral("제외"));

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
