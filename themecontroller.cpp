#include "themecontroller.h"
#include <QDebug>

ThemeController::ThemeController(QMainWindow* mainWindow,
                                QActionGroup* themeGroup,
                                QAction* deusExAction,
                                QAction* matrixAction,
                                QAction* gurrenLagannAction,
                                QAction* cyberpunkAction,
                                QTextEdit* logWidget,
                                Logger* logger,
                                QObject* parent)
    : QObject(parent), _mainWindow(mainWindow), _logWidget(logWidget), _logger(logger),
      _currentTheme(Theme::DeusEx),
      _themeGroup(themeGroup), _deusExAction(deusExAction), _matrixAction(matrixAction),
      _gurrenLagannAction(gurrenLagannAction), _cyberpunkAction(cyberpunkAction) {
}

void ThemeController::setMainWindow(QMainWindow* window) {
    _mainWindow = window;
}

void ThemeController::setLogger(Logger* logger) {
    _logger = logger;
}

void ThemeController::setLogWidget(QTextEdit* logWidget) {
    _logWidget = logWidget;
}

void ThemeController::applyTheme(Theme theme) {
    qDebug() << "ThemeController::applyTheme: start, theme =" << static_cast<int>(theme);
    _currentTheme = theme;
    
    // Update logger theme
    qDebug() << "ThemeController::applyTheme: updating logger theme";
    if (_logger) {
        _logger->setTheme(theme);
        qDebug() << "ThemeController::applyTheme: logger theme updated";
    } else {
        qDebug() << "ThemeController::applyTheme: WARNING - logger is null";
    }
    
    // Apply stylesheet to main window
    qDebug() << "ThemeController::applyTheme: applying stylesheet to main window";
    if (_mainWindow) {
        QString stylesheet = ThemeManager::getStylesheet(theme);
        qDebug() << "ThemeController::applyTheme: stylesheet obtained, length =" << stylesheet.length();
        _mainWindow->setStyleSheet(stylesheet);
        qDebug() << "ThemeController::applyTheme: stylesheet applied";
    } else {
        qDebug() << "ThemeController::applyTheme: WARNING - mainWindow is null";
    }
    
    // Устанавливаем базовый цвет текста для журнала и перекрашиваем весь существующий текст
    qDebug() << "ThemeController::applyTheme: updating log widget colors";
    if (_logWidget) {
        ThemeColors colors = ThemeManager::getColors(theme);
        QColor logTextColor = colors.logInfo;
        
        // Сохраняем весь текст из журнала
        QString logText = _logWidget->toPlainText();
        
        // Перекрашиваем весь текст в журнале
        if (!logText.isEmpty()) {
            // Перекрашиваем сообщения по типам
            QStringList lines = logText.split('\n');
            _logWidget->clear();
            _logWidget->setTextColor(logTextColor); // Базовый цвет
            
            for (const QString& line : lines) {
                if (line.isEmpty()) continue;
                
                // Определяем тип сообщения по содержимому строки
                if (line.contains("INFO:")) {
                    _logWidget->setTextColor(colors.logInfo);
                } else if (line.contains("WARNING:")) {
                    _logWidget->setTextColor(colors.logWarning);
                } else if (line.contains("ERROR:")) {
                    _logWidget->setTextColor(colors.logError);
                } else if (line.contains("SUCCESS:")) {
                    _logWidget->setTextColor(colors.logSuccess);
                } else {
                    _logWidget->setTextColor(logTextColor); // Базовый цвет для неизвестных типов
                }
                _logWidget->append(line);
            }
        } else {
            // Если журнал пуст, просто устанавливаем базовый цвет
            _logWidget->setTextColor(logTextColor);
        }
        qDebug() << "ThemeController::applyTheme: log widget colors updated";
    } else {
        qDebug() << "ThemeController::applyTheme: WARNING - logWidget is null";
    }
    
    qDebug() << "ThemeController::applyTheme: emitting themeChanged signal";
    emit themeChanged(theme);
    qDebug() << "ThemeController::applyTheme: themeChanged signal emitted, completed";
}

void ThemeController::onThemeChanged() {
    QAction* action = _themeGroup->checkedAction();
    if (action == _deusExAction) {
        applyTheme(Theme::DeusEx);
    } else if (action == _matrixAction) {
        applyTheme(Theme::Matrix);
    } else if (action == _gurrenLagannAction) {
        applyTheme(Theme::GurrenLagann);
    } else if (action == _cyberpunkAction) {
        applyTheme(Theme::Cyberpunk);
    }
}

