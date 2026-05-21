#include "gutter_widget.h"
#include "fold_region.h"
#include <QPainter>
#include <QFontMetrics>

GutterWidget::GutterWidget(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#252526"));
    setPalette(pal);

    QFont font("Consolas", 9);
    setFont(font);

    setMouseTracking(true);
    setFixedWidth(22);
}

void GutterWidget::setFoldRegions(const QList<FoldRegion> *regions)
{
    m_regions = regions;
}

void GutterWidget::setArrayIndices(const QMap<int, int> *indices)
{
    m_arrayIndices = indices;
}

void GutterWidget::setBlockInfo(const QVector<GutterBlockInfo> &blocks, int totalBlocks)
{
    m_blocks = blocks;
    m_totalBlocks = totalBlocks;
    // Don't call update() here — the gutter is already being repainted
    // by the updateRequest that triggered updateGutterData()
}

QSize GutterWidget::sizeHint() const
{
    return QSize(22, 0);
}

void GutterWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(event->rect(), QColor("#252526"));

    QFont gutterFont("Consolas", 9);
    QFont indexFont("Consolas", 9);
    indexFont.setItalic(true);
    QFontMetrics gutterFm(gutterFont);
    QFontMetrics indexFm(indexFont);

    painter.setFont(gutterFont);

    // Build a map of opener lines to fold regions
    QMap<int, const FoldRegion*> regionMap;
    if (m_regions) {
        for (const FoldRegion &r : *m_regions) {
            regionMap[r.openerLine] = &r;
        }
    }

    for (const GutterBlockInfo &block : m_blocks) {
        int cy = block.top + block.height / 2;

        // Fold triangle
        if (m_regions && regionMap.contains(block.blockNumber)) {
            const FoldRegion *r = regionMap[block.blockNumber];
            painter.setPen(QColor("#c5c5c5"));
            painter.setBrush(QColor("#c5c5c5"));

            if (r->isFolded) {
                QPolygon triangle;
                triangle << QPoint(4, cy - 4)
                         << QPoint(4, cy + 4)
                         << QPoint(10, cy);
                painter.drawPolygon(triangle);
            } else {
                QPolygon triangle;
                triangle << QPoint(3, cy - 3)
                         << QPoint(11, cy - 3)
                         << QPoint(7, cy + 3);
                painter.drawPolygon(triangle);
            }
            painter.setBrush(Qt::NoBrush);
        }
    }
}

void GutterWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->x() >= 12 || !m_regions) return;

    // Find which block was clicked
    for (const GutterBlockInfo &block : m_blocks) {
        if (event->y() >= block.top && event->y() < block.top + block.height) {
            for (const FoldRegion &r : *m_regions) {
                if (r.openerLine == block.blockNumber) {
                    emit foldToggleRequested(r.id);
                    return;
                }
            }
            break;
        }
    }
}
