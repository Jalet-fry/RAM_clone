#ifndef LOGGER_H
#define LOGGER_H

#include <QTextEdit>
#include <QTime>
#include "thememanager.h"

class Logger {
public:
    explicit Logger(QTextEdit* logWidget, Theme theme = Theme::DeusEx);
    
    void setTheme(Theme theme);
    void info(const QString& message);
    void warning(const QString& message);
    void error(const QString& message);
    void success(const QString& message);
    void clear();

private:
    void appendLog(const QString& message, const QColor& color);
    
    QTextEdit* _log;
    Theme _theme;
};

#endif // LOGGER_H

