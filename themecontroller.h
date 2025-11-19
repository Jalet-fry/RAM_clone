#ifndef THEMECONTROLLER_H
#define THEMECONTROLLER_H

#include <QObject>
#include <QMainWindow>
#include <QActionGroup>
#include <QAction>
#include <QTextEdit>
#include "thememanager.h"
#include "logger.h"

class ThemeController : public QObject {
    Q_OBJECT
public:
    explicit ThemeController(QMainWindow* mainWindow,
                            QActionGroup* themeGroup,
                            QAction* deusExAction,
                            QAction* matrixAction,
                            QAction* gurrenLagannAction,
                            QAction* cyberpunkAction,
                            QTextEdit* logWidget,
                            Logger* logger,
                            QObject* parent = nullptr);
    
    void setMainWindow(QMainWindow* window);
    void setLogger(Logger* logger);
    void setLogWidget(QTextEdit* logWidget);
    
    Theme getCurrentTheme() const { return _currentTheme; }
    void applyTheme(Theme theme);
    
public slots:
    void onThemeChanged();
    
signals:
    void themeChanged(Theme theme);
    
private:
    QMainWindow* _mainWindow;
    QTextEdit* _logWidget;
    Logger* _logger;
    Theme _currentTheme;
    
    // UI elements (not owned)
    QActionGroup* _themeGroup;
    QAction* _deusExAction;
    QAction* _matrixAction;
    QAction* _gurrenLagannAction;
    QAction* _cyberpunkAction;
};

#endif // THEMECONTROLLER_H

