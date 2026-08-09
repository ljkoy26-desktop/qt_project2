#ifndef SPRITESHEETVIEW_H
#define SPRITESHEETVIEW_H

#include <QWidget>
#include <QImage>
#include <QVector>

QT_BEGIN_NAMESPACE
class QListWidget;
class QListWidgetItem;
class QLabel;
class QScrollArea;
class QPushButton;
class QIcon;
class QPoint;
QT_END_NAMESPACE

// 타일 리스트 미리보기(드래그 앤 드랍 재정렬) + 전체 스트립 미리보기 + 선택 타일 확대 미리보기를
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
    void OnMoveUpClicked();
    void OnMoveDownClicked();
    void OnToggleExcludeClicked();
    void OnListContextMenuRequested(const QPoint &oPos);
    void OnMoveToFrontClicked();
    void OnMoveToBackClicked();
    void OnMoveToPositionClicked();
    void OnRemoveItemClicked();
    void OnColumnsButtonClicked();

private:
    void InitUi();
    void UpdateStripPreview();
    void RenumberItems();
    void MoveCurrentRow(int nOffset);
    void MoveCurrentRowTo(int nTargetRow);
    void HandleReorderChanged();
    QIcon BuildTileIcon(const QImage &imageTile) const;
    QListWidgetItem* CreateTileItem(const QImage &imageTile, int nRow);
    bool IsItemExcluded(QListWidgetItem *pItem) const;
    void SetItemExcluded(QListWidgetItem *pItem, bool bExcluded);

private:
    QListWidget *m_pListWidget;
    QLabel      *m_pStripPreviewLabel;
    QScrollArea *m_pStripScrollArea;
    QLabel      *m_pZoomLabel;
    QPushButton *m_pMoveUpButton;
    QPushButton *m_pMoveDownButton;
    QPushButton *m_pToggleExcludeButton;
    QPushButton *m_pColumns5Button;
    QPushButton *m_pColumns10Button;
    QPushButton *m_pColumns20Button;

    int          m_nTileSize;
    int          m_nColumnsPerRow;
};

#endif // SPRITESHEETVIEW_H
