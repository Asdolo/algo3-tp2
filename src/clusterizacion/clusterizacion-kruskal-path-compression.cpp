#include <vector>
#include <algorithm>
#include <math.h>
#include <iostream>
#include <tuple>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include "../include/aux.hpp"

using namespace std;

struct subset
{
    int parent;
    int rank;
};
bool noPertenece(vector<tuple<int, int, int>> res, int x, int y)
{
    for (uint i = 0; i < res.size(); i++)
    {
        if ((get<0>(res[i]) == x && get<1>(res[i]) == y) || (get<0>(res[i]) == y && get<1>(res[i]) == x))
            return false;
    }
    return true;
}

bool sortbyth(const tuple<int, int, int> &a,
              const tuple<int, int, int> &b)
{
    return (get<2>(a) < get<2>(b));
}

int FIND(int u, vector<subset> &subsets)
{
    if (subsets[u].parent != u)
        subsets[u].parent = FIND(subsets[u].parent, subsets);

    return subsets[u].parent;
}
void UNION(int u, int v, vector<subset> &subsets)
{
    int xroot = FIND(u, subsets);
    int yroot = FIND(v, subsets);

    if (subsets[xroot].rank < subsets[yroot].rank)
    {
        subsets[xroot].parent = yroot;
    }
    else if (subsets[xroot].rank > subsets[yroot].rank)
    {
        subsets[yroot].parent = xroot;
    }
    else
    {
        subsets[yroot].parent = xroot;
        subsets[xroot].rank++;
    }
}
vector<vector<double>> kruskalPathCompression(vector<vector<double>> E)
{
    // Creo los structs subset para union-find con path compression
    vector<subset> subsets;
    // Inicializo los subsets
    for (uint i = 0; i < E[0].size(); i++)
    {
        struct subset s;
        s.parent = i;
        s.rank = 0;
        subsets.push_back(s);
    }

    vector<tuple<int, int, int>> agm;
    vector<tuple<int, int, int>> listaDeAristas = obtenerListaDeAristas(E);
    sort(listaDeAristas.begin(), listaDeAristas.end(), sortbyth);

    for (uint i = 0; i < listaDeAristas.size(); i++)
    {
        tuple<int, int, int> arista = listaDeAristas[i];
        if (FIND(get<0>(arista), subsets) != FIND(get<1>(arista), subsets))
        {
            agm.push_back(arista);
            UNION(get<0>(arista), get<1>(arista), subsets);
        }
    }
    //Ya tengo el AGM en res. Lo paso a matriz de adyacencia
    vector<vector<double>> res(E[0].size(), vector<double>(E[0].size(), -1));
    for (uint i = 0; i < agm.size(); i++)
    {
        tuple<int, int, int> arista = agm[i];
        res[get<0>(arista)][get<1>(arista)] = get<2>(arista);
        res[get<1>(arista)][get<0>(arista)] = get<2>(arista);
    }
    return res;
}

/*
 Argumentos:
    1. Archivo .csv donde se va a guardar cada punto y a qué clúster pertenece.
    2. Archivo .csv donde se va a agregar una línea que indique la cantidad de puntos,
        los parámetros del test (vecindad, version, excesoNecesarioDesvioEstandar y ratioExceso),
        la cantidad de clústers resultantes y el tiempo de ejecución del algoritmo.
    3. Tamaño de la vencindad a tener en cuenta para definir si un eje es inconsistente.
    4. Versión de noción de eje inconsistente a utilizar (valor entre 1 y 2)
    5. Cantidad de veces que debe exceder el peso de un eje con respecto al desvió estándar
        de su vecindad para considerarse inconsistente (versión 1).
    6. Ratio que debe exceder un eje para considerarse inconsistente (versión 2).
*/
int main(int argc, char *argv[])
{
    int vecindad = argc >= 4 ? stoi(argv[3]) : 2;
    int version = argc >= 5 ? stoi(argv[4]) : 1;
    double excesoNecesarioDesvioEstandar = argc >= 6 ? stod(argv[5]) : 3;
    double ratioExceso = argc >= 7 ? stod(argv[6]) : 2;
    
    int cantPuntos = 0;
    cin >> cantPuntos;

    if (cantPuntos < 0)
    {
        cerr << "La cantidad de puntos debe ser no negativa." << endl;
        return -1;
    }

    ofstream archivoTablaClusters;
    ofstream archivoResultados;
    archivoTablaClusters.open(argc >= 2 ? argv[1] : "output/clusterizacion/clusters-kruskal-path-compression.csv", ios_base::app);
    archivoResultados.open(argc >= 3 ? argv[2] : "output/clusterizacion/mediciones-kruskal-path-compression.csv", ios_base::app);

    vector<tuple<int, int>> coordenadas(cantPuntos);

    // Leemos las coordenadas de cada punto por stdin
    for (int i = 0; i < cantPuntos; i++)
    {
        int x, y;
        cin >> x;
        cin >> y;

        coordenadas[i] = make_tuple(x, y);
    }

    // Modelaremos un grafo completo donde cada nodo será un punto del plano,
    // y el peso de cada arista entre cada par de nodos será la distancia en el plano
    // de los puntos que son representados por dichos nodos
    // Representamos en grafo con una matriz de adyacencia
    vector<vector<double>> E = pointsToGraph(coordenadas);

    // Ejecutamos kruskal con path compression, que nos devuelve una matriz de adyacencia que representa un AGM
    // del árbol que le pasamos.
    vector<vector<double>> agm;
    auto startTime = chrono::steady_clock::now();
    agm = kruskalPathCompression(E);
    auto endTime = chrono::steady_clock::now();
    
    tuple<vector<int>, int> res;
    
    auto startTime2 = chrono::steady_clock::now();
    res = obtenerClusters(agm, vecindad, version, excesoNecesarioDesvioEstandar, ratioExceso);//O(N^3)
    auto endTime2 = chrono::steady_clock::now();

    for (uint i = 0; i < get<0>(res).size(); i++)
    {
        cout << get<0>(res)[i] << endl;
    }

    // Guardamos las salidas en los archivos .csv
    for (int i = 0; i < cantPuntos; i++)
    {
        archivoTablaClusters << i << "," << get<0>(coordenadas[i]) << "," << get<1>(coordenadas[i]) << "," << get<0>(res)[i] << endl;
    }
    archivoTablaClusters.close();

    archivoResultados << cantPuntos << "," << vecindad << "," << version << "," << excesoNecesarioDesvioEstandar << "," << ratioExceso << "," << get<1>(res) << "," << chrono::duration<double, milli>((endTime - startTime) + (endTime2 - startTime2)).count() << endl;
    archivoResultados.close();

    return 0;
}