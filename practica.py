import pandas as pd
from time import perf_counter_ns
import statistics


# ============================================================
# 1. CARGAR EL ARCHIVO CSV
# ============================================================

df = pd.read_csv("Online Retail.csv", encoding="latin1")

# Trabajamos con CustomerID
# Se eliminan solamente los valores vacíos.
# Los duplicados SE CONSERVAN.
datos = df["CustomerID"].dropna().tolist()

print("==============================================")
print("       EXPERIMENTO DE BÚSQUEDA")
print("==============================================")
print("Total de registros del CSV:", len(df))
print("CustomerID válidos:", len(datos))


# ============================================================
# 2. BÚSQUEDA LINEAL
# ============================================================

def busqueda_lineal(a, x):

    ops = 0

    for i, valor in enumerate(a):

        ops += 1

        if valor == x:
            return i, ops

    return -1, ops


# ============================================================
# 3. BÚSQUEDA BINARIA
# ============================================================

def busqueda_binaria(a, x):

    izq = 0
    der = len(a) - 1
    ops = 0

    while izq <= der:

        medio = izq + (der - izq) // 2

        ops += 1

        if a[medio] == x:
            return medio, ops

        if a[medio] < x:
            izq = medio + 1

        else:
            der = medio - 1

    return -1, ops


# ============================================================
# 4. TAMAÑOS DE LAS MUESTRAS
# ============================================================

tamanos = [
    100,
    1000,
    10000,
    100000
]

# La guía permite 500000 si el equipo/dataset lo permite.
# Nuestro CSV tiene 406829 CustomerID no nulos, por lo que
# no podemos construir n=500000 sin inventar datos.

repeticiones = 30


# ============================================================
# 5. LISTAS PARA GUARDAR RESULTADOS
# ============================================================

resultados = []

datos_crudos = []


# ============================================================
# 6. EXPERIMENTO
# ============================================================

for n in tamanos:

    print("\n")
    print("==============================================")
    print("TAMAÑO n =", n)
    print("==============================================")


    # --------------------------------------------------------
    # MISMA MUESTRA PARA LOS DOS ALGORITMOS
    # --------------------------------------------------------

    muestra = datos[:n]


    # --------------------------------------------------------
    # OBJETIVOS
    # --------------------------------------------------------

    objetivo_inicio = muestra[0]

    objetivo_centro = muestra[len(muestra) // 2]

    objetivo_final = muestra[-1]

    # Objetivo que no existe
    objetivo_ausente = max(datos) + 1


    objetivos = {

        "inicio": objetivo_inicio,

        "centro": objetivo_centro,

        "final": objetivo_final,

        "ausente": objetivo_ausente
    }


    # ========================================================
    # ORDENAMIENTO PARA BÚSQUEDA BINARIA
    # ========================================================

    inicio_ordenamiento = perf_counter_ns()

    muestra_ordenada = sorted(muestra)

    fin_ordenamiento = perf_counter_ns()

    tiempo_ordenamiento = (
        fin_ordenamiento - inicio_ordenamiento
    )


    print(
        "Tiempo de ordenamiento:",
        tiempo_ordenamiento,
        "ns"
    )


    # ========================================================
    # RECORRER LOS 4 ESCENARIOS
    # ========================================================

    for nombre_objetivo, objetivo in objetivos.items():

        print("\n----------------------------------------------")
        print("Objetivo:", nombre_objetivo)
        print("CustomerID:", objetivo)
        print("----------------------------------------------")


        # ====================================================
        # BÚSQUEDA LINEAL
        # ====================================================

        tiempos_lineal = []

        comparaciones_lineal = []


        for repeticion in range(1, repeticiones + 1):

            inicio = perf_counter_ns()

            posicion, ops = busqueda_lineal(
                muestra,
                objetivo
            )

            fin = perf_counter_ns()

            tiempo = fin - inicio


            # Guardar resultados de esta repetición
            tiempos_lineal.append(tiempo)

            comparaciones_lineal.append(ops)


            datos_crudos.append({

                "n": n,

                "escenario": nombre_objetivo,

                "objetivo": objetivo,

                "algoritmo": "Lineal",

                "repeticion": repeticion,

                "posicion": posicion,

                "comparaciones": ops,

                "tiempo_ns": tiempo
            })


        # ====================================================
        # ESTADÍSTICAS BÚSQUEDA LINEAL
        # ====================================================

        promedio_tiempo = statistics.mean(
            tiempos_lineal
        )

        mediana_tiempo = statistics.median(
            tiempos_lineal
        )

        minimo_tiempo = min(
            tiempos_lineal
        )

        maximo_tiempo = max(
            tiempos_lineal
        )


        promedio_comparaciones = statistics.mean(
            comparaciones_lineal
        )

        mediana_comparaciones = statistics.median(
            comparaciones_lineal
        )

        minimo_comparaciones = min(
            comparaciones_lineal
        )

        maximo_comparaciones = max(
            comparaciones_lineal
        )


        resultados.append({

            "n": n,

            "escenario": nombre_objetivo,

            "objetivo": objetivo,

            "algoritmo": "Lineal",

            "promedio_tiempo_ns": promedio_tiempo,

            "mediana_tiempo_ns": mediana_tiempo,

            "minimo_tiempo_ns": minimo_tiempo,

            "maximo_tiempo_ns": maximo_tiempo,

            "promedio_comparaciones":
                promedio_comparaciones,

            "mediana_comparaciones":
                mediana_comparaciones,

            "minimo_comparaciones":
                minimo_comparaciones,

            "maximo_comparaciones":
                maximo_comparaciones,

            "tiempo_ordenamiento_ns": 0
        })


        # ====================================================
        # BÚSQUEDA BINARIA
        # ====================================================

        tiempos_binaria = []

        comparaciones_binaria = []


        for repeticion in range(1, repeticiones + 1):

            inicio = perf_counter_ns()

            posicion, ops = busqueda_binaria(
                muestra_ordenada,
                objetivo
            )

            fin = perf_counter_ns()

            tiempo = fin - inicio


            tiempos_binaria.append(tiempo)

            comparaciones_binaria.append(ops)


            datos_crudos.append({

                "n": n,

                "escenario": nombre_objetivo,

                "objetivo": objetivo,

                "algoritmo": "Binaria",

                "repeticion": repeticion,

                "posicion": posicion,

                "comparaciones": ops,

                "tiempo_ns": tiempo
            })


        # ====================================================
        # ESTADÍSTICAS BÚSQUEDA BINARIA
        # ====================================================

        promedio_tiempo = statistics.mean(
            tiempos_binaria
        )

        mediana_tiempo = statistics.median(
            tiempos_binaria
        )

        minimo_tiempo = min(
            tiempos_binaria
        )

        maximo_tiempo = max(
            tiempos_binaria
        )


        promedio_comparaciones = statistics.mean(
            comparaciones_binaria
        )

        mediana_comparaciones = statistics.median(
            comparaciones_binaria
        )

        minimo_comparaciones = min(
            comparaciones_binaria
        )

        maximo_comparaciones = max(
            comparaciones_binaria
        )


        resultados.append({

            "n": n,

            "escenario": nombre_objetivo,

            "objetivo": objetivo,

            "algoritmo": "Binaria",

            "promedio_tiempo_ns": promedio_tiempo,

            "mediana_tiempo_ns": mediana_tiempo,

            "minimo_tiempo_ns": minimo_tiempo,

            "maximo_tiempo_ns": maximo_tiempo,

            "promedio_comparaciones":
                promedio_comparaciones,

            "mediana_comparaciones":
                mediana_comparaciones,

            "minimo_comparaciones":
                minimo_comparaciones,

            "maximo_comparaciones":
                maximo_comparaciones,

            "tiempo_ordenamiento_ns":
                tiempo_ordenamiento
        })


        # ====================================================
        # MOSTRAR RESULTADOS EN PANTALLA
        # ====================================================

        print("\nLINEAL")

        print(
            "Promedio comparaciones:",
            promedio_comparaciones
        )

        print(
            "Mediana comparaciones:",
            mediana_comparaciones
        )

        print(
            "Promedio tiempo:",
            promedio_tiempo,
            "ns"
        )

        print(
            "Mediana tiempo:",
            mediana_tiempo,
            "ns"
        )


        print("\nBINARIA")

        print(
            "Promedio comparaciones:",
            promedio_comparaciones
        )

        print(
            "Mediana comparaciones:",
            mediana_comparaciones
        )

        print(
            "Promedio tiempo:",
            promedio_tiempo,
            "ns"
        )

        print(
            "Mediana tiempo:",
            mediana_tiempo,
            "ns"
        )


# ============================================================
# 7. CREAR DATAFRAME CON LOS RESULTADOS
# ============================================================

df_resultados = pd.DataFrame(resultados)

df_crudos = pd.DataFrame(datos_crudos)


# ============================================================
# 8. GUARDAR DATOS CRUDOS
# ============================================================

df_crudos.to_csv(
    "datos_crudos.csv",
    index=False
)


# ============================================================
# 9. GUARDAR RESUMEN ESTADÍSTICO
# ============================================================

df_resultados.to_csv(
    "resumen_estadistico.csv",
    index=False
)


# ============================================================
# 10. MOSTRAR TABLA FINAL
# ============================================================

print("\n\n")
print("==============================================")
print("           RESUMEN FINAL")
print("==============================================")


print(
    df_resultados.to_string(index=False)
)


print("\n")
print("==============================================")
print("ARCHIVOS GENERADOS")
print("==============================================")

print("1. datos_crudos.csv")
print("2. resumen_estadistico.csv")

print("\nExperimento terminado correctamente.")