#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

int busquedaLineal(const vector<long long>& a, long long x, long long& ops){
    ops=0;
    for(size_t i=0;i<a.size();++i){ ++ops; if(a[i]==x) return (int)i; }
    return -1;
}
int busquedaBinaria(const vector<long long>& a, long long x, long long& ops){
    int izq=0, der=(int)a.size()-1; ops=0;
    while(izq<=der){
        int medio=izq+(der-izq)/2; ++ops;
        if(a[medio]==x) return medio;
        if(a[medio]<x) izq=medio+1; else der=medio-1;
    }
    return -1;
}
long long now_ns(){ return chrono::duration_cast<chrono::nanoseconds>(
    chrono::steady_clock::now().time_since_epoch()).count(); }

vector<string> parseCSVLine(const string& line){
    vector<string> fields; string field; bool quoted=false;
    for(size_t i=0;i<line.size();++i){
        char c=line[i];
        if(c=='"'){
            if(quoted && i+1<line.size() && line[i+1]=='"'){ field+='"'; ++i; }
            else quoted=!quoted;
        } else if(c==',' && !quoted){ fields.push_back(field); field.clear(); }
        else field+=c;
    }
    fields.push_back(field);
    return fields;
}
vector<long long> cargarCustomerID(const string& archivo){
    ifstream f(archivo); string linea; vector<long long> datos;
    if(!f) throw runtime_error("No se pudo abrir el CSV.");
    getline(f,linea);
    while(getline(f,linea)){
        auto fields=parseCSVLine(linea);
        if(fields.size()>6 && !fields[6].empty()){
            try { datos.push_back(stoll(fields[6])); } catch(...) {}
        }
    }
    return datos;
}
int main(){
    const string csv="Online Retail(1).csv";
    auto datos=cargarCustomerID(csv);
    vector<int> tamanos={100,1000,10000,100000,500000};
    const int REP=30;
    ofstream raw("datos_crudos_cpp.csv"), sum("resumen_estadistico_cpp.csv");
    raw<<"n,escenario,objetivo,algoritmo,repeticion,posicion,comparaciones,tiempo_ns\n";
    sum<<"n,escenario,objetivo,algoritmo,mediana_comparaciones,mediana_tiempo_ns,tiempo_ordenamiento_ns\n";
    cout<<"CustomerID validos: "<<datos.size()<<"\n";
    for(int n:tamanos){
        if(n>(int)datos.size()){ cout<<"Omitiendo n="<<n<<" (datos insuficientes)\n"; continue; }
        vector<long long> muestra(datos.begin(),datos.begin()+n), ordenada=muestra;
        long long t0=now_ns(); sort(ordenada.begin(),ordenada.end()); long long sort_ns=now_ns()-t0;
        long long ausente=*max_element(datos.begin(),datos.end())+1;
        vector<pair<string,long long>> objs={{"inicio",muestra[0]},{"centro",muestra[n/2]},{"final",muestra[n-1]},{"ausente",ausente}};
        for(auto [esc,obj]:objs){
            for(int alg=0;alg<2;++alg){
                vector<long long> opss,times; opss.reserve(REP); times.reserve(REP);
                for(int r=1;r<=REP;++r){
                    long long ops; int pos; t0=now_ns();
                    if(alg==0) pos=busquedaLineal(muestra,obj,ops);
                    else pos=busquedaBinaria(ordenada,obj,ops);
                    long long dt=now_ns()-t0;
                    /* ops already counted */
                    opss.push_back(ops); times.push_back(dt);
                    raw<<n<<","<<esc<<","<<obj<<","<<(alg==0?"Lineal":"Binaria")<<","<<r<<","<<pos<<","<<ops<<","<<dt<<"\n";
                }
                sort(opss.begin(),opss.end()); sort(times.begin(),times.end());
                auto med=[](const vector<long long>& v){return v[v.size()/2];};
                sum<<n<<","<<esc<<","<<obj<<","<<(alg==0?"Lineal":"Binaria")<<","<<med(opss)<<","<<med(times)<<","<<(alg==1?sort_ns:0)<<"\n";
            }
        }
    }
}
