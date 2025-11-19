# Автоматический экспорт PlantUML диаграмм в PNG

## Быстрый старт

Просто запустите один из скриптов:

### Вариант 1: Через BAT файл (проще всего)
```bash
export_all_diagrams.bat
```

### Вариант 2: Через PowerShell
```powershell
.\export_all_diagrams.ps1
```

## Что делает скрипт

1. ✅ Проверяет наличие Java
2. ✅ Автоматически скачивает `plantuml.jar` если его нет
3. ✅ Находит все `.puml` файлы в папке `docs/`
4. ✅ Экспортирует их все в PNG формат
5. ✅ Сохраняет PNG файлы рядом с исходными `.puml` файлами

## Требования

- **Java** (JDK или JRE)
  - Если нет: установите через `choco install openjdk`
  - Или скачайте: https://www.java.com/download/

## Результат

После выполнения скрипта все PNG файлы будут созданы в папке `docs/` рядом с исходными `.puml` файлами.

Например:
- `docs/class_diagram.puml` → `docs/class_diagram.png`
- `docs/usecase_diagram.puml` → `docs/usecase_diagram.png`

## Альтернативные способы

### Через расширение VS Code
1. Откройте VS Code с этой папкой
2. Нажмите `Ctrl+Shift+P`
3. Выберите: `PlantUML: Export All Diagrams in Current Workspace`

### Через горячие клавиши (для одной диаграммы)
1. Откройте любой `.puml` файл
2. Нажмите `Ctrl+Alt+E` для экспорта в PNG

## Устранение проблем

### Ошибка: "Java не найдена"
- Установите Java: https://www.java.com/download/
- Или через Chocolatey: `choco install openjdk`

### Ошибка: "Не удалось скачать PlantUML"
- Проверьте интернет-соединение
- Скачайте вручную: https://github.com/plantuml/plantuml/releases
- Сохраните как `plantuml.jar` в корне проекта

### PNG файлы не создаются
- Проверьте, что файлы `.puml` имеют правильный синтаксис
- Убедитесь, что файлы начинаются с `@startuml` и заканчиваются `@enduml`

