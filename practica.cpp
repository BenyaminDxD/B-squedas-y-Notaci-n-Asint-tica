// ============================================================
// EXPERIMENTO DE BUSQUEDA - Version simplificada en C++
// Escenario 7: Indice de clientes de comercio electronico
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;

// ---------- Busquedas ----------

int busquedaLineal(const vector<long long>& a, long long x, long long& ops) {
    ops = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        ++ops;
        if (a[i] == x) return static_cast<int>(i);
    }
    return -1;
}

int busquedaBinaria(const vector<long long>& a, long long x, long long& ops) {
    int izq = 0, der = static_cast<int>(a.size()) - 1;
    ops = 0;
    while (izq <= der) {
        int medio = izq + (der - izq) / 2;
        ++ops;
        if (a[medio] == x) return medio;
        if (a[medio] < x) izq = medio + 1;
        else der = medio - 1;
    }
    return -1;
}

// ---------- Utilidades de lectura de CSV ----------

vector<string> parsearLinea(const string& linea) {
    vector<string> campos;
    string actual;
    bool dentroComillas = false;
    for (size_t i = 0; i < linea.size(); i++) {
        char c = linea[i];
        if (c == '"') dentroComillas = !dentroComillas;
        else if (c == ',' && !dentroComillas) { campos.push_back(actual); actual.clear(); }
        else actual += c;
    }
    campos.push_back(actual);
    return campos;
}

string limpiar(string s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ')) s.pop_back();
    return s;
}

vector<long long> cargarCustomerID(const string& ruta, long long& totalRegistros) {
    ifstream archivo(ruta);
    if (!archivo.is_open()) { cerr << "No se pudo abrir: " << ruta << endl; exit(1); }

    string linea;
    getline(archivo, linea);
    vector<string> encabezado = parsearLinea(linea);

    int col = -1;
    for (size_t i = 0; i < encabezado.size(); i++) {
        if (limpiar(encabezado[i]) == "CustomerID") { col = (int)i; break; }
    }
    if (col == -1) { cerr << "No se encontro columna CustomerID" << endl; exit(1); }

    vector<long long> datos;
    totalRegistros = 0;
    while (getline(archivo, linea)) {
        if (linea.empty()) continue;
        totalRegistros++;
        vector<string> campos = parsearLinea(linea);
        if (col >= (int)campos.size()) continue;
        string valor = limpiar(campos[col]);
        if (valor.empty()) continue;
        try {
            long long id = static_cast<long long>(stod(valor)); // "17850.0" -> 17850
            datos.push_back(id);
        } catch (...) { continue; }
    }
    return datos;
}

// ---------- Estadisticas ----------

double media(const vector<long long>& v) {
    double s = 0;
    for (size_t i = 0; i < v.size(); i++) s += (double)v[i];
    return s / v.size();
}

double mediana(vector<long long> v) {
    sort(v.begin(), v.end());
    size_t n = v.size();
    if (n % 2 == 1) return (double)v[n / 2];
    return ((double)v[n / 2 - 1] + (double)v[n / 2]) / 2.0;
}

// ---------- Corre N repeticiones de un algoritmo ----------

struct Medicion {
    vector<long long> tiempos;
    vector<long long> comparaciones;
};

Medicion correrRepeticiones(int algoritmo, const vector<long long>& datos, long long objetivo, int repeticiones) {
    // algoritmo: 0 = lineal, 1 = binaria
    Medicion m;
    for (int r = 0; r < repeticiones; r++) {
        long long ops = 0;
        high_resolution_clock::time_point t0 = high_resolution_clock::now();
        if (algoritmo == 0) busquedaLineal(datos, objetivo, ops);
        else                busquedaBinaria(datos, objetivo, ops);
        high_resolution_clock::time_point t1 = high_resolution_clock::now();

        m.tiempos.push_back(duration_cast<nanoseconds>(t1 - t0).count());
        m.comparaciones.push_back(ops);
    }
    return m;
}

// ---------- Programa principal ----------

int main() {
    cout << "EXPERIMENTO DE BUSQUEDA (lineal vs binaria)\n";

    long long totalRegistros = 0;
    vector<long long> datosCompletos = cargarCustomerID("Online Retail.csv", totalRegistros);
    cout << "Registros totales: " << totalRegistros << " | CustomerID validos: " << datosCompletos.size() << "\n";

    vector<long long> tamanos;
    tamanos.push_back(100);
    tamanos.push_back(1000);
    tamanos.push_back(10000);
    tamanos.push_back(100000);
    tamanos.push_back(500000);

    const int repeticiones = 30;

    long long maximoGlobal = *max_element(datosCompletos.begin(), datosCompletos.end());

    ofstream crudos("datos_crudos.csv");
    crudos << "n,escenario,objetivo,algoritmo,tiempo_ns,comparaciones\n";

    ofstream resumen("resumen_estadistico.csv");
    resumen << "n,escenario,objetivo,algoritmo,prom_tiempo_ns,mediana_tiempo_ns,min_tiempo_ns,max_tiempo_ns,"
            << "prom_comparaciones,mediana_comparaciones,min_comparaciones,max_comparaciones,tiempo_ordenamiento_ns\n";

    for (size_t idxN = 0; idxN < tamanos.size(); idxN++) {
        long long n = tamanos[idxN];

        if (n > (long long)datosCompletos.size()) {
            cout << "\nn = " << n << " omitido: no hay suficientes CustomerID validos.\n";
            continue;
        }

        cout << "\n--- n = " << n << " ---\n";
        vector<long long> muestra(datosCompletos.begin(), datosCompletos.begin() + n);

        // Version ordenada para la busqueda binaria (tiempo medido aparte)
        high_resolution_clock::time_point ti = high_resolution_clock::now();
        vector<long long> ordenada = muestra;
        sort(ordenada.begin(), ordenada.end());
        high_resolution_clock::time_point tf = high_resolution_clock::now();
        long long tiempoOrdenamiento = duration_cast<nanoseconds>(tf - ti).count();
        cout << "Tiempo de ordenamiento: " << tiempoOrdenamiento << " ns\n";

        vector<pair<string, long long> > objetivos;
        objetivos.push_back(make_pair(string("inicio"), muestra.front()));
        objetivos.push_back(make_pair(string("centro"), muestra[muestra.size() / 2]));
        objetivos.push_back(make_pair(string("final"), muestra.back()));
        objetivos.push_back(make_pair(string("ausente"), maximoGlobal + 1));

        for (size_t idxObj = 0; idxObj < objetivos.size(); idxObj++) {
            string nombre = objetivos[idxObj].first;
            long long objetivo = objetivos[idxObj].second;

            for (int alg = 0; alg <= 1; alg++) {
                string nombreAlg = (alg == 0) ? "Lineal" : "Binaria";
                const vector<long long>& base = (alg == 0) ? muestra : ordenada;

                Medicion med = correrRepeticiones(alg, base, objetivo, repeticiones);

                for (int r = 0; r < repeticiones; r++) {
                    crudos << n << "," << nombre << "," << objetivo << "," << nombreAlg << ","
                           << med.tiempos[r] << "," << med.comparaciones[r] << "\n";
                }

                resumen << n << "," << nombre << "," << objetivo << "," << nombreAlg << ","
                        << media(med.tiempos) << "," << mediana(med.tiempos) << ","
                        << *min_element(med.tiempos.begin(), med.tiempos.end()) << ","
                        << *max_element(med.tiempos.begin(), med.tiempos.end()) << ","
                        << media(med.comparaciones) << "," << mediana(med.comparaciones) << ","
                        << *min_element(med.comparaciones.begin(), med.comparaciones.end()) << ","
                        << *max_element(med.comparaciones.begin(), med.comparaciones.end()) << ","
                        << (alg == 1 ? tiempoOrdenamiento : 0) << "\n";

                cout << nombreAlg << " (" << nombre << "): prom_comp=" << media(med.comparaciones)
                     << " prom_tiempo_ns=" << media(med.tiempos) << "\n";
            }
        }
    }

    crudos.close();
    resumen.close();

    cout << "\nArchivos generados: datos_crudos.csv, resumen_estadistico.csv\n";
    return 0;
}
