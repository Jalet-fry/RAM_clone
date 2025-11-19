#include "logger.h"
#include "thememanager.h"

Logger::Logger(QTextEdit* logWidget, Theme theme)
    : _log(logWidget), _theme(theme) {
}

void Logger::setTheme(Theme theme) {
    _theme = theme;
}

void Logger::appendLog(const QString& message, const QColor& color) {
    if (!_log) return;
    
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    _log->setTextColor(color);
    _log->append(QString("[%1] %2").arg(timestamp).arg(message));
}

void Logger::info(const QString& message) {
    ThemeColors colors = ThemeManager::getColors(_theme);
    appendLog(QString("INFO: %1").arg(message), colors.logInfo);
}

void Logger::warning(const QString& message) {
    ThemeColors colors = ThemeManager::getColors(_theme);
    appendLog(QString("WARNING: %1").arg(message), colors.logWarning);
}

void Logger::error(const QString& message) {
    ThemeColors colors = ThemeManager::getColors(_theme);
    appendLog(QString("ERROR: %1").arg(message), colors.logError);
}

void Logger::success(const QString& message) {
    ThemeColors colors = ThemeManager::getColors(_theme);
    appendLog(QString("SUCCESS: %1").arg(message), colors.logSuccess);
}

void Logger::clear() {
    if (_log) {
        _log->clear();
        // Reset text color to default theme color after clearing
        ThemeColors colors = ThemeManager::getColors(_theme);
        _log->setTextColor(colors.logInfo);
    }
}

