# Export all PlantUML diagrams to PNG
# Automatically downloads plantuml.jar if needed

$ErrorActionPreference = "Continue"

# Устанавливаем UTF-8 кодировку для корректной работы с русскими буквами
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Exporting all PlantUML diagrams to PNG" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Сохраняем исходное расположение
$initialLocation = Get-Location

# Получаем директорию, где находится скрипт
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $scriptDir) {
    $scriptDir = Get-Location
}

# Устанавливаем пути относительно расположения скрипта
$diagramsPath = Join-Path $scriptDir "docs"
$plantumlJar = Join-Path $scriptDir "plantuml.jar"
$outputFormat = "png"
$plantumlVersion = "1.2024.5"
$plantumlUrl = "https://github.com/plantuml/plantuml/releases/download/v$plantumlVersion/plantuml-$plantumlVersion.jar"

# Переходим в директорию скрипта
Push-Location $scriptDir

if (-not (Test-Path $diagramsPath)) {
    Write-Host "Error: Folder '$diagramsPath' not found!" -ForegroundColor Red
    Pop-Location
    Set-Location $initialLocation
    exit 1
}

Write-Host "Checking Java..." -NoNewline
$javaCmd = Get-Command java -ErrorAction SilentlyContinue
if (-not $javaCmd) {
    Write-Host " Not found" -ForegroundColor Red
    Write-Host ""
    Write-Host "Error: Java is not installed!" -ForegroundColor Red
    Write-Host "Install Java: https://www.java.com/download/" -ForegroundColor Yellow
    Write-Host "Or use: choco install openjdk" -ForegroundColor Yellow
    exit 1
}
Write-Host " Found" -ForegroundColor Green

if (-not (Test-Path $plantumlJar)) {
    Write-Host ""
    Write-Host "PlantUML not found. Downloading..." -ForegroundColor Yellow
    try {
        Write-Host "Downloading plantuml.jar..." -NoNewline
        Invoke-WebRequest -Uri $plantumlUrl -OutFile $plantumlJar -UseBasicParsing
        Write-Host " Success" -ForegroundColor Green
    } catch {
        Write-Host " Error" -ForegroundColor Red
        Write-Host ""
        Write-Host "Failed to download PlantUML automatically." -ForegroundColor Red
        Write-Host "Download manually: $plantumlUrl" -ForegroundColor Yellow
        Write-Host "Or install via: choco install plantuml" -ForegroundColor Yellow
        exit 1
    }
} else {
    Write-Host "PlantUML found: $plantumlJar" -ForegroundColor Green
}

Write-Host ""

$pumlFiles = Get-ChildItem -Path $diagramsPath -Filter "*.puml" -Recurse

if ($pumlFiles.Count -eq 0) {
    Write-Host "Error: No .puml files found in '$diagramsPath'!" -ForegroundColor Red
    Pop-Location
    Set-Location $initialLocation
    exit 1
}

Write-Host "Found diagrams: $($pumlFiles.Count)" -ForegroundColor Green
Write-Host ""

# Сохраняем текущее расположение (директория скрипта)
$originalLocation = Get-Location
Push-Location $diagramsPath

try {
    Write-Host "Exporting all diagrams to PNG..." -ForegroundColor Cyan
    $javaPath = $javaCmd.Source
    # $plantumlJar уже содержит полный путь
    $jarPath = $plantumlJar
    
    # Получаем список всех .puml файлов для явной обработки
    $pumlFilesList = Get-ChildItem -Filter "*.puml" | ForEach-Object { $_.Name }
    
    Write-Host "Found .puml files:" -ForegroundColor Gray
    foreach ($file in $pumlFilesList) {
        Write-Host "  - $file" -ForegroundColor Gray
    }
    Write-Host ""
    
    # Устанавливаем переменные окружения для UTF-8 и качества
    $env:LANG = "ru_RU.UTF-8"
    # Увеличиваем DPI для более четкого текста (300 DPI вместо стандартных 96)
    $env:PLANTUML_DPI = "300"
    
    # Обрабатываем каждый файл отдельно для лучшей диагностики
    $successCount = 0
    $failCount = 0
    
    foreach ($pumlFile in $pumlFilesList) {
        Write-Host "Processing: $pumlFile" -NoNewline -ForegroundColor Cyan
        
        # Используем массив аргументов для правильной передачи параметров Java
        # -DPLANTUML_LIMIT_SIZE увеличивает максимальный размер изображения для лучшего качества
        # -Djava.awt.headless=true для работы без GUI
        $javaArgs = @(
            "-Dfile.encoding=UTF-8",
            "-Djava.awt.headless=true",
            "-DPLANTUML_LIMIT_SIZE=8192",
            "-jar",
            $jarPath,
            "-tpng",
            "-charset",
            "UTF-8",
            $pumlFile
        )
        
        $result = & $javaPath $javaArgs 2>&1
        $exitCode = $LASTEXITCODE
        
        if ($exitCode -eq 0) {
            Write-Host " [OK]" -ForegroundColor Green
            $successCount++
        } else {
            Write-Host " [FAIL]" -ForegroundColor Red
            $failCount++
            foreach ($line in $result) {
                if ($line -match "Error|ERROR|error|Exception") {
                    Write-Host "    $line" -ForegroundColor Yellow
                }
            }
        }
    }
    
    Write-Host ""
    
    $pngFiles = Get-ChildItem -Filter "*.png" -ErrorAction SilentlyContinue
    
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Cyan
    $summaryColor = if ($failCount -eq 0) { "Green" } else { "Yellow" }
    Write-Host "Summary: $successCount succeeded, $failCount failed" -ForegroundColor $summaryColor
    Write-Host "========================================" -ForegroundColor Cyan
    
    if ($pngFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "Export completed! Created $($pngFiles.Count) PNG file(s)" -ForegroundColor Green
        Write-Host ""
        Write-Host "Created PNG files:" -ForegroundColor Green
        foreach ($png in $pngFiles) {
            Write-Host "  - $($png.Name)" -ForegroundColor Gray
        }
    } else {
        Write-Host ""
        Write-Host "Warning: No PNG files were created!" -ForegroundColor Yellow
        Write-Host "Check for errors in the output above." -ForegroundColor Yellow
    }
    
    Write-Host ""
    
    # Exit with 0 even if there were some errors, as long as some files were created
    if ($pngFiles.Count -gt 0) {
        exit 0
    } else {
        exit 1
    }
} catch {
    Write-Host ""
    $errorMessage = $_.Exception.Message
    Write-Host "Error: $errorMessage" -ForegroundColor Red
    exit 1
} finally {
    Pop-Location  # Возвращаемся из docs в директорию скрипта
    Set-Location $initialLocation  # Возвращаемся в исходную директорию
}

Write-Host ""

