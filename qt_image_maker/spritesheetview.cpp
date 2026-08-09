#include "spritesheetview.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QIcon>
#include <QVariant>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QPoint>

namespace
{
    // 타일 이미지(Qt::UserRole)와 별개로, "내보내기 대상에서 제외" 여부를 저장하는 데이터 롤.
    const int kExcludedRole = Qt::UserRole + 1;

    // 격자에 표시할 타일 아이콘 한 변 크기(px). 원본 타일 크기(16x16, 64x64 등)와 무관하게
    // 이 크기로 통일해서 보여준다.
    const int kGridIconEdge = 64;
    // 격자 셀 사이 여백(px).
    const int kGridSpacing = 6;
}

SpriteSheetView::SpriteSheetView(QWidget *pParent)
    : QWidget(pParent)
    , m_pListWidget(nullptr)
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
    // 타일 격자: 드래그로 순서를 바꿀 수 있고, IconMode + Wrapping으로 창 폭에 맞춰
    // 한 줄에 보이는 개수가 자동으로 조절된다(스트립 미리보기와 타일 순서 리스트를 겸함).
    m_pListWidget = new QListWidget(this);
    m_pListWidget->setViewMode(QListView::IconMode);
    m_pListWidget->setFlow(QListView::LeftToRight);
    m_pListWidget->setWrapping(true);
    m_pListWidget->setMovement(QListView::Snap);
    m_pListWidget->setDragDropMode(QAbstractItemView::InternalMove);
    m_pListWidget->setDefaultDropAction(Qt::MoveAction);
    m_pListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pListWidget->setResizeMode(QListView::Adjust);
    m_pListWidget->setSpacing(kGridSpacing);
    m_pListWidget->setIconSize(QSize(kGridIconEdge, kGridIconEdge));
    m_pListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_pListWidget->model(), &QAbstractItemModel::rowsMoved, this, &SpriteSheetView::OnRowsMoved);
    connect(m_pListWidget, &QListWidget::currentItemChanged, this, &SpriteSheetView::OnCurrentItemChanged);
    connect(m_pListWidget, &QListWidget::customContextMenuRequested, this, &SpriteSheetView::OnListContextMenuRequested);

    m_pZoomLabel = new QLabel(this);
    m_pZoomLabel->setAlignment(Qt::AlignCenter);
    m_pZoomLabel->setStyleSheet(QStringLiteral("background-color: #303030;"));

    QLabel *pStripTitle = new QLabel(QStringLiteral("스트립 미리보기 (현재 순서, 드래그로 순서 변경)"), this);
    QLabel *pZoomTitle = new QLabel(QStringLiteral("선택 타일 확대"), this);

    m_pMoveUpButton = new QPushButton(QStringLiteral("이전으로"), this);
    m_pMoveDownButton = new QPushButton(QStringLiteral("다음으로"), this);
    m_pMoveUpButton->setToolTip(QStringLiteral("선택한 타일을 이전 순서로 이동"));
    m_pMoveDownButton->setToolTip(QStringLiteral("선택한 타일을 다음 순서로 이동"));
    m_pMoveUpButton->setFixedHeight(24);
    m_pMoveDownButton->setFixedHeight(24);

    connect(m_pMoveUpButton, &QPushButton::clicked, this, &SpriteSheetView::OnMoveUpClicked);
    connect(m_pMoveDownButton, &QPushButton::clicked, this, &SpriteSheetView::OnMoveDownClicked);

    m_pToggleExcludeButton = new QPushButton(QStringLiteral("제외"), this);
    m_pToggleExcludeButton->setToolTip(QStringLiteral("선택한 타일을 내보내기 대상에서 제외/복귀"));
    m_pToggleExcludeButton->setFixedHeight(24);
    m_pToggleExcludeButton->setEnabled(false);

    connect(m_pToggleExcludeButton, &QPushButton::clicked, this, &SpriteSheetView::OnToggleExcludeClicked);

    QHBoxLayout *pToolbarLayout = new QHBoxLayout();
    pToolbarLayout->addWidget(pStripTitle);
    pToolbarLayout->addStretch(1);
    pToolbarLayout->addWidget(m_pMoveUpButton);
    pToolbarLayout->addWidget(m_pMoveDownButton);
    pToolbarLayout->addWidget(m_pToggleExcludeButton);

    QVBoxLayout *pZoomLayout = new QVBoxLayout();
    pZoomLayout->addWidget(pZoomTitle);
    pZoomLayout->addWidget(m_pZoomLabel);
    pZoomLayout->addStretch(1);

    QWidget *pZoomPanel = new QWidget(this);
    pZoomPanel->setLayout(pZoomLayout);

    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addLayout(pToolbarLayout);
    pMainLayout->addWidget(m_pListWidget, 1);
    pMainLayout->addWidget(pZoomPanel);

    setLayout(pMainLayout);
}

void SpriteSheetView::Populate(const QVector<QImage> &vecTiles, int nTileSize)
{
    m_nTileSize = nTileSize;

    m_pListWidget->clear();

    for (int i = 0; i < vecTiles.size(); ++i)
    {
        m_pListWidget->addItem(CreateTileItem(vecTiles.at(i), i));
    }

    if (m_pListWidget->count() > 0)
    {
        m_pListWidget->setCurrentRow(0);
    }
}

void SpriteSheetView::AppendTiles(const QVector<QImage> &vecNewTiles)
{
    int nStartRow = m_pListWidget->count();

    for (int i = 0; i < vecNewTiles.size(); ++i)
    {
        m_pListWidget->addItem(CreateTileItem(vecNewTiles.at(i), nStartRow + i));
    }
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
