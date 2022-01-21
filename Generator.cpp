#include <iostream>
#include <atomic>
#include <thread>
#include <cstdlib>
#include <random>
#include <cmath>
#include <cstring>
#include <unordered_set>
#include <string>
#include <memory>
#include <fstream>

using namespace std;

double P[2][2] = {{0.57, 0.19}, {0.19, 0.05}};
double U[2] = {0, 0};
double V[2][2];
void initialize()
{
    for (auto i = 0; i < 2; i++)
        for (auto j = 0; j < 2; j++)
            U[i] += P[i][j];
    for (auto i = 0; i < 2; i++)
        for (auto j = 0; j < 2; j++)
            V[i][j] = P[i][j] / U[i];
}
void genExpectDegree(shared_ptr<double[]> expectDegree, int vertices, long long totalEdges)
{
    expectDegree[0] = U[0];
    expectDegree[1] = U[1];
    int range = 2;
    while (range < vertices)
    {
        range *= 2;
        for (int i = range - 1; i >= 0; i--)
            expectDegree[i] = expectDegree[i / 2] * U[i % 2];
    }
    for (auto i = 0; i < vertices; i++)
        expectDegree[i] *= totalEdges;
}
void genRealDegree(shared_ptr<double[]> expectDegree, shared_ptr<int[]> degree, default_random_engine &eng, int vertices, long long &totalEdges)
{
    long long restDegree = 0;
    for (int i = 0; i < vertices; i++)
    {
        poisson_distribution<int> d(expectDegree[i]);
        degree[i] = d(eng);
        if (degree[i] > vertices)
        {
            restDegree += degree[i] - vertices;
            degree[i] = vertices;
        }
    }
    while (restDegree > 0)
    {
        double expectRestedges = 0;
        for (int i = 0; i < vertices; i++)
            if (degree[i] < vertices)
                expectRestedges += expectDegree[i];
        for (int i = 0; i < vertices; i++)
            if (degree[i] < vertices)
            {
                poisson_distribution<int> d(expectDegree[i] / expectRestedges);
                int addDegree = d(eng);
                if (degree[i] + addDegree > vertices)
                    addDegree = vertices - degree[i];
                restDegree -= addDegree;
                degree[i] += addDegree;
            }
    }
    totalEdges = 0;
    for (int i = 0; i < vertices; i++)
        totalEdges += degree[i];
}

void genNeighborsPossibility(shared_ptr<double[]> neighborProbability, int vertices, double V[])
{
    neighborProbability[0] = V[0];
    neighborProbability[1] = V[1];
    int range = 2;
    while (range < vertices)
    {
        range *= 2;
        for (int i = range - 1; i >= 0; i--)
            neighborProbability[i] = neighborProbability[i / 2] * V[i % 2];
    }
}
void genNeighbors(shared_ptr<int[]> edges, shared_ptr<double[]> neighborProbability, int vertices, int degree, long long &k, shared_ptr<int[]> maps, default_random_engine &eng, int thisVertex)
{
    for (auto i = 0; i < vertices; i++)
        maps[i] = i;
    int length = vertices;
    while (degree > 0)
    {
        discrete_distribution<> d{neighborProbability.get(), neighborProbability.get() + length};
        auto previousDegree = degree;
        for (auto i = 0; i < previousDegree; i++)
        {
            int neighbor = d(eng);
            if (maps[neighbor] > 0)
            {
                edges[k] = thisVertex;
                k++;
                edges[k] = maps[neighbor];
                k++;
                maps[neighbor] = -1;
                degree--;
            }
        }
        auto j = 0;
        for (auto i = 0; i < length; i++)
            if (maps[i] > 0)
            {
                maps[j] = maps[i];
                neighborProbability[j] = neighborProbability[i];
                j++;
            }
        length = j;
    }
}

void genSparseNeighbors(shared_ptr<int[]> edges, int degree, default_random_engine &eng, int logVertices, double V[], long long &k, int thisVertex)
{
    discrete_distribution<> d{V, V + 2};
    unordered_set<int> set;
    for (; degree > 0;)
    {
        int neighbor = 0;
        for (auto j = 0, y = 1, x = 0; j < logVertices; j++, y *= 2)
        {
            x = d(eng);
            neighbor += y * x;
        }
        if (set.find(neighbor) == set.end())
        {
            set.insert(neighbor);
            edges[k] = thisVertex;
            k++;
            edges[k] = neighbor;
            k++;
            degree--;
        }
    }
}

void genEdges(shared_ptr<int[]> edges, shared_ptr<int[]> degree, long long totalEdges, int vertices, int logVertices, default_random_engine &eng)
{
    long long k = 0;
    shared_ptr<double[]> neighborProbability(new double[vertices]);
    shared_ptr<int[]> maps(new int[vertices]);

    for (int i = 0; i < vertices; i++)
    {
        if (degree[i] == vertices)
        {
            for (int j = 0; j < vertices; j++)
            {
                edges[k] = i;
                k++;
                edges[k] = j;
                k++;
            }
        }
        else if (degree[i] > vertices / logVertices)
        {
            memset(neighborProbability.get(), 0, sizeof(double) * vertices);
            genNeighborsPossibility(neighborProbability, vertices, V[i % 2]);
            genNeighbors(edges, neighborProbability, vertices, degree[i], k, maps, eng, i);
        }
        else
        {
            genSparseNeighbors(edges, degree[i], eng, logVertices, V[i % 2], k, i);
        }
    }
}

int main(int argc, char *argv[])
{
    initialize();
    string fileName = argv[1];
    int logVertices = atoi(argv[2]);
    int vertices = pow(2, logVertices);
    int averageDegree = atoi(argv[3]);
    long long totalEdges = averageDegree * (long long)vertices;
    long long seed = 0;
    random_device r;
    seed = r();
    int threadNum = 1;
    if (argc > 4)
        threadNum = atoi(argv[4]);
    if (argc > 5)
        seed = atol(argv[5]);
    default_random_engine eng(seed);
    shared_ptr<int[]> degree(new int[vertices]);
    shared_ptr<double[]> expectDegree(new double[vertices]);
    genExpectDegree(expectDegree, vertices, totalEdges);
    genRealDegree(expectDegree, degree, eng, vertices, totalEdges);
    cout << "here" << endl;
    shared_ptr<int[]> edges(new int[totalEdges * 2]);
    genEdges(edges, degree, totalEdges, vertices, logVertices, eng);
    cout << totalEdges << endl;
    fstream outFile(fileName + "edgelist", ios::out | ios::binary);
    outFile.write((char *)edges.get(), sizeof(int) * totalEdges * 2);
    fstream propertiesFile(fileName + "properties1.txt", ios::out);
    propertiesFile << vertices << " " << totalEdges << endl;
    // array<int, 3> a = {1, 2, 10};
    // discrete_distribution<> d{a.begin(), a.end()};
    // int a[] = {1, 2, 10};
    // discrete_distribution<> d{a, a + 3};
    // for (int i = 0; i < 100; i++)
    //     cout << d(eng) << endl;
}