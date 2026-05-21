#include "fold_region.h"
#include <QTextDocument>
#include <QTextBlock>
#include <algorithm>

FoldRegionManager::FoldRegionManager() = default;

void FoldRegionManager::clearRegions()
{
    m_regions.clear();
    m_byLine.clear();
    m_byId.clear();
}

void FoldRegionManager::findFoldRegions(QTextDocument *doc)
{
    m_regions.clear();
    m_byLine.clear();
    m_byId.clear();

    int docLen = doc->characterCount();

    // Fast character scan using doc->characterAt() — no full copy
    struct StackEntry {
        QChar ch;
        int line;
    };
    QVector<StackEntry> stack;
    stack.reserve(64);

    int regionId = 0;
    bool inString = false;
    int line = 0;

    for (int i = 0; i < docLen; ++i) {
        QChar c = doc->characterAt(i);

        if (c == '\n') {
            line++;
            continue;
        }
        if (c == '\r') continue;
        // QPlainTextEdit uses ParagraphSeparator for line breaks
        if (c == QChar(0x2029)) {
            line++;
            continue;
        }

        if (inString) {
            if (c == '\\') {
                i++;
                if (i >= docLen) break;
                continue;
            }
            if (c == '"') {
                inString = false;
            }
            continue;
        }

        if (c == '"') {
            inString = true;
            continue;
        }

        if (c == '{' || c == '[') {
            stack.append({c, line});
        } else if (c == '}' || c == ']') {
            QChar expected = (c == '}') ? '{' : '[';
            if (!stack.isEmpty() && stack.last().ch == expected) {
                StackEntry opener = stack.takeLast();
                if (opener.line != line) {
                    FoldRegion region;
                    region.id = regionId++;
                    region.openChar = opener.ch;
                    region.openerLine = opener.line;
                    region.closerLine = line;
                    region.depth = stack.size();
                    region.isFolded = false;
                    region.itemCount = -1;
                    m_regions.append(region);
                }
            } else {
                stack.clear();
            }
        }
    }

    // Build lookup maps
    for (int i = 0; i < m_regions.size(); ++i) {
        m_byLine[m_regions[i].openerLine] = i;
        m_byId[m_regions[i].id] = i;
    }
}

int FoldRegionManager::computeItemCount(QTextDocument *doc, int regionIdx)
{
    const FoldRegion &region = m_regions[regionIdx];

    int totalBlocks = doc->blockCount();
    if (region.openerLine >= totalBlocks || region.closerLine >= totalBlocks) return 0;

    QTextBlock openerBlock = doc->findBlockByNumber(region.openerLine);
    QTextBlock closerBlock = doc->findBlockByNumber(region.closerLine);
    if (!openerBlock.isValid() || !closerBlock.isValid()) return 0;

    // Find the opener bracket character in the opener line
    QString openerText = openerBlock.text();
    int bracketPos = -1;
    for (int i = 0; i < openerText.length(); ++i) {
        if (openerText[i] == region.openChar) {
            bracketPos = openerBlock.position() + i;
            break;
        }
    }
    if (bracketPos < 0) return 0;

    // Find the closer bracket character in the closer line
    QChar closeChar = region.isArray() ? ']' : '}';
    QString closerText = closerBlock.text();
    int closePos = -1;
    for (int i = closerText.length() - 1; i >= 0; --i) {
        if (closerText[i] == closeChar) {
            closePos = closerBlock.position() + i;
            break;
        }
    }
    if (closePos <= bracketPos) return 0;

    int docLen = doc->characterCount();
    if (bracketPos >= docLen || closePos >= docLen) return 0;

    // Scan character-by-character between bracketPos+1 and closePos-1
    if (region.isArray()) {
        int depth = 0;
        bool inStr = false;
        int items = 1;
        bool hasContent = false;

        for (int i = bracketPos + 1; i < closePos && i < docLen; ++i) {
            QChar c = doc->characterAt(i);
            if (c == '\n' || c == '\r' || c == QChar(0x2029)) continue;
            if (inStr) {
                if (c == '\\') { i++; if (i >= closePos || i >= docLen) break; continue; }
                if (c == '"') inStr = false;
                continue;
            }
            if (c == '"') { inStr = true; hasContent = true; continue; }
            if (c == '{' || c == '[') { depth++; hasContent = true; continue; }
            if (c == '}' || c == ']') { depth--; continue; }
            if (c == ',' && depth == 0) { items++; continue; }
            if (!c.isSpace()) hasContent = true;
        }

        return hasContent ? items : 0;
    } else {
        int depth = 0;
        bool inStr = false;
        int keys = 0;
        bool expectKey = true;

        for (int i = bracketPos + 1; i < closePos && i < docLen; ++i) {
            QChar c = doc->characterAt(i);
            if (c == '\n' || c == '\r' || c == QChar(0x2029)) continue;
            if (inStr) {
                if (c == '\\') { i++; if (i >= closePos || i >= docLen) break; continue; }
                if (c == '"') {
                    inStr = false;
                    if (depth == 0 && expectKey) {
                        keys++;
                        expectKey = false;
                    }
                }
                continue;
            }
            if (c == '"') { inStr = true; continue; }
            if (c == '{' || c == '[') { depth++; expectKey = false; continue; }
            if (c == '}' || c == ']') { depth--; continue; }
            if (c == ',' && depth == 0) { expectKey = true; continue; }
            if (c == ':' && depth == 0) { expectKey = false; continue; }
        }

        return keys;
    }
}

int FoldRegionManager::getItemCount(QTextDocument *doc, int regionIdx)
{
    if (regionIdx < 0 || regionIdx >= m_regions.size()) return 0;
    if (m_regions[regionIdx].itemCount < 0) {
        m_regions[regionIdx].itemCount = computeItemCount(doc, regionIdx);
    }
    return m_regions[regionIdx].itemCount;
}

FoldRegion *FoldRegionManager::regionById(int id)
{
    auto it = m_byId.find(id);
    if (it != m_byId.end() && it.value() >= 0 && it.value() < m_regions.size())
        return &m_regions[it.value()];
    return nullptr;
}

int FoldRegionManager::regionIndexAtLine(int line)
{
    auto it = m_byLine.find(line);
    if (it != m_byLine.end() && it.value() >= 0 && it.value() < m_regions.size())
        return it.value();
    return -1;
}

FoldRegion *FoldRegionManager::regionAtLine(int line)
{
    auto it = m_byLine.find(line);
    if (it != m_byLine.end() && it.value() >= 0 && it.value() < m_regions.size())
        return &m_regions[it.value()];
    return nullptr;
}

void FoldRegionManager::fold(QTextDocument *doc, int regionId)
{
    FoldRegion *r = regionById(regionId);
    if (!r || r->isFolded) return;

    r->isFolded = true;

    QTextBlock block = doc->findBlockByNumber(r->openerLine + 1);
    while (block.isValid() && block.blockNumber() <= r->closerLine) {
        block.setVisible(false);
        block = block.next();
    }

    QTextBlock opener = doc->findBlockByNumber(r->openerLine);
    QTextBlock closer = doc->findBlockByNumber(r->closerLine);
    doc->markContentsDirty(opener.position(), closer.position() + closer.length() - opener.position());
}

void FoldRegionManager::unfold(QTextDocument *doc, int regionId)
{
    FoldRegion *r = regionById(regionId);
    if (!r || !r->isFolded) return;

    r->isFolded = false;

    // Make all blocks between opener and closer visible first
    QTextBlock block = doc->findBlockByNumber(r->openerLine + 1);
    while (block.isValid() && block.blockNumber() <= r->closerLine) {
        block.setVisible(true);
        block = block.next();
    }

    // Re-hide blocks that belong to already-folded child regions
    for (const auto &region : m_regions) {
        if (&region == r) continue;
        if (region.openerLine > r->openerLine && region.closerLine < r->closerLine && region.isFolded) {
            QTextBlock b = doc->findBlockByNumber(region.openerLine + 1);
            while (b.isValid() && b.blockNumber() <= region.closerLine) {
                b.setVisible(false);
                b = b.next();
            }
        }
    }

    QTextBlock opener = doc->findBlockByNumber(r->openerLine);
    QTextBlock closer = doc->findBlockByNumber(r->closerLine);
    doc->markContentsDirty(opener.position(), closer.position() + closer.length() - opener.position());
}

void FoldRegionManager::unfoldLine(QTextDocument *doc, int line)
{
    // Unfold any folded region that contains the given line
    bool changed = false;
    for (auto &region : m_regions) {
        if (region.isFolded && line >= region.openerLine && line <= region.closerLine) {
            region.isFolded = false;
            changed = true;

            QTextBlock block = doc->findBlockByNumber(region.openerLine + 1);
            while (block.isValid() && block.blockNumber() <= region.closerLine) {
                block.setVisible(true);
                block = block.next();
            }
        }
    }

    if (changed) {
        // Re-hide blocks that belong to still-folded child regions
        for (const auto &region : m_regions) {
            if (region.isFolded) {
                QTextBlock b = doc->findBlockByNumber(region.openerLine + 1);
                while (b.isValid() && b.blockNumber() <= region.closerLine) {
                    b.setVisible(false);
                    b = b.next();
                }
            }
        }

        int minLine = INT_MAX, maxLine = 0;
        for (const auto &region : m_regions) {
            if (line >= region.openerLine && line <= region.closerLine) {
                minLine = std::min(minLine, region.openerLine);
                maxLine = std::max(maxLine, region.closerLine);
            }
        }
        QTextBlock first = doc->findBlockByNumber(minLine);
        QTextBlock last = doc->findBlockByNumber(maxLine);
        doc->markContentsDirty(first.position(), last.position() + last.length() - first.position());
    }
}

void FoldRegionManager::foldAll(QTextDocument *doc, int minDepth)
{
    int minLine = INT_MAX, maxLine = 0;
    for (auto &region : m_regions) {
        if (region.depth >= minDepth && !region.isFolded) {
            region.isFolded = true;
            minLine = std::min(minLine, region.openerLine);
            maxLine = std::max(maxLine, region.closerLine);

            QTextBlock block = doc->findBlockByNumber(region.openerLine + 1);
            while (block.isValid() && block.blockNumber() <= region.closerLine) {
                block.setVisible(false);
                block = block.next();
            }
        }
    }
    if (minLine <= maxLine) {
        QTextBlock first = doc->findBlockByNumber(minLine);
        QTextBlock last = doc->findBlockByNumber(maxLine);
        doc->markContentsDirty(first.position(), last.position() + last.length() - first.position());
    }
}

void FoldRegionManager::unfoldAll(QTextDocument *doc)
{
    int minLine = INT_MAX, maxLine = 0;
    for (auto &region : m_regions) {
        if (region.isFolded) {
            region.isFolded = false;
            minLine = std::min(minLine, region.openerLine);
            maxLine = std::max(maxLine, region.closerLine);

            QTextBlock block = doc->findBlockByNumber(region.openerLine + 1);
            while (block.isValid() && block.blockNumber() <= region.closerLine) {
                block.setVisible(true);
                block = block.next();
            }
        }
    }
    if (minLine <= maxLine) {
        QTextBlock first = doc->findBlockByNumber(minLine);
        QTextBlock last = doc->findBlockByNumber(maxLine);
        doc->markContentsDirty(first.position(), last.position() + last.length() - first.position());
    }
}

QMap<int, int> FoldRegionManager::getArrayIndices(QTextDocument *doc) const
{
    QMap<int, int> result;

    for (const FoldRegion &region : m_regions) {
        if (!region.isArray() || region.isFolded) continue;

        int idx = 0;
        QTextBlock block = doc->findBlockByNumber(region.openerLine + 1);
        while (block.isValid() && block.blockNumber() <= region.closerLine) {
            if (block.isVisible()) {
                QString text = block.text().trimmed();
                if (!text.isEmpty() && text != "," && text != "]" && text != "}") {
                    result[block.blockNumber()] = idx++;
                }
            }
            block = block.next();
        }
    }

    return result;
}
