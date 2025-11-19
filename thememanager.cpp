#include "thememanager.h"

ThemeColors ThemeManager::getColors(Theme theme) {
    ThemeColors colors;
    
    if (theme == Theme::DeusEx) {
        // Основные цвета Deus Ex
        colors.bgPrimary = QColor(26, 26, 26);
        colors.bgSecondary = QColor(45, 45, 45);
        colors.text = QColor(224, 224, 224);
        colors.accent = QColor(212, 175, 55);  // #d4af37
        colors.accentLight = QColor(244, 208, 63); // #f4d03f
        colors.border = QColor(212, 175, 55);
        
        // Цвета для таблицы
        colors.tableBgEven = QColor(26, 26, 26);
        colors.tableBgOdd = QColor(35, 35, 35);
        colors.tableText = QColor(224, 224, 224);
        
        // Специальные цвета (ПРИОРИТЕТ)
        colors.failedTestBg = QColor(200, 50, 50);      // ЯРКИЙ КРАСНЫЙ
        colors.failedTestText = QColor(255, 255, 255); // Белый текст
        colors.passedTestBg = QColor(40, 50, 40);
        colors.passedTestText = QColor(212, 175, 55);  // Золотистый (accent цвет)
        colors.faultyNotTestedBg = QColor(80, 70, 40);
        colors.faultyNotTestedText = QColor(244, 208, 63);
        colors.untestedBgEven = QColor(30, 30, 30);
        colors.untestedBgOdd = QColor(35, 35, 35);
        
        // Цвета для статуса
        colors.statusPassedBg = QColor(100, 150, 100);
        colors.statusPassedText = QColor(255, 255, 255);
        colors.statusFailedBg = QColor(200, 50, 50);
        colors.statusFailedText = QColor(255, 255, 255);
        colors.statusUntestedBg = QColor(45, 45, 45);
        colors.statusUntestedText = QColor(212, 175, 55);  // Золотистый (accent цвет) вместо белого
        
        // Цвета для логов
        colors.logInfo = QColor(224, 224, 224);
        colors.logWarning = QColor(244, 208, 63);
        colors.logError = QColor(255, 100, 100);
        colors.logSuccess = QColor(100, 255, 100);
    } else { // Matrix
        // Основные цвета Matrix
        colors.bgPrimary = QColor(0, 0, 0);
        colors.bgSecondary = QColor(10, 10, 10);
        colors.text = QColor(0, 255, 65);  // #00ff41
        colors.accent = QColor(0, 255, 0);  // #00ff00
        colors.accentLight = QColor(0, 255, 65);
        colors.border = QColor(0, 204, 0);
        
        // Цвета для таблицы
        colors.tableBgEven = QColor(0, 0, 0);
        colors.tableBgOdd = QColor(10, 10, 10);
        colors.tableText = QColor(0, 255, 65);
        
        // Специальные цвета (ПРИОРИТЕТ)
        colors.failedTestBg = QColor(200, 0, 0);        // ЯРКИЙ КРАСНЫЙ
        colors.failedTestText = QColor(255, 255, 255);  // Белый текст
        colors.passedTestBg = QColor(0, 30, 0);
        colors.passedTestText = QColor(0, 255, 65);
        colors.faultyNotTestedBg = QColor(0, 50, 0);
        colors.faultyNotTestedText = QColor(0, 255, 0);
        colors.untestedBgEven = QColor(5, 5, 5);
        colors.untestedBgOdd = QColor(10, 10, 10);
        
        // Цвета для статуса
        colors.statusPassedBg = QColor(0, 100, 0);
        colors.statusPassedText = QColor(0, 255, 65);  // Зелёный (как tableText) вместо белого
        colors.statusFailedBg = QColor(200, 0, 0);
        colors.statusFailedText = QColor(255, 255, 255);
        colors.statusUntestedBg = QColor(10, 10, 10);
        colors.statusUntestedText = QColor(0, 255, 65);
        
        // Цвета для логов
        colors.logInfo = QColor(0, 255, 65);
        colors.logWarning = QColor(255, 255, 0);
        colors.logError = QColor(255, 0, 0);
        colors.logSuccess = QColor(0, 255, 0);
    }
    
    return colors;
}

QString ThemeManager::getStylesheet(Theme theme) {
    ThemeColors colors = getColors(theme);
    
    if (theme == Theme::DeusEx) {
        return QString(
            "QMainWindow { background-color: %1; } "
            "QWidget { background-color: %1; color: %2; } "
            "QMenuBar { background-color: %3; color: %2; border-bottom: 1px solid %4; } "
            "QMenuBar::item { background-color: transparent; color: %2; padding: 4px 8px; } "
            "QMenuBar::item:selected { background-color: %5; color: %4; } "
            "QMenuBar::item:pressed { background-color: %1; } "
            "QMenu { background-color: %3; color: %2; border: 1px solid %4; } "
            "QMenu::item { background-color: transparent; padding: 4px 20px; } "
            "QMenu::item:selected { background-color: %5; color: %4; } "
            "QMenu::item:checked { background-color: %5; color: %4; } "
            "QGroupBox { background-color: %3; color: %2; border: 2px solid %4; border-radius: 5px; margin-top: 10px; padding-top: 10px; font-weight: bold; } "
            "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; color: %4; } "
            "QPushButton { background-color: %3; color: %2; border: 2px solid %4; border-radius: 4px; padding: 5px; min-width: 80px; } "
            "QPushButton:hover { background-color: %5; border-color: %6; } "
            "QPushButton:pressed { background-color: %1; } "
            "QComboBox { background-color: %3; color: %2; border: 2px solid %4; border-radius: 4px; padding: 3px; } "
            "QComboBox:hover { border-color: %6; } "
            "QComboBox::drop-down { border: none; } "
            "QComboBox::down-arrow { image: none; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 5px solid %4; } "
            "QComboBox QAbstractItemView { background-color: %3; color: %2; selection-background-color: %4; selection-color: %1; border: 1px solid %4; } "
            "QLineEdit { background-color: %3; color: %2; border: 2px solid %4; border-radius: 4px; padding: 3px; } "
            "QLineEdit:focus { border-color: %6; } "
            "QDoubleSpinBox { background-color: %3; color: %2; border: 2px solid %4; border-radius: 4px; padding: 3px; } "
            "QDoubleSpinBox:focus { border-color: %6; } "
            "QTableWidget { background-color: %1; color: %2; border: 2px solid %4; gridline-color: %5; } "
            "QTableWidget::item { border: none; } "
            "QTableWidget::item:selected { background-color: %4; color: %1; } "
            "QHeaderView::section { background-color: %3; color: %4; border: 1px solid %4; padding: 5px; font-weight: bold; } "
            "QTextEdit { background-color: #0a0a0a; color: %2; border: 2px solid %4; border-radius: 4px; } "
            "QProgressBar { background-color: %3; color: %2; border: 2px solid %4; border-radius: 4px; text-align: center; } "
            "QProgressBar::chunk { background-color: %4; } "
            "QLabel { color: %2; } "
        )
        .arg(colors.bgPrimary.name())
        .arg(colors.text.name())
        .arg(colors.bgSecondary.name())
        .arg(colors.accent.name())
        .arg(QColor(61, 61, 61).name()) // #3d3d3d
        .arg(colors.accentLight.name());
    } else { // Matrix
        return QString(
            "QMainWindow { background-color: %1; } "
            "QWidget { background-color: %1; color: %2; } "
            "QMenuBar { background-color: %3; color: %2; border-bottom: 1px solid %4; } "
            "QMenuBar::item { background-color: transparent; color: %2; padding: 4px 8px; } "
            "QMenuBar::item:selected { background-color: %5; color: %4; } "
            "QMenuBar::item:pressed { background-color: %1; } "
            "QMenu { background-color: %3; color: %2; border: 1px solid %4; } "
            "QMenu::item { background-color: transparent; padding: 4px 20px; } "
            "QMenu::item:selected { background-color: %5; color: %4; } "
            "QMenu::item:checked { background-color: %5; color: %4; } "
            "QGroupBox { background-color: %3; color: %2; border: 2px solid %4; border-radius: 5px; margin-top: 10px; padding-top: 10px; font-weight: bold; } "
            "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; color: %4; } "
            "QPushButton { background-color: %3; color: %2; border: 2px solid %4; border-radius: 4px; padding: 5px; min-width: 80px; } "
            "QPushButton:hover { background-color: %5; border-color: %6; } "
            "QPushButton:pressed { background-color: %1; } "
            "QComboBox { background-color: %3; color: %2; border: 2px solid %4; border-radius: 4px; padding: 3px; } "
            "QComboBox:hover { border-color: %6; } "
            "QComboBox::drop-down { border: none; } "
            "QComboBox::down-arrow { image: none; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 5px solid %4; } "
            "QComboBox QAbstractItemView { background-color: %3; color: %2; selection-background-color: %4; selection-color: %1; border: 1px solid %4; } "
            "QLineEdit { background-color: %3; color: %2; border: 2px solid %4; border-radius: 4px; padding: 3px; } "
            "QLineEdit:focus { border-color: %6; } "
            "QDoubleSpinBox { background-color: %3; color: %2; border: 2px solid %4; border-radius: 4px; padding: 3px; } "
            "QDoubleSpinBox:focus { border-color: %6; } "
            "QTableWidget { background-color: %1; color: %2; border: 2px solid %4; gridline-color: %5; } "
            "QTableWidget::item { border: none; } "
            "QTableWidget::item:selected { background-color: %4; color: %1; } "
            "QHeaderView::section { background-color: %3; color: %4; border: 1px solid %4; padding: 5px; font-weight: bold; } "
            "QTextEdit { background-color: %1; color: %2; border: 2px solid %4; border-radius: 4px; } "
            "QProgressBar { background-color: %3; color: %2; border: 2px solid %4; border-radius: 4px; text-align: center; } "
            "QProgressBar::chunk { background-color: %4; } "
            "QLabel { color: %2; } "
        )
        .arg(colors.bgPrimary.name())
        .arg(colors.text.name())
        .arg(colors.bgSecondary.name())
        .arg(colors.accent.name())
        .arg(QColor(26, 26, 26).name()) // #1a1a1a
        .arg(colors.accentLight.name());
    }
}

