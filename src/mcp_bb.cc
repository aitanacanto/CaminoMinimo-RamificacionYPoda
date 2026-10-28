//AITANA CANTÓ RICO
#include <iostream>
#include <vector>
#include <limits>
#include <tuple>
#include <queue>
#include <utility> 
#include <fstream>
#include <iomanip>
using namespace std;

// Variables globales
const string usage = "Usage:\nmcp bb [ -p] [ --p2D] -f fichero entrada";
const string errFilename = "ERROR: missing filename.";
const string errUnknown = "ERROR: unknown option ";
const string errFile = "ERROR: can't open file: ";
const long infinito = numeric_limits<long>::max();
using Nodo = tuple<float, long, long, long>;


// Struct del mapa de coordenadas
struct MapCoord {
    vector<vector<int>> weight;
    int column;
    int row;
};

struct Data {
    vector<vector<long>> almacenTopSolu; // Almacen de mejor coste para cada celda
    vector<vector<bool>> noIter; // Matriz para diferenciar si la solucion proviene del Iterativo
    long bestSolu;    // Mejor solucion encontrada
    vector<int> movementsBestSolu; // Movimientos mejor solucion
    priority_queue<Nodo, vector<Nodo>, greater<Nodo>> cola; // Cola Prioridad
    vector<vector<int>> pessimisticPath; //Camino de la cota pesimista, desde la celda hasta el final del mapa
    vector<vector<int>> path2Cell; //Camino desde el inicio del mapa a la celda  
    vector<vector<long>> matrizPesimista; // Matriz de la cota pesimista
};

struct Stadistics{
    long visited_nodes = 0; 		// Nodos Visitados
    long explored_nodes = 0;     // Nodos explorados
    long visited_leaf_nodes = 0; // Nodos Hoja Visitados
    long no_promissing_discarded_nodes = 0;	// Nodos No prometedores
    long no_feasible_discarded_nodes = 0;    // Nodos No factibles
    long npromising_but_discarded = 0;      // Nodos prometedores descartados
    long nbest_solution_updated_from_leafs = 0;
    long nbest_solution_updated_from_pessimistic_bound = 0;
};

// FUNCIONES IMPLEMENTADAS
void mcp_bb(Data &d, const MapCoord &m, Stadistics &s);
long mcp_it(const MapCoord &, vector<vector<long>> &);

// FUNCIONES AUXILIARES
vector<vector<long>> cota_pesimista(vector<vector<int>> &, MapCoord);
void getPath(Data &, const MapCoord &, int, int);
void opP2D(const MapCoord &, const Data &);
void imprimirMatriz(const vector<vector<char>> &);
bool arguments(int, char *[], bool &, bool &, string &, MapCoord &);
bool readingFich(MapCoord &, string);
long smallest(long, long, long);

////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    MapCoord mapC;
    Data d;
    Stadistics stadistics;
    bool p2d, p;
    p2d = p = false;
    string fichName = "";

    if (arguments(argc, argv, p2d, p, fichName, mapC) && readingFich(mapC, fichName)) {
        // Inicializamos los datos
        d.noIter = vector<vector<bool>>(mapC.row, vector<bool>(mapC.column, false));
        d.path2Cell = vector<vector<int>>(mapC.row, vector<int>(mapC.column, 0));

        auto start = clock();
            d.matrizPesimista = cota_pesimista(d.pessimisticPath, mapC);
            d.bestSolu = mcp_it(mapC, d.almacenTopSolu);       
            mcp_bb(d, mapC, stadistics);  
        auto end = clock(); 

        cout << d.bestSolu << endl;
        cout << stadistics.visited_nodes << " "
             << stadistics.explored_nodes << " "
             << stadistics.visited_leaf_nodes << " "
             << stadistics.no_feasible_discarded_nodes << " "
             << stadistics.no_promissing_discarded_nodes << " "
             << stadistics.npromising_but_discarded << " "
	         << stadistics.nbest_solution_updated_from_leafs << " "
	         << stadistics.nbest_solution_updated_from_pessimistic_bound << " "<< endl;

        cout << 1000*(double)(end-start)/(double)CLOCKS_PER_SEC << endl;
        if (p2d){ opP2D(mapC, d);  }
        if (p){ 
            cout << "<";
            for(int mov: d.movementsBestSolu){  
                cout << mov;
            }
            cout << ">";
        }
    }
    return 0;
}


////////////////////////////////////////////////////////////////////

void mcp_bb(Data &d, const MapCoord &m, Stadistics &s){
    int lastRow = m.row -1;
    int lastCol = m.column -1;
    int rDesp = 0, 
        cDesp = 0, 
        rPes = 0, 
        cPes = 0;
    
    //Calculamos el menor elemento del mapa
    long smallest = infinito;
	for(int i = 0; i < m.row; i++){
		for(int j = 0; j<  m.column; j++){
			if(m.weight[i][j] < smallest){
				smallest = m.weight[i][j];
			}
		}
	}

    d.cola.emplace(0,0,0,0);
    
    while(!d.cola.empty()){

        Nodo current = d.cola.top();
        long sol = get<1>(current);
        int r = get<2>(current);
        int c = get<3>(current);
        d.cola.pop();

        if(c == lastCol && r == lastRow){ //Es hoja
            d.bestSolu = min(d.bestSolu, m.weight[r][c] + sol);
            if(sol < d.bestSolu){
                rPes=r;
                cPes=c;
                s.nbest_solution_updated_from_leafs++;
            }
            s.visited_leaf_nodes++;
        }
        else{
            sol = m.weight[r][c] + sol;
            vector<pair<int, int>> desp = {{-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}};
            for(unsigned z=0; z < desp.size(); z++){
                rDesp = desp[z].first + r;
                cDesp = desp[z].second + c;

                if(rDesp<=lastRow && rDesp>=0 && cDesp<=lastCol && cDesp>=0){ // Solucion Factible
                    if(d.matrizPesimista[rDesp][cDesp] + sol < d.bestSolu){
                        s.nbest_solution_updated_from_pessimistic_bound++;
                        rPes = rDesp; cPes = cDesp;
                        d.bestSolu = d.matrizPesimista[rPes][cPes] + sol;
                        d.path2Cell[rPes][cPes] = 1+z;
                    }

                    long cotaOptimista = 0;
                    if(rDesp > cDesp){  cotaOptimista = (lastCol - cDesp)* smallest; }
                    else{               cotaOptimista = (lastRow - rDesp)* smallest; }

                    if(cotaOptimista + sol < d.bestSolu){ // Nodo Prometedor
                        long calculated = sol + m.weight[rDesp][cDesp];
                        long stored = d.almacenTopSolu[rDesp][cDesp];
                        bool difIterREcur = (!d.noIter[rDesp][cDesp] || (d.noIter[rDesp][cDesp] && (calculated < stored)));

                        if(difIterREcur){
                            s.explored_nodes++;
                            d.noIter[rDesp][cDesp] = true;
                            d.path2Cell[rDesp][cDesp] = 1+z;
                            d.almacenTopSolu[rDesp][cDesp] = calculated;
                            d.cola.emplace(cotaOptimista + sol, sol, rDesp, cDesp);
                        }
                        else{   s.npromising_but_discarded++; }
                    }
                    else{   s.no_promissing_discarded_nodes++;    }
                }
                else{
                    s.no_feasible_discarded_nodes++;
                }
                s.visited_nodes++;
            }
        }
    }
    getPath(d,m,rPes, cPes);
}

void getPath(Data &d, const MapCoord &m, int rPes, int cPes){
    int lastRow = m.row -1;
    int lastCol = m.column -1;
    int r = rPes;
    int c = cPes;
    vector<pair<int, int>> desp = {{1,  0}, {1,  -1}, {0,  -1}, {-1,  -1}, {-1,  0}, {-1, 1}, {0, 1}, {1, 1}};    

    int k = 0;
    while(c != 0 || r != 0){
        k = d.path2Cell[r][c] - 1;
        r += desp[k].first;
        c += desp[k].second;
        d.movementsBestSolu.insert(d.movementsBestSolu.begin(), k+1);
    }

	while(rPes != lastRow  || cPes != lastCol){
		d.movementsBestSolu.push_back(d.pessimisticPath[rPes][cPes]);
		switch(d.pessimisticPath[rPes][cPes]){
			case 5:
				rPes++;
			break;
			case 4:
				rPes++;
				cPes++;
			break;
			case 3:
				cPes++;
			break;
		}
	}
}

vector<vector<long>> cota_pesimista(vector<vector<int>> &pessimisticPath, MapCoord m) {
    int lastRow = m.row - 1;
    int lastCol = m.column - 1;
    long r1, r2, r3;

    // Inicializar matrizPesimista a 0
    vector<vector<long>> matrizPesimista;
    for (int i = 0; i < m.row; ++i) {
        vector<long> row;
        for (int j = 0; j < m.column; ++j) {
            row.push_back(0);
        }
        matrizPesimista.push_back(row);
    }

    // Inicializar pessimisticPath a 0
    for (int i = 0; i < m.row; ++i) {
        vector<int> row;
        for (int j = 0; j < m.column; ++j) {
            row.push_back(0);
        }
        pessimisticPath.push_back(row);
    }

    // Inicializar la esquina inferior derecha
    pessimisticPath[lastRow][lastCol] = 0;
    matrizPesimista[lastRow][lastCol] = m.weight[lastRow][lastCol];

    // Inicializar última fila y última columna
    for (int r = lastRow - 1; r >= 0; r--) {
        pessimisticPath[r][lastCol] = 5;
        matrizPesimista[r][lastCol] = matrizPesimista[r + 1][lastCol] + m.weight[r][lastCol];
    }

    for (int c = lastCol - 1; c >= 0; c--) {
        pessimisticPath[lastRow][c] = 3;
        matrizPesimista[lastRow][c] = matrizPesimista[lastRow][c + 1] + m.weight[lastRow][c];
    }

    // Calcular valores para el resto de la matriz
    for (int r = lastRow - 1; r >= 0; r--) {
        for (int c = lastCol - 1; c >= 0; c--) {
            r1 = matrizPesimista[r + 1][c + 1];
            r2 = matrizPesimista[r][c + 1];
            r3 = matrizPesimista[r + 1][c];

            matrizPesimista[r][c] = m.weight[r][c] + smallest(r1, r2, r3);

            if (r3 == smallest(r3, r2, r1)) {
                pessimisticPath[r][c] = 5;
            } else if (r2 == smallest(r3, r2, r1)) {
                pessimisticPath[r][c] = 3;
            } else {    pessimisticPath[r][c] = 4;  }
        }
    }
    return matrizPesimista;
}



long mcp_it(const MapCoord &mapC, vector<vector<long>> &aux) {
    // CREAMOS VECTOR y lo inicializamos a 0
    aux = vector<vector<long>>(mapC.row, vector<long>(mapC.column, 0));

    // 1a FILA
    for (int j = 0; j < mapC.column; j++) {
        if (j == 0) {   aux[0][j] = mapC.weight[0][j];  } 
        else {          aux[0][j] = mapC.weight[0][j] + aux[0][j - 1];  }
    }

    // 1a COLUMNA
    for (int i = 1; i < mapC.row; i++) {
        aux[i][0] = mapC.weight[i][0] + aux[i - 1][0];
    }

    // RESTO MATRIX
    for (int i = 1; i < mapC.row; i++) {
        for (int j = 1; j < mapC.column; j++) {
            long arriba = aux[i - 1][j];
            long izquierda = aux[i][j - 1];
            long diagonal = aux[i - 1][j - 1];
            aux[i][j] = mapC.weight[i][j] + smallest(arriba, izquierda, diagonal);
        }
    }
    return aux[mapC.row - 1][mapC.column - 1];
}


void opP2D(const MapCoord &mapC, const Data &d) {
    vector<vector<char>> path(mapC.row, vector<char>(mapC.column, '.'));
    vector<pair<int, int>> des = {{-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}};
    long pathCost = 0;

    int r = 0; int c = 0;
    for (unsigned i = 0; i < d.movementsBestSolu.size(); i++) {
        path[r][c] = 'x';
        pathCost += mapC.weight[r][c];
        r += des[d.movementsBestSolu[i] - 1].first;
        c += des[d.movementsBestSolu[i] - 1].second;
    }
    path[r][c] = 'x';
    pathCost += mapC.weight[r][c];
    imprimirMatriz(path);
    cout << pathCost << endl;
}


////////////////////////////////////////////////////////////////////
long smallest(long r1, long r2, long r3) {
    long tmp;
    tmp = r1 <= r2 ? r1 : r2;
    return tmp <= r3 ? tmp : r3;
}

void imprimirMatriz(const vector<vector<char>> &matriz) {
    for (const auto &row : matriz) {
        for (char cell : row) {
            cout << cell;
        }
        cout << endl;
    }
}

bool arguments(int argc, char *argv[], bool &p2d, bool &p, string &fichName, MapCoord &mapC) {
    if (argc == 1) {
        cerr << usage << endl;
        return false;
    } else {
        for (int i = 1; i < argc; ++i) {
            string opt = argv[i];
            if (opt == "-p") {
                p = true;
            } else if (opt == "--p2D") {
                p2d = true;
            } else if (opt == "-f") {
                if (i + 1 < argc) {
                    fichName = argv[++i];
                } else {
                    cerr << errFilename << endl;
                    return false;
                }
            } else {
                cerr << errUnknown << opt << endl;
                return false;
            }
        }
        return true;
    }
}

bool readingFich(MapCoord &mapC, string fichName) {
    ifstream fich(fichName.c_str());
    if (!fich.is_open()) {
        cerr << errFile << fichName << endl;
        return false;
    } else {
        int num;
        fich >> mapC.row >> mapC.column;
        mapC.weight.resize(mapC.row, vector<int>(mapC.column));
        for (int i = 0; i < mapC.row; ++i) {
            for (int j = 0; j < mapC.column; ++j) {
                fich >> num;
                mapC.weight[i][j] = num;
            }
        }
        fich.close();
        return true;
    }
}
