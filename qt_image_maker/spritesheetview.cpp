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
#include <QButtonGroup>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QPoint>
#include <QFont>
#include <QRect>

namespace
{
    // 타일 이미지(Qt::UserRole)와 별개로, "내보내기 대상에서 제외" 여부를 저장하는 데이터 롤.
    const int kExcludedRole = Qt::UserRole + 1;

    // 격자 미리보기에서 원본 타일 크기(16x16, 64x64 등)와 무관하게 셀에 표시할 고정 크기(px).
    const int kGridCellSize = 64;
    // 셀 하단에 순번 텍스트를 표시할 여백 높이(px).
    const int kGridNumberAreaHeight = 16;
    // 셀 사이/캔버스 가장자리 여백(px).
    const int kGridCellSpacing = 4;
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
    , m_pColumns5Button(nullptr)
    , m_pColumns10Button(nullptr)
    , m_pColumns20Button(nullptr)
    , m_nTileSize(0)
    , m_nColumnsPerRow(10)
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
    m_pListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_pListWidget->model(), &QAbstractItemModel::rowsMoved, this, &SpriteSheetView::OnRowsMoved);
    connect(m_pListWidget, &QListWidget::currentItemChanged, this, &SpriteSheetView::OnCurrentItemChanged);
    connect(m_pListWidget, &QListWidget::customContextMenuRequested, this, &SpriteSheetView::OnListContextMenuRequested);

    m_pStripPreviewLabel = new QLabel(this);
    m_pStripPreviewLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_pStripPreviewLabel->setStyleSheet(QStringLiteral("background-color: #303030;"));

    m_pStripScrollArea = new QScrollArea(this);
    m_pStripScrollArea->setWidget(m_pStripPreviewLabel);
    m_pStripScrollArea->setWidgetResizable(false);
    m_pStripScrollArea->setFixedHeight(180);
    m_pStripScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pStripScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_pZoomLabel = new QLabel(this);
    m_pZoomLabel->setAlignment(Qt::AlignCenter);
    m_pZoomLabel->setStyleSheet(QStringLiteral("background-color: #303030;"));

    // 한 줄에 몇 개씩 격자로 보여줄지 선택하는 버튼(라디오 형태, 하나만 선택 유지).
    m_pColumns5Button = new QPushButton(QStringLiteral("5개씩"), this);
    m_pColumns10Button = new QPushButton(QStringLiteral("10개씩"), this);
    m_pColumns20Button = new QPushButton(QStringLiteral("20개씩"), this);
    m_pColumns5Button->setCheckable(true);
    m_pColumns10Button->setCheckable(true);
    m_pColumns20Button->setCheckable(true);
    m_pColumns5Button->setProperty("columnsPerRow", 5);
    m_pColumns10Button->setProperty("columnsPerRow", 10);
    m_pColumns20Button->setProperty("columnsPerRow", 20);
    m_pColumns10Button->setChecked(true);

    QButtonGroup *pColumnsButtonGroup = new QButtonGroup(this);
    pColumnsButtonGroup->setExclusive(true);
    pColumnsButtonGroup->addButton(m_pColumns5Button);
    pColumnsButtonGroup->addButton(m_pColumns10Button);
    pColumnsButtonGroup->addButton(m_pColumns20Button);

    connect(m_pColumns5Button, &QPushButton::clicked, this, &SpriteSheetView::OnColumnsButtonClicked);
    connect(m_pColumns10Button, &QPushButton::clicked, this, &SpriteSheetView::OnColumnsButtonClicked);
    connect(m_pColumns20Button, &QPushButton::clicked, this, &SpriteSheetView::OnColumnsButtonClicked);

    QLabel *pStripTitle = new QLabel(QStringLiteral("스트립 미리보기 (현재 순서)"), this);
    QLabel *pZoomTitle = new QLabel(QStringLiteral("선택 타일 확대"), this);
    QLabel *pListTitle = new QLabel(QStringLiteral("타일 순서 (드래그로 변경)"), this);
    pListTitle->setWordWrap(true);
    pListTitle->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

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

    QHBoxLayout *pStripTitleLayout = new QHBoxLayout();
    pStripTitleLayout->addWidget(pStripTitle);
    pStripTitleLayout->addStretch(1);
    pStripTitleLayout->addWidget(m_pColumns5Button);
    pStripTitleLayout->addWidget(m_pColumns10Button);
    pStripTitleLayout->addWidget(m_pColumns20Button);

    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addLayout(pStripTitleLayout);
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

void SpriteSheetView::OnColumnsButtonClicked()
{
    QPushButton *pButton = qobject_cast<QPushButton*>(sender());
    if (pButton == nullptr)
    {
        return;
    }

    m_nColumnsPerRow = pButton->property("columnsPerRow").toInt();
    UpdateStripPreview();
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

void SpriteSheetView::MoveCurrentRowTo(int nTargetRow)
{
    int nCurrentRow = m_pListWidget->currentRow();
    if (nCurrentRow < 0)
    {
        return;
    }

    nTargetRow = qBound(0, nTargetRow, m_pListWidget->count() - 1);
    if (nTargetRow == nCurrentRow)
    {
        return;
    }

    QListWidgetItem *pItem = m_pListWidget->takeItem(nCurrentRow);
    m_pListWidget->insertItem(nTargetRow, pItem);
    m_pListWidget->setCurrentItem(pItem);

    HandleReorderChanged();
}

void SpriteSheetView::OnListContextMenuRequested(const QPoint &oPos)
{
    QListWidgetItem *pItem = m_pListWidget->itemAt(oPos);
    if (pItem == nullptr)
    {
        return;
    }

    m_pListWidget->setCurrentItem(pItem);

    QMenu oMenu(this);
    QAction *pActionToFront = oMenu.addAction(QStringLiteral("맨앞으로"));
    QAction *pActionToBack = oMenu.addAction(QStringLiteral("맨뒤로"));
    QAction *pActionToggleExclude = oMenu.addAction(IsItemExcluded(pItem) ? QStringLiteral("이미지 복귀") : QStringLiteral("이미지 제외"));
    QAction *pActionRemove = oMenu.addAction(QStringLiteral("해당 이미지 제거"));
    QAction *pActionMoveToPosition = oMenu.addAction(QStringLiteral("숫자를 입력하여 이동"));

    QAction *pSelectedAction = oMenu.exec(m_pListWidget->mapToGlobal(oPos));
    if (pSelectedAction == nullptr)
    {
        return;
    }

    if (pSelectedAction == pActionToFront)
    {
        OnMoveToFrontClicked();
    }
    else if (pSelectedAction == pActionToBack)
    {
        OnMoveToBackClicked();
    }
    else if (pSelectedAction == pActionToggleExclude)
    {
        OnToggleExcludeClicked();
    }
    else if (pSelectedAction == pActionRemove)
    {
        OnRemoveItemClicked();
    }
    else if (pSelectedAction == pActionMoveToPosition)
    {
        OnMoveToPositionClicked();
    }
}

void SpriteSheetView::OnMoveToFrontClicked()
{
    MoveCurrentRowTo(0);
}

void SpriteSheetView::OnMoveToBackClicked()
{
    MoveCurrentRowTo(m_pListWidget->count() - 1);
}

void SpriteSheetView::OnMoveToPositionClicked()
{
    int nCurrentRow = m_pListWidget->currentRow();
    if (nCurrentRow < 0)
    {
        return;
    }

    int nMaxIndex = m_pListWidget->count() - 1;

    bool bOk = false;
    int nTargetPosition = QInputDialog::getInt(this, QStringLiteral("순서 이동"), QStringLiteral("이동할 순번을 입력하세요 (0 ~ %1):").arg(nMaxIndex), nCurrentRow, 0, nMaxIndex, 1, &bOk);
    if (!bOk)
    {
        return;
    }

    MoveCurrentRowTo(nTargetPosition);
}

void SpriteSheetView::OnRemoveItemClicked()
{
    QListWidgetItem *pCurrent = m_pListWidget->currentItem();
    if (pCurrent == nullptr)
    {
        return;
    }

    QMessageBox::StandardButton eButton = QMessageBox::question(this, QStringLiteral("이미지 제거"), QStringLiteral("정말로 지우겠습니까?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (eButton != QMessageBox::Yes)
    {
        return;
    }

    delete m_pListWidget->takeItem(m_pListWidget->row(pCurrent));

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

    // 원본 타일 크기(16x16, 64x64 등)와 무관하게 셀은 항상 kGridCellSize로 고정 표시하고,
    // 한 줄에 m_nColumnsPerRow개씩 배치한 뒤 넘치면 다음 줄로 격자 형태로 이어간다.
    int nCellWidth = kGridCellSize + kGridCellSpacing;
    int nCellHeight = kGridCellSize + kGridNumberAreaHeight + kGridCellSpacing;
    int nRowCount = (vecTiles.size() + m_nColumnsPerRow - 1) / m_nColumnsPerRow;

    int nCanvasWidth = nCellWidth * m_nColumnsPerRow + kGridCellSpacing;
    int nCanvasHeight = nCellHeight * nRowCount + kGridCellSpacing;

    QImage imageGrid(nCanvasWidth, nCanvasHeight, QImage::Format_ARGB32);
    imageGrid.fill(Qt::transparent);

    QPainter painter(&imageGrid);

    QFont oFont = painter.font();
    oFont.setPixelSize(11);
    painter.setFont(oFont);
    painter.setPen(Qt::white);

    for (int i = 0; i < vecTiles.size(); ++i)
    {
        int nRow = i / m_nColumnsPerRow;
        int nCol = i % m_nColumnsPerRow;

        int nCellX = kGridCellSpacing + nCol * nCellWidth;
        int nCellY = kGridCellSpacing + nRow * nCellHeight;

        QImage imageScaled = vecTiles.at(i).scaled(kGridCellSize, kGridCellSize, Qt::KeepAspectRatio, Qt::FastTransformation);
        int nImageX = nCellX + (kGridCellSize - imageScaled.width()) / 2;
        int nImageY = nCellY + (kGridCellSize - imageScaled.height()) / 2;
        painter.drawImage(nImageX, nImageY, imageScaled);

        QRect oNumberRect(nCellX, nCellY + kGridCellSize, kGridCellSize, kGridNumberAreaHeight);
        painter.drawText(oNumberRect, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(i));
    }
    painter.end();

    m_pStripPreviewLabel->setPixmap(QPixmap::fromImage(imageGrid));
    m_pStripPreviewLabel->setFixedSize(imageGrid.size());
}
