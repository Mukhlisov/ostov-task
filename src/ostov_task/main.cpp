#include <bitset>
#include <cstdint>
#include <iostream>
#include <array>
#include <fstream>
#include <stack>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <nlohmann/json.hpp>

unsigned getLeafVerticesNumber(const std::bitset<64> &chosenEdges,
                               const std::vector<std::array<int, 2> > &adjacentVertices,
                               const std::unordered_map<int, std::vector<int> > &incidentEdges);

std::tuple<unsigned, std::bitset<64> > tryFindOstovWithMaxLeafs(
    const std::vector<std::array<int, 2> > &adjacentVertices,
    const std::unordered_map<int, std::vector<int> > &incidentEdges);

/// Вектор смежных вершин (рёбер)
/// HashMap с вершинами в качестве ключей и вектором индексов в качестве значений
/// Вектор индексов, хранит индексы указывающие на ребра под соответствующим индексом из adjacentVertices
std::tuple<std::vector<std::array<int, 2> >, std::unordered_map<int, std::vector<int> > > readGraphFromFile(
    const char* filePath);

std::vector<std::vector<int> > buildGraph(std::bitset<64> mask, const std::vector<std::array<int, 2> > &initialGraph);

int main(const int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path/to/file>" << std::endl;
        return EXIT_FAILURE;
    }

    const auto [adjacentVertices, incidentEdges] = readGraphFromFile(argv[1]);
    const auto [maxLeafs, maxLeafsMask] = tryFindOstovWithMaxLeafs(adjacentVertices, incidentEdges);

    const auto resultGraph = buildGraph(maxLeafsMask, adjacentVertices);

    nlohmann::json outputJson;
    if (maxLeafs > 0) {
        outputJson["status"] = "success";
        outputJson["value"] = maxLeafs;
        outputJson["graph"] = resultGraph;
    } else {
        outputJson["status"] = "error";
        outputJson["value"] = "Не найдено подходящих подграфов";
    }

    std::cout << outputJson.dump() << std::endl;

    return 0;
}

int getOppositeVertex(const std::array<int, 2> &edge, const int &currentVertex) {
    return edge[0] == currentVertex ? edge[1] : edge[0];
}

/// Так как вершины учитывваются два раза предполагаемая степень висячей вершины = 2
unsigned updateLeafAmount(const std::array<int, 2> edge, std::vector<int> &verticesDegree, const unsigned &leafAmount) {
    int leafDelta = 0;
    for (const auto vertex: edge) {
        verticesDegree[vertex]++;

        if (verticesDegree[vertex] == 2)
            leafDelta++;
        else if (verticesDegree[vertex] == 4)
            leafDelta--;
    }

    return leafAmount + leafDelta;
}

/// Поиск всех висячих вершин
/// Параллельно идет проверка графа на связность при помощи DFS
/// vertices - 1 == edges и DFS => остов
unsigned getLeafVerticesNumber(const std::bitset<64> &chosenEdges,
                               const std::vector<std::array<int, 2> > &adjacentVertices,
                               const std::unordered_map<int, std::vector<int> > &incidentEdges) {
    std::stack<int> verticesToVisit;
    std::unordered_set<int> visitedVertices;

    const unsigned verticesNumber = incidentEdges.size();

    std::vector<int> verticesDegree(verticesNumber, 0);
    unsigned edgesAmount = 0;
    unsigned leafAmount = 0;

    verticesToVisit.push(0);
    visitedVertices.insert(0);

    while (!verticesToVisit.empty()) {
        const int currentVertex = verticesToVisit.top();
        verticesToVisit.pop();

        for (const auto edgeIndex: incidentEdges.at(currentVertex)) {
            if (chosenEdges[edgeIndex] == 0) continue;

            edgesAmount++;
            const auto &edge = adjacentVertices[edgeIndex];
            leafAmount = updateLeafAmount(edge, verticesDegree, leafAmount);

            const int oppositeVertex = getOppositeVertex(edge, currentVertex);

            if (visitedVertices.contains(oppositeVertex)) continue;
            verticesToVisit.push(oppositeVertex);
            visitedVertices.insert(oppositeVertex);
        }
    }

    edgesAmount /= 2; // При проходе ребра учитывались дважды
    if (edgesAmount != verticesNumber - 1 || incidentEdges.size() != visitedVertices.size())
        return 0;

    return leafAmount;
}

std::tuple<unsigned, std::bitset<64> > tryFindOstovWithMaxLeafs(
    const std::vector<std::array<int, 2> > &adjacentVertices,
    const std::unordered_map<int, std::vector<int> > &incidentEdges) {
    
    const uint64_t range = 1ULL << adjacentVertices.size();
    unsigned maxLeafs = 0;
    std::bitset<64> maxLeafsMask{};

    #pragma omp parallel for shared(adjacentVertices, incidentEdges, maxLeafs, maxLeafsMask, range) default(none)
    for (long long i = 1; i < (long long)range; i++) {
        const std::bitset<64> mask(i);
        const unsigned leafVerticesNumber = getLeafVerticesNumber(mask, adjacentVertices, incidentEdges);

        #pragma omp critical
        {
            if (leafVerticesNumber > maxLeafs) {
                maxLeafs = leafVerticesNumber;
                maxLeafsMask = mask;
            }
        }
    }

    return std::make_tuple(maxLeafs, maxLeafsMask);
}

void insertVertex(std::unordered_map<int, std::vector<int>> &incidentEdges, const int vertex, const int edgeNumber) {
    if (!incidentEdges.contains(vertex))
        incidentEdges[vertex] = {};
    incidentEdges[vertex].push_back(edgeNumber);
}

std::tuple<std::vector<std::array<int, 2>>, std::unordered_map<int, std::vector<int>>> readGraphFromFile(
    const char* filePath) {
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open input file" << std::endl;
        throw std::runtime_error("Failed to open input file");
    }

    nlohmann::json graphData;
    try {
        inputFile >> graphData;
    } catch ([[maybe_unused]] const std::exception &e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        throw std::runtime_error("Failed to parse JSON");
    }

    std::vector<std::array<int, 2>> adjacentVertices{};
    std::unordered_map<int, std::vector<int>> incidentEdges{};
    if (graphData.contains("edges")) {
        int edgeNumber = 0;
        for (const auto& edge : graphData["edges"]) {
            if (edge.size() == 2) {
                adjacentVertices.push_back({edge[0], edge[1]});
                insertVertex(incidentEdges, edge[0], edgeNumber);
                insertVertex(incidentEdges, edge[1], edgeNumber);
                edgeNumber++;
            }
        }
    } else {
        std::cerr << "No edges in JSON" << std::endl;
        throw std::runtime_error("No edges in JSON");
    }

    return std::make_tuple(adjacentVertices, incidentEdges);
}

std::vector<std::vector<int> > buildGraph(const std::bitset<64> mask,
                                            const std::vector<std::array<int, 2> > &initialGraph) {
    std::vector<std::vector<int> > resultGraph{};
    for (int i = 0; i < initialGraph.size(); i++) {
        if (mask[i] == 1)
            resultGraph.push_back({initialGraph[i][0], initialGraph[i][1]});
    }

    return resultGraph;
}
