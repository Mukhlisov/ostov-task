@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

echo.
echo ==================================================
echo    Остов с максимальным числом висячих вершин
echo ==================================================
echo.

REM 1. Проверяем Python
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ОШИБКА] Python не установлен!
    echo Установите Python 3.11+ с сайта python.org (обязательно поставьте галочку "Add to PATH")
    pause
    exit /b 1
)

REM 2. Создаём venv, если его нет
if not exist "venv\Scripts\python.exe" (
    echo Создаю виртуальное окружение...
    python -m venv venv
)

REM 3. Активируем venv
call venv\Scripts\activate.bat

REM 4. Устанавливаем пакеты
echo Устанавливаю нужные пакеты (может занять 10–30 секунд...)
pip install -r requirements.txt
if %errorlevel% neq 1 (
    echo [ОШИБКА] Не удалось установить пакеты
    pause
)

REM 5. Проверяем бинарник C++
set "BIN=src\ostov_task.exe"
if not exist "%BIN%" (
    echo Собираю C++ exe...
    cmake -S src\ostov_task -B src\ostov_task\cmake-build-release -DCMAKE_BUILD_TYPE=Release
    cmake --build src\ostov_task\cmake-build-release --config Release
)

REM 6. Запускаем Streamlit
echo.
echo Запускаю веб-интерфейс (откроется в браузере через 5–10 секунд...)
streamlit run src\main.py --server.port=8501

pause
endlocal