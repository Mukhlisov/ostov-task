import streamlit as st
import subprocess
import json
import tempfile
import os
import networkx as nx
import matplotlib.pyplot as plt
import platform
import os

def getPathToBinaryFile():
    bin_dir = os.path.join("src", "ostov_task", "cmake-build-release")
    bin_name = "ostov_task"
    if platform.system() == "Windows":
        bin_name += ".exe"
    bin_path = os.path.join(bin_dir, bin_name)

    if not os.path.exists(bin_path):
        raise RuntimeError(f"Бинарник не найден: {bin_path}. Запусти сборку")
    return bin_path;
            

st.title("Поиск остова с максимальным количеством висячих вершин")

vertices_input = st.text_input("Вершины (через запятую, напр. 0,1,2,3,4)")
edges_input = st.text_area("Рёбра (каждое в формате 0-1, по одному на строку)")

if st.button("Запустить"):
    if vertices_input and edges_input:
        try:
            vert_list = [int(v.strip()) for v in vertices_input.split(",")]
            edge_list = []
            for line in edges_input.splitlines():
                if line.strip():
                    parts = line.strip().split("-")
                    edge_list.append([int(parts[0].strip()), int(parts[1].strip())])
        except ValueError:
            st.error("Неверный формат вершин или рёбер")
            st.stop()

        st.subheader("Входной граф")
        G_input = nx.Graph()
        G_input.add_nodes_from(vert_list)
        G_input.add_edges_from(edge_list)
        fig_input, ax_input = plt.subplots()
        nx.draw(G_input, with_labels=True, ax=ax_input)
        st.pyplot(fig_input)

        graph_data = {"vertices": vert_list, "edges": edge_list}
        with tempfile.NamedTemporaryFile(mode='w', delete=False, dir='.', suffix='.json') as tmp:
            json.dump(graph_data, tmp)
            input_file = tmp.name

        try:
            bin_path = getPathToBinaryFile()
            result = subprocess.run([bin_path, input_file], capture_output=True, text=True, check=True)
            output_json = json.loads(result.stdout)
        except subprocess.CalledProcessError as e:
            st.error(f"Ошибка запуска бинарника: {e.stderr}")
            os.remove(input_file)
            st.stop()
        except json.JSONDecodeError:
            st.error("Неверный JSON из бинарника")
            os.remove(input_file)
            st.stop()
        except RuntimeError as e:
            st.error(str(e))
            os.remove(input_file)
            st.stop()

        os.remove(input_file)

        status = output_json.get("status")
        value = output_json.get("value")
        graph = output_json.get("graph", [])

        if status == "success":
            st.success(f"Максимум висячих вершин: {value}")

            st.subheader("Выходной остов")
            G_output = nx.Graph()
            G_output.add_nodes_from(vert_list)
            G_output.add_edges_from(graph)
            fig_output, ax_output = plt.subplots()
            nx.draw(G_output, with_labels=True, ax=ax_output)
            st.pyplot(fig_output)
        elif status == "error":
            st.error(f"Ошибка: {value}")
        else:
            st.error("Неверный статус в JSON")
    else:
        st.error("Введите вершины и рёбра")