#ifndef SPRITESHEETVIEW_H
#define SPRITESHEETVIEW_H

#include <QWidget>
#include <QImage>
#include <QVector>

QT_BEGIN_NAMESPACE
class QListWidget;
class QListWidgetItem;
class QLabel;
class QPushButton;
class QIcon;
class QPoint;
QT_END_NAMESPACE

// 타일을 격자 형태(창 폭에 맞춰 한 줄에 보이는 개수가 자동으로 조절됨)로 보여주고,
// 드래그 앤 드랍으로 순서를 바꿀 수 있는 스트립 미리보기 + 선택 타일 확대 미리보기를
// 담당하는 위젯. 실제 이미지 로딩/저장은 SpriteSheetModel과 MainWindow에서 처리하고,
// 이 위젯은 순수하게 "현재 화면에 보이는 타일 순서"만 책임진다.
class SpriteSheetView : public QWidget
{
    Q_OBJECT

public:
    explicit SpriteSheetView(QWidget *pParent = nullptr);

    // vecTiles 순서대로 리스트를 채우고 미리보기를 갱신한다. 기존 목록(제외 상태 포함)은 모두 초기화된다.
    void Populate(const QVector<QImage> &vecTiles, int nTileSize);

    // vecNewTiles를 기존 목록 뒤에 추가한다. 기존 항목의 순서/제외 상태는 그대로 유지된다.
    // (이미지 이어붙이기 시 사용)
    void AppendTiles(const QVector<QImage> &vecNewTiles);

    // 리스트에 표시된(사용자가 드래그로 재정렬했을 수 있는) 현재 순서 중,
    // "제외" 처리되지 않은 타일만 반환한다. 내보내기/미리보기/모델 동기화에 사용된다.
    QVector<QImage> CurrentOrder() const;

    void Clear();

signals:
    // 사용자가 드래그 앤 드랍으로 타일 순서를 변경했을 때 발생.
    void OrderChanged();

private slots:
    void OnRowsMoved();
    void OnCurrentItemChanged(QListWidgetItem *pCurrent, QListWidgetItem *pPrevious);
    void OnSelectionChanged();
    void OnMoveUpClicked();
    void OnMoveDownClicked();
    void OnToggleExcludeClicked();
    void OnListContextMenuRequested(const QPoint &oPos);
    void OnMoveToFrontClicked();
    void OnMoveToBackClicked();
    void OnMoveToPositionClicked();
    void OnRemoveItemClicked();

private:
    void InitUi();
    void RenumberItems();
    // 선택된 항목들을 row 오름차순으로 정렬해 반환한다(그룹 이동/삭제/제외 토글 등에서 공용으로 사용).
    QVector<QListWidgetItem*> SortedSelectedItems() const;
    // 인접한 두 row(nRowA < nRowB)에 있는 아이템의 위치를 서로 맞바꾼다.
    void SwapRows(int nRowA, int nRowB);
    // 선택된 항목 전체를 하나의 그룹으로 묶어 nOffset(-1 또는 +1)칸 이동시킨다.
    void MoveSelectedRows(int nOffset);
    // 선택된 항목 전체를 상대 순서를 유지한 채 맨 앞(bToFront=true) 또는 맨 뒤로 이동시킨다.
    void MoveSelectedRowsToEdge(bool bToFront);
    // 숫자를 입력해 이동하는 단일 선택 전용 동작(현재 currentRow 기준).
    void MoveCurrentRowTo(int nTargetRow);
    // 선택된 항목들의 제외 상태를 일괄 통일해서 토글한다(하나라도 포함 상태면 전체 제외로, 전체 제외 상태면 전체 복귀로).
    void ApplyExcludeToggle();
    void HandleReorderChanged();
    QIcon BuildTileIcon(const QImage &imageTile) const;
    QListWidgetItem* CreateTileItem(const QImage &imageTile, int nRow);
    bool IsItemExcluded(QListWidgetItem *pItem) const;
    void SetItemExcluded(QListWidgetItem *pItem, bool bExcluded);

private:
    QListWidget *m_pListWidget;
    QLabel      *m_pZoomLabel;
    QPushButton *m_pMoveUpButton;
    QPushButton *m_pMoveDownButton;
    QPushButton *m_pToggleExcludeButton;

    int          m_nTileSize;
};

#endif // SPRITESHEETVIEW_H
