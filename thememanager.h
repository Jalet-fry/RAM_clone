#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QColor>
#include <QString>

enum class Theme {
    DeusEx,
    Matrix,
    GurrenLagann,
    Cyberpunk
};

struct ThemeColors {
    // Основные цвета
    QColor bgPrimary;
    QColor bgSecondary;
    QColor text;
    QColor accent;
    QColor accentLight;
    QColor border;
    
    // Цвета для таблицы
    QColor tableBgEven;
    QColor tableBgOdd;
    QColor tableText;
    
    // Специальные цвета (с приоритетом)
    QColor failedTestBg;      // КРАСНОЕ выделение для неисправных тестов
    QColor failedTestText;    // Текст для красного выделения
    QColor passedTestBg;      // Зеленый для исправных
    QColor passedTestText;    // Текст для зеленого
    QColor faultyNotTestedBg; // Золотистый/зеленый для неисправной области (не тестировалась)
    QColor faultyNotTestedText;
    QColor untestedBgEven;    // Темный для непротестированных (четные)
    QColor untestedBgOdd;     // Темный для непротестированных (нечетные)
    
    // Цвета для статуса
    QColor statusPassedBg;
    QColor statusPassedText;
    QColor statusFailedBg;
    QColor statusFailedText;
    QColor statusUntestedBg;
    QColor statusUntestedText;
    
    // Цвета для логов
    QColor logInfo;
    QColor logWarning;
    QColor logError;
    QColor logSuccess;
};

class ThemeManager {
public:
    static ThemeColors getColors(Theme theme);
    static QString getStylesheet(Theme theme);
};

#endif // THEMEMANAGER_H

