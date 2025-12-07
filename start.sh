#!/bin/bash
set -e

cd "$(dirname "$0")"

echo -e "\n=================================================="
echo -e "   Остов с максимальным числом висячих вершин"
echo -e "==================================================\n"

if [! command -v python3 &> /dev/null ];
then
    echo "Python3 не найден, установите его"
    exit 1
fi

if [ ! -f "venv/bin/python" ];
then
    echo "Создаю виртуальное окружение..."
    python3 -m venv venv
fi

source venv/bin/activate

echo "Устанавливаю пакеты Python (может занять 10–30 секунд...)"
pip install -r requirements.txt

echo -e "Запускаю веб-интерфейс (открывать браузере через 5–10 секунд...)"
streamlit run src/main.py --server.port=8501