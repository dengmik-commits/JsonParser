#ifndef FOLD_REGION_H
#define FOLD_REGION_H

#include <QVector>
#include <QMap>
#include <QTextDocument>
#include <QTextBlock>
#include <QChar>

struct FoldRegion {
    int id;
    QChar openChar;     // '{' or '['
    int openerLine;     // 0-based line number
    int closerLine;     // 0-based line number
    int depth;
    bool isFolded = false;
    int itemCount = -1; // lazy: -1 = not yet computed

    bool isObject() const { return openChar == '{'; }
    bool isArray() const { return openChar == '['; }
};

class FoldRegionManager
{
public:
    FoldRegionManager();

    void findFoldRegions(QTextDocument *doc);
    void clearRegions();
    void fold(QTextDocument *doc, int regionId);
    void unfold(QTextDocument *doc, int regionId);
    void unfoldLine(QTextDocument *doc, int line);
    void foldAll(QTextDocument *doc, int minDepth = 1);
    void unfoldAll(QTextDocument *doc);

    int regionIndexAtLine(int line); // returns -1 if no region
    FoldRegion *regionAtLine(int line);
    FoldRegion *regionById(int id);
    int getItemCount(QTextDocument *doc, int regionIdx);

    QMap<int, int> getArrayIndices(QTextDocument *doc) const;

    const QList<FoldRegion> &regions() const { return m_regions; }

private:
    QList<FoldRegion> m_regions;
    QMap<int, int> m_byLine;   // openerLine → index in m_regions
    QMap<int, int> m_byId;     // id → index in m_regions

    int computeItemCount(QTextDocument *doc, int regionIdx);
};

#endif // FOLD_REGION_H