@echo off
chcp 65001 >nul
title Остов с максимальным числом висячих вершин

echo.
echo ==================================================
echo    Остов с максимальным числом висячих вершин
echo ==================================================
echo.

python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ОШИБКА] Python не найден в PATH
    echo Установите Python 3.11+ и поставьте галочку "Add to PATH"
    pause
    exit /b 1
)

if not exist "venv\Scripts\python.exe" (
    echo Создаю виртуальное окружение...
    python -m venv venv
    echo.
)

call venv\Scripts\activate.bat

echo Устанавливаю Python-пакеты...
pip install -r requirements.txt >nul 2>&1
if %errorlevel% neq 0 (
    echo [ОШИБКА] Не удалось установить пакеты из requirements.txt
    pause
    exit /b 1
)

echo.
echo Запуск веб-интерфейса...
echo Закрыть окно или нажать Ctrl+C для завершения
echo.
python -m streamlit run src\main.py --server.port=8501