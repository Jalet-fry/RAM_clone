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
        colors.statusPassedText = QColor(212, 175, 55);  // Золотистый (accent цвет) вместо белого
        colors.statusFailedBg = QColor(200, 50, 50);
        colors.statusFailedText = QColor(255, 255, 255);
        colors.statusUntestedBg = QColor(45, 45, 45);
        colors.statusUntestedText = QColor(212, 175, 55);  // Золотистый (accent цвет) вместо белого
        
        // Цвета для логов
        colors.logInfo = QColor(212, 175, 55);  // Золотистый (accent цвет) вместо светло-серого
        colors.logWarning = QColor(244, 208, 63);
        colors.logError = QColor(255, 100, 100);
        colors.logSuccess = QColor(100, 255, 100);
    } else if (theme == Theme::Matrix) {
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
    } else if (theme == Theme::GurrenLagann) {
        // Основные цвета Gurren Lagann
        colors.bgPrimary = QColor(26, 26, 46);      // #1a1a2e темно-синий
        colors.bgSecondary = QColor(15, 15, 30);    // #0f0f1e темно-серый
        colors.text = QColor(255, 215, 0);          // #FFD700 золотисто-желтый
        colors.accent = QColor(255, 0, 0);          // #FF0000 яркий красный
        colors.accentLight = QColor(255, 69, 0);    // #FF4500 оранжево-красный
        colors.border = QColor(255, 0, 0);
        
        // Цвета для таблицы
        colors.tableBgEven = QColor(26, 26, 46);
        colors.tableBgOdd = QColor(35, 35, 55);
        colors.tableText = QColor(255, 215, 0);
        
        // Специальные цвета (ПРИОРИТЕТ)
        colors.failedTestBg = QColor(200, 0, 0);        // ЯРКИЙ КРАСНЫЙ
        colors.failedTestText = QColor(255, 255, 255); // Белый текст
        colors.passedTestBg = QColor(0, 50, 100);      // Темно-синий для успешных
        colors.passedTestText = QColor(0, 102, 255);   // #0066FF яркий синий
        colors.faultyNotTestedBg = QColor(80, 40, 0);
        colors.faultyNotTestedText = QColor(255, 165, 0); // #FFA500 оранжевый
        colors.untestedBgEven = QColor(20, 20, 35);
        colors.untestedBgOdd = QColor(30, 30, 45);
        
        // Цвета для статуса
        colors.statusPassedBg = QColor(0, 100, 150);
        colors.statusPassedText = QColor(0, 153, 255);  // #0099FF яркий синий
        colors.statusFailedBg = QColor(200, 0, 0);
        colors.statusFailedText = QColor(255, 255, 255);
        colors.statusUntestedBg = QColor(15, 15, 30);
        colors.statusUntestedText = QColor(255, 215, 0);  // Золотисто-желтый
        
        // Цвета для логов
        colors.logInfo = QColor(255, 215, 0);      // Золотисто-желтый
        colors.logWarning = QColor(255, 165, 0);   // Оранжевый
        colors.logError = QColor(255, 0, 0);       // Красный
        colors.logSuccess = QColor(0, 153, 255);   // Яркий синий
    } else { // Cyberpunk
        // Основные цвета Cyberpunk
        colors.bgPrimary = QColor(10, 10, 10);         // #0a0a0a очень темный
        colors.bgSecondary = QColor(26, 10, 26);       // #1a0a1a темный фиолетово-черный
        colors.text = QColor(0, 255, 255);            // #00FFFF неоновый циан
        colors.accent = QColor(255, 0, 255);          // #FF00FF неоновый розовый
        colors.accentLight = QColor(255, 20, 147);    // #FF1493 глубокий розовый
        colors.border = QColor(255, 0, 255);
        
        // Цвета для таблицы
        colors.tableBgEven = QColor(10, 10, 10);
        colors.tableBgOdd = QColor(20, 10, 20);
        colors.tableText = QColor(0, 255, 255);
        
        // Специальные цвета (ПРИОРИТЕТ)
        colors.failedTestBg = QColor(200, 0, 0);        // ЯРКИЙ КРАСНЫЙ
        colors.failedTestText = QColor(255, 255, 255);  // Белый текст
        colors.passedTestBg = QColor(30, 0, 50);        // Темно-фиолетовый для успешных
        colors.passedTestText = QColor(128, 0, 255);    // #8000FF фиолетовый
        colors.faultyNotTestedBg = QColor(50, 0, 50);
        colors.faultyNotTestedText = QColor(255, 20, 147); // #FF1493 глубокий розовый
        colors.untestedBgEven = QColor(15, 5, 15);
        colors.untestedBgOdd = QColor(20, 10, 20);
        
        // Цвета для статуса
        colors.statusPassedBg = QColor(50, 0, 100);
        colors.statusPassedText = QColor(128, 0, 255);  // Фиолетовый
        colors.statusFailedBg = QColor(200, 0, 0);
        colors.statusFailedText = QColor(255, 255, 255);
        colors.statusUntestedBg = QColor(26, 10, 26);
        colors.statusUntestedText = QColor(0, 255, 255);  // Неоновый циан
        
        // Цвета для логов
        colors.logInfo = QColor(0, 255, 255);      // Неоновый циан
        colors.logWarning = QColor(255, 255, 0);  // Желтый
        colors.logError = QColor(255, 0, 0);      // Красный
        colors.logSuccess = QColor(128, 0, 255);  // Фиолетовый
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
            "QProgressBar QLabel { color: %2; } "
            "QScrollBar:vertical { background-color: #0a0a0a; width: 12px; border: 1px solid %4; } "
            "QScrollBar::groove:vertical { background-color: #0a0a0a; } "
            "QScrollBar::sub-page:vertical, QScrollBar::add-page:vertical { background-color: #0a0a0a; } "
            "QScrollBar::handle:vertical { background-color: %4; min-height: 20px; border-radius: 6px; } "
            "QScrollBar::handle:vertical:hover { background-color: %6; } "
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; background-color: #0a0a0a; } "
            "QScrollBar:horizontal { background-color: #0a0a0a; height: 12px; border: 1px solid %4; } "
            "QScrollBar::groove:horizontal { background-color: #0a0a0a; } "
            "QScrollBar::sub-page:horizontal, QScrollBar::add-page:horizontal { background-color: #0a0a0a; } "
            "QScrollBar::handle:horizontal { background-color: %4; min-width: 20px; border-radius: 6px; } "
            "QScrollBar::handle:horizontal:hover { background-color: %6; } "
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; background-color: #0a0a0a; } "
            "QLabel { color: %2; } "
        )
        .arg(colors.bgPrimary.name())
        .arg(colors.text.name())
        .arg(colors.bgSecondary.name())
        .arg(colors.accent.name())
        .arg(QColor(61, 61, 61).name()) // #3d3d3d
        .arg(colors.accentLight.name());
    } else if (theme == Theme::Matrix) {
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
            "QProgressBar { background-color: %3; color: #ffffff; border: 2px solid %4; border-radius: 4px; text-align: center; } "
            "QProgressBar::chunk { background-color: %4; } "
            "QProgressBar QLabel { color: #ffffff; } "
            "QScrollBar:vertical { background-color: #0a0a0a; width: 12px; border: 1px solid %4; } "
            "QScrollBar::groove:vertical { background-color: #0a0a0a; } "
            "QScrollBar::sub-page:vertical, QScrollBar::add-page:vertical { background-color: #0a0a0a; } "
            "QScrollBar::handle:vertical { background-color: %4; min-height: 20px; border-radius: 6px; } "
            "QScrollBar::handle:vertical:hover { background-color: %6; } "
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; background-color: #0a0a0a; } "
            "QScrollBar:horizontal { background-color: #0a0a0a; height: 12px; border: 1px solid %4; } "
            "QScrollBar::groove:horizontal { background-color: #0a0a0a; } "
            "QScrollBar::sub-page:horizontal, QScrollBar::add-page:horizontal { background-color: #0a0a0a; } "
            "QScrollBar::handle:horizontal { background-color: %4; min-width: 20px; border-radius: 6px; } "
            "QScrollBar::handle:horizontal:hover { background-color: %6; } "
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; background-color: #0a0a0a; } "
            "QLabel { color: %2; } "
        )
        .arg(colors.bgPrimary.name())
        .arg(colors.text.name())
        .arg(colors.bgSecondary.name())
        .arg(colors.accent.name())
        .arg(QColor(26, 26, 26).name()) // #1a1a1a
        .arg(colors.accentLight.name());
    } else if (theme == Theme::GurrenLagann) {
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
            "QTextEdit { background-color: #0f0f1e; color: %2; border: 2px solid %4; border-radius: 4px; } "
            "QProgressBar { background-color: %3; color: %2; border: 2px solid %4; border-radius: 4px; text-align: center; } "
            "QProgressBar::chunk { background-color: %4; } "
            "QProgressBar QLabel { color: %2; } "
            "QScrollBar:vertical { background-color: #0f0f1e; width: 12px; border: 1px solid %4; } "
            "QScrollBar::groove:vertical { background-color: #0f0f1e; } "
            "QScrollBar::sub-page:vertical, QScrollBar::add-page:vertical { background-color: #0f0f1e; } "
            "QScrollBar::handle:vertical { background-color: %4; min-height: 20px; border-radius: 6px; } "
            "QScrollBar::handle:vertical:hover { background-color: %6; } "
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; background-color: #0f0f1e; } "
            "QScrollBar:horizontal { background-color: #0f0f1e; height: 12px; border: 1px solid %4; } "
            "QScrollBar::groove:horizontal { background-color: #0f0f1e; } "
            "QScrollBar::sub-page:horizontal, QScrollBar::add-page:horizontal { background-color: #0f0f1e; } "
            "QScrollBar::handle:horizontal { background-color: %4; min-width: 20px; border-radius: 6px; } "
            "QScrollBar::handle:horizontal:hover { background-color: %6; } "
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; background-color: #0f0f1e; } "
            "QLabel { color: %2; } "
        )
        .arg(colors.bgPrimary.name())
        .arg(colors.text.name())
        .arg(colors.bgSecondary.name())
        .arg(colors.accent.name())
        .arg(QColor(50, 30, 30).name()) // Темно-красноватый для hover
        .arg(colors.accentLight.name());
    } else { // Cyberpunk
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
            "QProgressBar { background-color: %3; color: #ffffff; border: 2px solid %4; border-radius: 4px; text-align: center; } "
            "QProgressBar::chunk { background-color: %4; } "
            "QProgressBar QLabel { color: #ffffff; } "
            "QScrollBar:vertical { background-color: #0a0a0a; width: 12px; border: 1px solid %4; } "
            "QScrollBar::groove:vertical { background-color: #0a0a0a; } "
            "QScrollBar::sub-page:vertical, QScrollBar::add-page:vertical { background-color: #0a0a0a; } "
            "QScrollBar::handle:vertical { background-color: %4; min-height: 20px; border-radius: 6px; } "
            "QScrollBar::handle:vertical:hover { background-color: %6; } "
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; background-color: #0a0a0a; } "
            "QScrollBar:horizontal { background-color: #0a0a0a; height: 12px; border: 1px solid %4; } "
            "QScrollBar::groove:horizontal { background-color: #0a0a0a; } "
            "QScrollBar::sub-page:horizontal, QScrollBar::add-page:horizontal { background-color: #0a0a0a; } "
            "QScrollBar::handle:horizontal { background-color: %4; min-width: 20px; border-radius: 6px; } "
            "QScrollBar::handle:horizontal:hover { background-color: %6; } "
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; background-color: #0a0a0a; } "
            "QLabel { color: %2; } "
        )
        .arg(colors.bgPrimary.name())
        .arg(colors.text.name())
        .arg(colors.bgSecondary.name())
        .arg(colors.accent.name())
        .arg(QColor(50, 20, 50).name()) // Темно-фиолетовый для hover
        .arg(colors.accentLight.name());
    }
}

