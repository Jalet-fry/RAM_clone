#ifndef TABLEITEMDELEGATE_H
#define TABLEITEMDELEGATE_H

#include <QStyledItemDelegate>
#include "thememanager.h"

class TableItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit TableItemDelegate(QObject* parent = nullptr);
    
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setTheme(Theme theme);
    
private:
    Theme _theme;
};

#endif // TABLEITEMDELEGATE_H

