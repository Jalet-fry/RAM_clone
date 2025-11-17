#include "tableitemdelegate.h"
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QVariant>
#include <QBrush>

TableItemDelegate::TableItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

void TableItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    // Проверяем, является ли элемент неисправным (помечен через Qt::UserRole + 1)
    QVariant isFailedVariant = index.data(Qt::UserRole + 1);
    bool isFailed = isFailedVariant.toBool();
    
    if (isFailed) {
        // Получаем цвета напрямую из ролей BackgroundRole и ForegroundRole
        QVariant bgVariant = index.data(Qt::BackgroundRole);
        QVariant fgVariant = index.data(Qt::ForegroundRole);
        
        QColor bgColor;
        QColor textColor;
        
        if (bgVariant.canConvert<QBrush>()) {
            QBrush bgBrush = bgVariant.value<QBrush>();
            bgColor = bgBrush.color();
        }
        
        if (fgVariant.canConvert<QBrush>()) {
            QBrush fgBrush = fgVariant.value<QBrush>();
            textColor = fgBrush.color();
        }
        
        // Если цвета не найдены, используем значения по умолчанию
        if (!bgColor.isValid()) {
            bgColor = QColor(200, 50, 50); // Красный по умолчанию
        }
        if (!textColor.isValid()) {
            textColor = QColor(255, 255, 255); // Белый по умолчанию
        }
        
        // Создаем опцию стиля с кастомными цветами
        QStyleOptionViewItem opt = option;
        opt.palette.setColor(QPalette::Base, bgColor);
        opt.palette.setColor(QPalette::Text, textColor);
        opt.backgroundBrush = QBrush(bgColor);
        
        // Рисуем фон
        painter->fillRect(option.rect, bgColor);
        
        // Получаем текст и выравнивание
        QString text = index.data(Qt::DisplayRole).toString();
        Qt::Alignment align = Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt());
        if (align == 0) {
            align = Qt::AlignLeft | Qt::AlignVCenter; // По умолчанию
        }
        
        // Рисуем текст с правильным выравниванием
        painter->setPen(textColor);
        QRect textRect = option.rect;
        painter->drawText(textRect, align, text);
        
        return;
    }
    
    // Если элемент не помечен как неисправный, используем стандартную отрисовку
    QStyledItemDelegate::paint(painter, option, index);
}

