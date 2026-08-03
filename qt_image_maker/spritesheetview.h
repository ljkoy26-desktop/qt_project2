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
QT_END_NAMESPACE

// 타일 리스트 미리보기(드래그 앤 드랍 재정렬) + 전체 스트립 미리보기 + 선택 타일 확대 미리보기를
// 담당하는 위젯. 실제 이미지 로딩/저장은 SpriteSheetModel과 MainWindow에서 처리하고,
// 이 위젯은 순수하게 "현재 화면에 보이는 타일 순서"만 책임진다.
class SpriteSheetView : public QWidget
{
    Q_OBJECT

public:
    explicit SpriteSheetView(QWidget *pParent = nullptr);

    // vecTiles 순서대로 리스트를 채우고 미리보기를 갱신한다.
    void Populate(const QVector<QImage> &vecTiles, int nTileSize);

    // 리스트에 표시된(사용자가 드래그로 재정렬했을 수 있는) 현재 순서를 반환한다.
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

private:
    void InitUi();
    void UpdateStripPreview();
    void RenumberItems();
    void MoveCurrentRow(int nOffset);
    void HandleReorderChanged();

private:
    QListWidget *m_pListWidget;
    QLabel      *m_pStripPreviewLabel;
    QScrollArea *m_pStripScrollArea;
    QLabel      *m_pZoomLabel;
    QPushButton *m_pMoveUpButton;
    QPushButton *m_pMoveDownButton;

    int          m_nTileSize;
};

#endif // SPRITESHEETVIEW_H
