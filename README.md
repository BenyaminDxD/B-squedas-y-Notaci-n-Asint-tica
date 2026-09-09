# SIS210 — Semana 2: Complejidad y búsqueda

## Contenido
- `practica.py`: implementación y benchmark en Python 3.
- `benchmark.cpp`: implementación y benchmark equivalente en C++20.
- `Online Retail(1).csv`: dataset utilizado.
- `datos_crudos_python.csv`: 30 mediciones por combinación en Python.
- `resumen_estadistico_python.csv`: resumen estadístico Python.
- `datos_crudos_cpp.csv`: resultados crudos de C++20.
- `resumen_estadistico_cpp.csv`: resumen C++20.
- `grafico_n_vs_comparaciones.png`: gráfico requerido.
- `grafico_n_vs_tiempo.png`: gráfico requerido.
- `informe_SIS210_Semana2.docx`: informe técnico.
- `README.md`: reproducción.

## Dataset
Online Retail de UCI. Se utiliza `CustomerID`, eliminando valores vacíos y conservando duplicados.
El CSV contiene 541,909 transacciones y 406,829 CustomerID no nulos.

## Diseño experimental
n = 100, 1,000, 10,000 y 100,000. La guía propone 500,000 si el dataset/equipo lo permite; en este archivo no es posible porque solo hay 406,829 CustomerID válidos.
Escenarios: inicio, centro, final y ausente. Se ejecutan 30 repeticiones por combinación.
La búsqueda binaria usa una copia ordenada y el tiempo de ordenamiento se registra por separado.

## Python
Requisitos:
- Python 3.11+
- pandas

Ejecutar:
```bash
python practica.py
```

## C++20
Compilar con GCC:
```bash
g++ -std=c++20 -O2 benchmark.cpp -o benchmark_cpp
./benchmark_cpp
```

## Reproducibilidad
Ejecutar cada lenguaje en la misma máquina, con el mismo CSV y sin contar la carga del archivo dentro de la región cronometrada. La región cronometrada contiene únicamente la búsqueda.

## Git
Commits sugeridos por la guía:
```bash
git init
git add . && git commit -m "baseline"
git add practica.py benchmark.cpp && git commit -m "analysis"
git add datos_crudos_python.csv resumen_estadistico_python.csv datos_crudos_cpp.csv resumen_estadistico_cpp.csv && git commit -m "benchmark"
git add . && git commit -m "final"
```

## Nota
El archivo recibido originalmente era Python; para completar el requisito de ambos lenguajes se añadió una implementación equivalente en C++20.
