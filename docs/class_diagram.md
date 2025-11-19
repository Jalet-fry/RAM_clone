# Диаграмма классов проекта RAM_clone

## Общая архитектура

```mermaid
classDiagram
    class MainWindow {
        -MemoryModel* _mem
        -unique_ptr~TesterWorker~ _worker
        -unique_ptr~Logger~ _logger
        -unique_ptr~MemoryTableManager~ _tableManager
        -unique_ptr~StatisticsManager~ _statisticsManager
        -unique_ptr~FaultController~ _faultController
        -unique_ptr~TestController~ _testController
        -unique_ptr~ResultsNavigator~ _resultsNavigator
        -unique_ptr~ThemeController~ _themeController
        +MainWindow(parent)
        +~MainWindow()
        -onTableDataChanged(begin, end)
        -onFaultInjected()
        -onMemoryReset()
        -onTestStarted()
        -onTestFinished(results)
        -onThemeChanged(theme)
    }

    class MemoryModel {
        -QMutex _mutex
        -vector~Word~ _words
        -unique_ptr~FaultInjector~ _faultInjector
        +size() size_t
        +reset()
        +read(addr) Word
        +write(addr, value)
        +writeDirect(addr, value)
        +injectFault(fault)
        +currentFault() InjectedFault
        +dataChanged(begin, end)$
        +faultInjected()$
        +errorOccurred(message)$
    }

    class FaultInjector {
        -QMutex _mutex
        -InjectedFault _injected
        +injectFault(fault)
        +currentFault() InjectedFault
        +reset()
        +applyFault(addr, value) Word
        +isAddrFaulty(addr) bool
        -_isAddrFaultyUnlocked(addr) bool
        -getRNG() mt19937&
    }

    class MemoryTester {
        -MemoryModel* _mem
        -vector~TestResult~ _results
        +runTest(algo)
        -writePattern(addr, pattern)
        -readAndVerify(addr, expected)
        -updateProgress(current, total, phasePercent, basePercent)
        +progress(percent)$
        +progressDetail(addr, expected, read)$
        +finished(results)$
    }

    class TesterWorker {
        -QThread _thread
        -MemoryModel* _mem
        -unique_ptr~MemoryTester~ _tester
        +initialize()
        +run(algo)
        +progress(percent)$
        +progressDetail(addr, expected, read)$
        +finished(results)$
    }

    class FaultController {
        -MemoryModel* _mem
        -Logger* _logger
        -QComboBox* _faultCombo
        -QLineEdit* _addrEdit
        -QLineEdit* _lenEdit
        -QDoubleSpinBox* _flipProbSpin
        +injectFault()
        +resetMemory()
        +onFaultModelChanged(index)
        -validateInput(addr, len) bool
        +faultInjected()$
        +memoryReset()$
    }

    class TestController {
        -TesterWorker* _worker
        -MemoryModel* _mem
        -MemoryTableManager* _tableManager
        -Logger* _logger
        -bool _testRunning
        -QTime _testStartTime
        -int _lastTestTimeMsecs
        +startTest()
        +onTestFinished(results)
        +onAlgorithmChanged(index)
        +updateProgressDetails(addr, expected, read)
        +testStarted()$
        +testFinished(results)$
        +testResultsUpdated(results)$
    }

    class MemoryTableManager {
        -QTableWidget* _table
        -MemoryModel* _mem
        -Logger* _logger
        -Theme _currentTheme
        -vector~TestResult~ _lastResults
        +setTestResults(results)
        +clearTestResults()
        +setTheme(theme)
        +refreshTable(begin, end)
        +highlightAddress(addr)
        +updateProgressHighlight(addr, lastAddr)
    }

    class StatisticsManager {
        -MemoryModel* _mem
        -vector~TestResult~ _lastResults
        -Theme _currentTheme
        -int _testTimeMsecs
        -QLabel* _totalAddressesLabel
        -QLabel* _testedAddressesLabel
        -QLabel* _faultsFoundLabel
        -QLabel* _coverageLabel
        +setTestResults(results)
        +clearTestResults()
        +setTestTime(msecs)
        +setTheme(theme)
        +updateStatistics()
        +updateFaultInfo()
    }

    class ThemeController {
        -QMainWindow* _mainWindow
        -QTextEdit* _logWidget
        -Logger* _logger
        -Theme _currentTheme
        -QActionGroup* _themeGroup
        +applyTheme(theme)
        +onThemeChanged()
        +themeChanged(theme)$
    }

    class ResultsNavigator {
        -QTableWidget* _table
        -MemoryModel* _mem
        -Logger* _logger
        +scrollToNextFault(results, currentAddr)
    }

    class Logger {
        -QTextEdit* _log
        -Theme _theme
        +setTheme(theme)
        +info(message)
        +warning(message)
        +error(message)
        +success(message)
        +clear()
    }

    class DataFormatter {
        +formatBinary(value)$ QString
        +getFaultModelName(model)$ QString
        +getAlgorithmDescription(algo)$ QString
    }

    class ThemeManager {
        +getColors(theme)$ ThemeColors
        +getStylesheet(theme)$ QString
    }

    class TableItemDelegate {
        -Theme _theme
        +setTheme(theme)
        +paint(painter, option, index)
    }

    MainWindow --> MemoryModel : owns
    MainWindow --> TesterWorker : owns
    MainWindow --> Logger : owns
    MainWindow --> MemoryTableManager : owns
    MainWindow --> StatisticsManager : owns
    MainWindow --> FaultController : owns
    MainWindow --> TestController : owns
    MainWindow --> ResultsNavigator : owns
    MainWindow --> ThemeController : owns

    MemoryModel --> FaultInjector : owns
    TesterWorker --> MemoryTester : owns
    TesterWorker --> MemoryModel : uses
    MemoryTester --> MemoryModel : uses
    FaultController --> MemoryModel : uses
    FaultController --> Logger : uses
    TestController --> TesterWorker : uses
    TestController --> MemoryModel : uses
    TestController --> MemoryTableManager : uses
    TestController --> Logger : uses
    MemoryTableManager --> MemoryModel : uses
    MemoryTableManager --> Logger : uses
    StatisticsManager --> MemoryModel : uses
    ResultsNavigator --> MemoryModel : uses
    ResultsNavigator --> Logger : uses
    ThemeController --> Logger : uses
```

## Описание классов

### Основные компоненты

#### MainWindow
Главное окно приложения. Управляет всеми компонентами UI и координирует работу между контроллерами и менеджерами.

#### MemoryModel
Модель данных памяти. Обеспечивает потокобезопасный доступ к данным памяти и управляет внедрением неисправностей.

#### FaultInjector
Класс для внедрения неисправностей в память. Применяет различные модели неисправностей (Stuck-at-0, Stuck-at-1, Bit-flip, Open Read) при чтении данных.

#### MemoryTester
Реализует алгоритмы тестирования памяти (Walking 1s, Walking 0s, March Simple). Выполняется в отдельном потоке.

#### TesterWorker
Обертка для выполнения тестирования в отдельном потоке. Управляет жизненным циклом потока и MemoryTester.

### Контроллеры

#### FaultController
Контроллер для управления неисправностями. Обрабатывает пользовательский ввод и валидирует данные перед внедрением неисправностей.

#### TestController
Контроллер для управления тестированием. Запускает тесты, отслеживает прогресс и обрабатывает результаты.

#### ThemeController
Контроллер для управления темами интерфейса. Применяет различные цветовые схемы к приложению.

### Менеджеры

#### MemoryTableManager
Управляет отображением таблицы памяти. Обновляет данные, применяет цветовое кодирование и выделение.

#### StatisticsManager
Управляет отображением статистики тестирования. Вычисляет покрытие, количество найденных неисправностей и другие метрики.

#### ResultsNavigator
Навигация по результатам тестирования. Позволяет переходить к следующей найденной неисправности.

### Вспомогательные классы

#### Logger
Класс для логирования событий в текстовое поле с поддержкой цветового форматирования.

#### DataFormatter
Статический класс для форматирования данных (двоичное представление, названия моделей неисправностей и алгоритмов).

#### ThemeManager
Статический класс для управления темами. Предоставляет цвета и стили для различных тем.

#### TableItemDelegate
Кастомный делегат для отрисовки элементов таблицы с поддержкой специального выделения неисправных ячеек.

## Типы данных

```mermaid
classDiagram
    class Word {
        <<typedef>>
        uint32_t
    }

    class FaultModel {
        <<enum>>
        None
        StuckAt0
        StuckAt1
        BitFlip
        OpenRead
    }

    class TestAlgorithm {
        <<enum>>
        WalkingOnes
        WalkingZeros
        MarchSimple
    }

    class InjectedFault {
        +FaultModel model
        +size_t addr
        +size_t len
        +double flip_probability
    }

    class TestResult {
        +size_t addr
        +Word expected
        +Word read
        +bool passed
    }

    class Theme {
        <<enum>>
        DeusEx
        Matrix
        GurrenLagann
        Cyberpunk
    }

    class ThemeColors {
        +QColor bgPrimary
        +QColor bgSecondary
        +QColor text
        +QColor accent
        +QColor accentLight
        +QColor border
        +QColor tableBgEven
        +QColor tableBgOdd
        +QColor tableText
        +QColor failedTestBg
        +QColor failedTestText
        +QColor passedTestBg
        +QColor passedTestText
        +QColor faultyNotTestedBg
        +QColor faultyNotTestedText
        +QColor untestedBgEven
        +QColor untestedBgOdd
        +QColor statusPassedBg
        +QColor statusPassedText
        +QColor statusFailedBg
        +QColor statusFailedText
        +QColor statusUntestedBg
        +QColor statusUntestedText
        +QColor logInfo
        +QColor logWarning
        +QColor logError
        +QColor logSuccess
    }
```

## Потоки выполнения

```mermaid
sequenceDiagram
    participant UI as MainWindow (UI Thread)
    participant TC as TestController
    participant TW as TesterWorker
    participant MT as MemoryTester
    participant MM as MemoryModel

    UI->>TC: startTest()
    TC->>TW: run(algo) [QueuedConnection]
    TW->>MT: runTest(algo)
    
    loop For each address
        MT->>MM: writeDirect(addr, pattern)
        MM-->>MT: success
        MT->>MT: updateProgress()
        MT-->>TW: progress(percent) [DirectConnection]
        TW-->>TC: progress(percent) [QueuedConnection]
        TC-->>UI: updateProgressBar()
        
        MT->>MM: read(addr)
        MM->>MM: applyFault()
        MM-->>MT: value (with fault)
        MT->>MT: readAndVerify()
        MT-->>TW: progressDetail(addr, expected, read) [DirectConnection]
        TW-->>TC: progressDetail(...) [QueuedConnection]
        TC-->>UI: updateProgressDetails()
    end
    
    MT-->>TW: finished(results) [DirectConnection]
    TW-->>TC: finished(results) [QueuedConnection]
    TC-->>UI: onTestFinished(results)
    UI->>UI: updateTable()
    UI->>UI: updateStatistics()
```

