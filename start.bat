@echo off
chcp 1251 >nul
title Остов с максимальным числом висячих вершин

echo.
echo ==================================================
echo    Остов с максимальным числом висячих вершин
echo ==================================================
echo.

:: Проверяем Python
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ОШИБКА] Python не найден в PATH
    echo Установите Python 3.11+ и поставьте галочку "Add to PATH"
    pause
    exit /b 1
)

:: Создаём venv, если нет
if not exist "venv\Scripts\python.exe" (
    echo Создаю виртуальное окружение...
    python -m venv venv
    echo.
)

:: Активируем venv
call venv\Scripts\activate.bat

:: Устанавливаем пакеты
echo Устанавливаю Python-пакеты...
pip install -r requirements.txt
if %errorlevel% neq 0 (
    echo [ОШИБКА] Не удалось установить пакеты из requirements.txt
    pause
    exit /b 1
)

:: Запускаем Streamlit в этом же окне
echo.
echo Запуск веб-интерфейса...
echo Закрыть окно или нажать Ctrl+C для завершения
echo.
python -m streamlit run src\main.py --server.port=8501
