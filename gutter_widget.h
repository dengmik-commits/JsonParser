#ifndef GUTTER_WIDGET_H
#define GUTTER_WIDGET_H

#include <QWidget>
#include <QMap>
#include <QList>

struct FoldRegion;

struct GutterBlockInfo {
    int blockNumber;     // 0-based
    int top;             // pixel y position
    int height;          // pixel height
    bool visible;
};

class GutterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GutterWidget(QWidget *parent = nullptr);

    void setFoldRegions(const QList<FoldRegion> *regions);
    void setArrayIndices(const QMap<int, int> *indices);
    void setBlockInfo(const QVector<GutterBlockInfo> &blocks, int totalBlocks);

    QSize sizeHint() const override;

signals:
    void foldToggleRequested(int blockNumber);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    const QList<FoldRegion> *m_regions = nullptr;
    const QMap<int, int> *m_arrayIndices = nullptr;
    QVector<GutterBlockInfo> m_blocks;
    int m_totalBlocks = 0;
};

#endif // GUTTER_WIDGET_H