// ============================================================
// EXPERIMENTO DE BUSQUEDA - Version C++
// Traduccion directa del script en Python (pandas -> lectura
// manual de CSV, statistics -> funciones propias).
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <limits>
#include <iomanip>
#include <map>

using namespace std;
using namespace std::chrono;

// ============================================================
// 1. UTILIDADES DE LECTURA DE CSV
// ============================================================

// Separa una linea de CSV respetando comillas dobles (algunos
// campos, como "Description", contienen comas dentro de comillas).
static vector<string> parsearLineaCSV(const string& linea) {
    vector<string> campos;
    string actual;
    bool dentroComillas = false;

    for (size_t i = 0; i < linea.size(); i++) {
        char c = linea[i];

        if (c == '"') {
            dentroComillas = !dentroComillas;
        } else if (c == ',' && !dentroComillas) {
            campos.push_back(actual);
            actual.clear();
        } else {
            actual += c;
        }
    }
    campos.push_back(actual);

    return campos;
}

// Quita espacios y retorno de carro al final de un string
static string limpiar(const string& s) {
    string r = s;
    while (!r.empty() && (r.back() == '\r' || r.back() == '\n' || r.back() == ' ')) {
        r.pop_back();
    }
    return r;
}

// ============================================================
// 2. CARGAR EL ARCHIVO CSV (columna CustomerID)
// ============================================================

vector<double> cargarCustomerID(const string& rutaArchivo, long long& totalRegistros) {
    ifstream archivo(rutaArchivo);

    if (!archivo.is_open()) {
        cerr << "No se pudo abrir el archivo: " << rutaArchivo << endl;
        exit(1);
    }

    string linea;
    getline(archivo, linea);
    vector<string> encabezado = parsearLineaCSV(linea);

    int indiceCustomerID = -1;
    for (size_t i = 0; i < encabezado.size(); i++) {
        if (limpiar(encabezado[i]) == "CustomerID") {
            indiceCustomerID = static_cast<int>(i);
            break;
        }
    }

    if (indiceCustomerID == -1) {
        cerr << "No se encontro la columna CustomerID" << endl;
        exit(1);
    }

    vector<double> datos;
    totalRegistros = 0;

    while (getline(archivo, linea)) {
        if (linea.empty()) continue;

        totalRegistros++;

        vector<string> campos = parsearLineaCSV(linea);
        if (indiceCustomerID >= static_cast<int>(campos.size())) continue;

        string valor = limpiar(campos[indiceCustomerID]);

        // Se eliminan solamente los valores vacios.
        // Los duplicados SE CONSERVAN.
        if (valor.empty()) continue;

        try {
            double id = stod(valor);
            datos.push_back(id);
        } catch (...) {
            // valor no numerico, se ignora
            continue;
        }
    }

    return datos;
}

// ============================================================
// 3. BUSQUEDA LINEAL
// ============================================================

pair<long long, long long> busquedaLineal(const vector<double>& a, double x) {
    long long ops = 0;

    for (size_t i = 0; i < a.size(); i++) {
        ops++;

        if (a[i] == x) {
            return {static_cast<long long>(i), ops};
        }
    }

    return {-1, ops};
}

// ============================================================
// 4. BUSQUEDA BINARIA
// ============================================================

pair<long long, long long> busquedaBinaria(const vector<double>& a, double x) {
    long long izq = 0;
    long long der = static_cast<long long>(a.size()) - 1;
    long long ops = 0;

    while (izq <= der) {
        long long medio = izq + (der - izq) / 2;

        ops++;

        if (a[medio] == x) {
            return {medio, ops};
        }

        if (a[medio] < x) {
            izq = medio + 1;
        } else {
            der = medio - 1;
        }
    }

    return {-1, ops};
}

// ============================================================
// 5. ESTADISTICAS (equivalente a statistics.mean/median/min/max)
// ============================================================

double media(const vector<long long>& v) {
    double suma = 0;
    for (long long x : v) suma += static_cast<double>(x);
    return suma / static_cast<double>(v.size());
}

double mediana(vector<long long> v) {
    sort(v.begin(), v.end());
    size_t n = v.size();

    if (n % 2 == 1) {
        return static_cast<double>(v[n / 2]);
    } else {
        return (static_cast<double>(v[n / 2 - 1]) + static_cast<double>(v[n / 2])) / 2.0;
    }
}

long long minimo(const vector<long long>& v) {
    return *min_element(v.begin(), v.end());
}

long long maximo(const vector<long long>& v) {
    return *max_element(v.begin(), v.end());
}

// ============================================================
// 6. ESTRUCTURAS PARA GUARDAR RESULTADOS
// ============================================================

struct FilaCruda {
    long long n;
    string escenario;
    double objetivo;
    string algoritmo;
    int repeticion;
    long long posicion;
    long long comparaciones;
    long long tiempoNs;
};

struct FilaResumen {
    long long n;
    string escenario;
    double objetivo;
    string algoritmo;
    double promedioTiempoNs;
    double medianaTiempoNs;
    long long minimoTiempoNs;
    long long maximoTiempoNs;
    double promedioComparaciones;
    double medianaComparaciones;
    long long minimoComparaciones;
    long long maximoComparaciones;
    long long tiempoOrdenamientoNs;
};

// ============================================================
// 7. PROGRAMA PRINCIPAL
// ============================================================

int main() {
    cout << "==============================================" << endl;
    cout << "       EXPERIMENTO DE BUSQUEDA" << endl;
    cout << "==============================================" << endl;

    long long totalRegistros = 0;
    vector<double> datos = cargarCustomerID("Online Retail.csv", totalRegistros);

    cout << "Total de registros del CSV: " << totalRegistros << endl;
    cout << "CustomerID validos: " << datos.size() << endl;

    // Tamanos de las muestras
    vector<long long> tamanos = {100, 1000, 10000, 100000};

    // La guia permite 500000 si el equipo/dataset lo permite.
    // El CSV no tiene suficientes CustomerID no nulos para
    // construir n=500000 sin inventar datos.

    const int repeticiones = 30;

    vector<FilaResumen> resultados;
    vector<FilaCruda> datosCrudos;

    double maximoGlobal = *max_element(datos.begin(), datos.end());

    for (long long n : tamanos) {
        cout << "\n\n==============================================" << endl;
        cout << "TAMANO n = " << n << endl;
        cout << "==============================================" << endl;

        // Misma muestra para los dos algoritmos
        vector<double> muestra(datos.begin(), datos.begin() + n);

        // Objetivos
        double objetivoInicio = muestra.front();
        double objetivoCentro = muestra[muestra.size() / 2];
        double objetivoFinal = muestra.back();
        double objetivoAusente = maximoGlobal + 1;

        vector<pair<string, double>> objetivos = {
            {"inicio", objetivoInicio},
            {"centro", objetivoCentro},
            {"final", objetivoFinal},
            {"ausente", objetivoAusente}
        };

        // Ordenamiento para busqueda binaria
        auto inicioOrdenamiento = high_resolution_clock::now();
        vector<double> muestraOrdenada = muestra;
        sort(muestraOrdenada.begin(), muestraOrdenada.end());
        auto finOrdenamiento = high_resolution_clock::now();

        long long tiempoOrdenamiento = duration_cast<nanoseconds>(finOrdenamiento - inicioOrdenamiento).count();

        cout << "Tiempo de ordenamiento: " << tiempoOrdenamiento << " ns" << endl;

        for (auto& objetivoPar : objetivos) {
            const string& nombreObjetivo = objetivoPar.first;
            double objetivo = objetivoPar.second;

            cout << "\n----------------------------------------------" << endl;
            cout << "Objetivo: " << nombreObjetivo << endl;
            cout << "CustomerID: " << objetivo << endl;
            cout << "----------------------------------------------" << endl;

            // ==================== BUSQUEDA LINEAL ====================
            vector<long long> tiemposLineal;
            vector<long long> comparacionesLineal;

            for (int repeticion = 1; repeticion <= repeticiones; repeticion++) {
                auto inicio = high_resolution_clock::now();
                pair<long long, long long> resultado = busquedaLineal(muestra, objetivo);
                auto fin = high_resolution_clock::now();

                long long tiempo = duration_cast<nanoseconds>(fin - inicio).count();

                tiemposLineal.push_back(tiempo);
                comparacionesLineal.push_back(resultado.second);

                datosCrudos.push_back({
                    n, nombreObjetivo, objetivo, "Lineal", repeticion,
                    resultado.first, resultado.second, tiempo
                });
            }

            double promedioTiempoLineal = media(tiemposLineal);
            double medianaTiempoLineal = mediana(tiemposLineal);
            long long minimoTiempoLineal = minimo(tiemposLineal);
            long long maximoTiempoLineal = maximo(tiemposLineal);

            double promedioComparacionesLineal = media(comparacionesLineal);
            double medianaComparacionesLineal = mediana(comparacionesLineal);
            long long minimoComparacionesLineal = minimo(comparacionesLineal);
            long long maximoComparacionesLineal = maximo(comparacionesLineal);

            resultados.push_back({
                n, nombreObjetivo, objetivo, "Lineal",
                promedioTiempoLineal, medianaTiempoLineal, minimoTiempoLineal, maximoTiempoLineal,
                promedioComparacionesLineal, medianaComparacionesLineal,
                minimoComparacionesLineal, maximoComparacionesLineal,
                0
            });

            // ==================== BUSQUEDA BINARIA ====================
            vector<long long> tiemposBinaria;
            vector<long long> comparacionesBinaria;

            for (int repeticion = 1; repeticion <= repeticiones; repeticion++) {
                auto inicio = high_resolution_clock::now();
                pair<long long, long long> resultado = busquedaBinaria(muestraOrdenada, objetivo);
                auto fin = high_resolution_clock::now();

                long long tiempo = duration_cast<nanoseconds>(fin - inicio).count();

                tiemposBinaria.push_back(tiempo);
                comparacionesBinaria.push_back(resultado.second);

                datosCrudos.push_back({
                    n, nombreObjetivo, objetivo, "Binaria", repeticion,
                    resultado.first, resultado.second, tiempo
                });
            }

            double promedioTiempoBinaria = media(tiemposBinaria);
            double medianaTiempoBinaria = mediana(tiemposBinaria);
            long long minimoTiempoBinaria = minimo(tiemposBinaria);
            long long maximoTiempoBinaria = maximo(tiemposBinaria);

            double promedioComparacionesBinaria = media(comparacionesBinaria);
            double medianaComparacionesBinaria = mediana(comparacionesBinaria);
            long long minimoComparacionesBinaria = minimo(comparacionesBinaria);
            long long maximoComparacionesBinaria = maximo(comparacionesBinaria);

            resultados.push_back({
                n, nombreObjetivo, objetivo, "Binaria",
                promedioTiempoBinaria, medianaTiempoBinaria, minimoTiempoBinaria, maximoTiempoBinaria,
                promedioComparacionesBinaria, medianaComparacionesBinaria,
                minimoComparacionesBinaria, maximoComparacionesBinaria,
                tiempoOrdenamiento
            });

            // Mostrar resultados en pantalla (misma logica que el script original)
            cout << "\nLINEAL" << endl;
            cout << "Promedio comparaciones: " << promedioComparacionesLineal << endl;
            cout << "Mediana comparaciones: " << medianaComparacionesLineal << endl;
            cout << "Promedio tiempo: " << promedioTiempoLineal << " ns" << endl;
            cout << "Mediana tiempo: " << medianaTiempoLineal << " ns" << endl;

            cout << "\nBINARIA" << endl;
            cout << "Promedio comparaciones: " << promedioComparacionesBinaria << endl;
            cout << "Mediana comparaciones: " << medianaComparacionesBinaria << endl;
            cout << "Promedio tiempo: " << promedioTiempoBinaria << " ns" << endl;
            cout << "Mediana tiempo: " << medianaTiempoBinaria << " ns" << endl;
        }
    }

    // ============================================================
    // GUARDAR DATOS CRUDOS
    // ============================================================

    ofstream archivoCrudos("datos_crudos.csv");
    archivoCrudos << "n,escenario,objetivo,algoritmo,repeticion,posicion,comparaciones,tiempo_ns\n";

    for (const FilaCruda& f : datosCrudos) {
        archivoCrudos << f.n << "," << f.escenario << "," << fixed << setprecision(1) << f.objetivo << ","
                      << f.algoritmo << "," << f.repeticion << "," << f.posicion << ","
                      << f.comparaciones << "," << f.tiempoNs << "\n";
    }
    archivoCrudos.close();

    // ============================================================
    // GUARDAR RESUMEN ESTADISTICO
    // ============================================================

    ofstream archivoResumen("resumen_estadistico.csv");
    archivoResumen << "n,escenario,objetivo,algoritmo,promedio_tiempo_ns,mediana_tiempo_ns,"
                   << "minimo_tiempo_ns,maximo_tiempo_ns,promedio_comparaciones,mediana_comparaciones,"
                   << "minimo_comparaciones,maximo_comparaciones,tiempo_ordenamiento_ns\n";

    for (const FilaResumen& f : resultados) {
        archivoResumen << f.n << "," << f.escenario << "," << fixed << setprecision(1) << f.objetivo << ","
                       << f.algoritmo << "," << f.promedioTiempoNs << "," << f.medianaTiempoNs << ","
                       << f.minimoTiempoNs << "," << f.maximoTiempoNs << "," << f.promedioComparaciones << ","
                       << f.medianaComparaciones << "," << f.minimoComparaciones << "," << f.maximoComparaciones
                       << "," << f.tiempoOrdenamientoNs << "\n";
    }
    archivoResumen.close();

    // ============================================================
    // MOSTRAR TABLA FINAL
    // ============================================================

    cout << "\n\n==============================================" << endl;
    cout << "           RESUMEN FINAL" << endl;
    cout << "==============================================" << endl;

    cout << left
         << setw(8) << "n"
         << setw(10) << "escenario"
         << setw(12) << "algoritmo"
         << setw(16) << "prom_tiempo_ns"
         << setw(16) << "prom_compar."
         << endl;

    for (const FilaResumen& f : resultados) {
        cout << left
             << setw(8) << f.n
             << setw(10) << f.escenario
             << setw(12) << f.algoritmo
             << setw(16) << f.promedioTiempoNs
             << setw(16) << f.promedioComparaciones
             << endl;
    }

    cout << "\n==============================================" << endl;
    cout << "ARCHIVOS GENERADOS" << endl;
    cout << "==============================================" << endl;
    cout << "1. datos_crudos.csv" << endl;
    cout << "2. resumen_estadistico.csv" << endl;

    cout << "\nExperimento terminado correctamente." << endl;

    return 0;
}
