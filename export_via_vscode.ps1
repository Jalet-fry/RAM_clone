# Альтернативный скрипт для экспорта через расширение VS Code
# Использует команды VS Code API

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Экспорт через VS Code расширение" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Путь к папке с диаграммами
$diagramsPath = "docs"

# Проверяем наличие папки
if (-not (Test-Path $diagramsPath)) {
    Write-Host "Ошибка: Папка '$diagramsPath' не найдена!" -ForegroundColor Red
    exit 1
}

# Находим все .puml файлы
$pumlFiles = Get-ChildItem -Path $diagramsPath -Filter "*.puml" -Recurse

if ($pumlFiles.Count -eq 0) {
    Write-Host "Ошибка: Не найдено ни одного .puml файла!" -ForegroundColor Red
    exit 1
}

Write-Host "Найдено диаграмм: $($pumlFiles.Count)" -ForegroundColor Green
Write-Host ""
Write-Host "Этот скрипт использует расширение VS Code для экспорта." -ForegroundColor Yellow
Write-Host "Убедитесь, что:" -ForegroundColor Yellow
Write-Host "  1. Расширение PlantUML установлено" -ForegroundColor Yellow
Write-Host "  2. VS Code открыт с этой папкой" -ForegroundColor Yellow
Write-Host "  3. Используйте команду: PlantUML: Export All Diagrams" -ForegroundColor Yellow
Write-Host ""
Write-Host "Или используйте основной скрипт: export_all_diagrams.ps1" -ForegroundColor Cyan
Write-Host ""

# Показываем список файлов для экспорта
Write-Host "Файлы для экспорта:" -ForegroundColor Green
foreach ($file in $pumlFiles) {
    Write-Host "  - $($file.Name)" -ForegroundColor Gray
}

Write-Host ""
Write-Host "Для автоматического экспорта используйте:" -ForegroundColor Cyan
Write-Host "  .\export_all_diagrams.ps1" -ForegroundColor White
Write-Host ""

