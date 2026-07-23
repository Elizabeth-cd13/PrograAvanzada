# Proyecto Final: Simpletron


**Simpletron** es una computadora virtual diseñada para entender cómo funciona un procesador por dentro bajo la arquitectura de Von Neumann. A lo largo del curso de Programación Avanzada, trabajar en este proyecto nos ayudó a aterrizar los conceptos teóricos llevándolos a la práctica en tres áreas clave:

* **El funcionamiento del procesador:** Entender en carne propia cómo el simulador ejecuta el ciclo de instrucción (*Fetch, Decode, Execute*) y cómo se van actualizando los registros y la memoria en tiempo real.
* **De alto a bajo nivel:** Comprender todo el viaje que realiza el código: desde cómo escribimos un programa en lenguaje *SIMPLE* (con `if`, `let` y variables) hasta cómo el compilador usa estructuras de datos como pilas y tablas de símbolos para traducirlo a código de máquina *SML*.
* **Control de memoria y depuración:** Aprender la importancia de validar rangos de memoria, atrapar errores como la división entre cero y usar el *Memory Dump* como herramienta principal para depurar.

##  Información General
* **Materia:** Programación Avanzada
* **Proyecto:** Compilador y Simulador Simpletron (SML / SIMPLE)
* **Repositorio GitHub:** [https://github.com/Elizabeth-cd13/PrograAvanzada](https://github.com/Elizabeth-cd13/PrograAvanzada)
* **Integrante:** Elizabeth González Mendoza

---
# Control de versiones

Durante el desarrollo se utilizó Git y GitHub para:

- Crear ramas locales.
- Crear ramas remotas.
- Realizar commits.
- Fusionar ramas.

## Descripción General del Proyecto
Este proyecto integra la implementación completa de una **computadora virtual Simpletron (Simulador SML M8)** y un **compilador de dos pasadas** para el lenguaje de alto nivel **SIMPLE**.

El compilador traduce programas escritos en lenguaje *SIMPLE* a código de máquina *SML (Simpletron Machine Language)* con formato `OOAAA`. Posteriormente, el programa generado (`programa.simp`) es cargado y ejecutado por el simulador Simpletron, el cual incluye soporte para punto flotante, manejo de cadenas de texto y operaciones aritméticas extendidas.

---
# Requisitos

- GCC
- Visual Studio Code
- Git
- GitHub

## ️ Estructura del proyecto

```

README.md

docs/
    requerimientos.md
    diseno.md

src/
    simpletron/
    expresiones/
    compilador/

programas/
    simple/
    sml/

pruebas/
    entradas/
    salidas/

evidencias/
    ejecuciones/
    git-branch/
```
    
### Descripción Módulos del Sistema

#### 1. Módulo del Compilador (`compilador_simple_definitivo.c`)
Es el componente encargado de realizar la traducción del código fuente en lenguaje *SIMPLE* a lenguaje de máquina *SML*.
* **Analizador Léxico y Sintáctico:** Procesa cada línea del programa comprobando el orden ascendente de las líneas, la presencia de palabras clave minúsculas (`input`, `print`, `let`, `goto`, `if`, `end`) y la declaración de variables de una sola letra minúscula.
* **Tabla de Símbolos:** Mantiene el registro de cada símbolo encontrado categorizándolo por Tipo:
  * `'L'`: Líneas de código del programa y sus direcciones de memoria equivalentes.
  * `'V'`: Variables declaradas, asignándoles posiciones desde la parte superior de la memoria (`999`, `998`, etc.).
  * `'C'`: Constantes numéricas utilizadas.
* **Primera Pasada (`firstPass`):** Genera las instrucciones *SML* iniciales. Para saltos incondicionales (`goto`) o condicionales (`if`), si la línea destino aún no se ha procesado, coloca una marca/bandera (`flag`) para marcar la referencia como no resuelta.
* **Segunda Pasada (`secondPass`):** Recorre el arreglo de banderas para resolver los operandos pendientes, sustituyendo las referencias futuras por las direcciones de memoria reales calculadas durante la primera pasada.

#### 2. Módulo de Evaluación de Expresiones Aritméticas
Integrado dentro del compilador para procesar los enunciados `let`:
* **Verificación de Agrupación:** Valida que los paréntesis `()`, corchetes `[]` y llaves `{}` estén correctamente balanceados.
* **Conversión Infija a Postfija (*Shunting-yard*):** Transforma expresiones matemáticas tradicionales (infijas) a notación polaca inversa (postfija), respetando la jerarquía de operadores: Potencia (`^`) > Multiplicación/División/Módulo (`*`, `/`, `%`) > Suma/Resta (`+`, `-`).
* **Generación de Código Aritmético:** Evalúa la expresión postfija utilizando memoria temporal para cargar operandos (`LOAD`), efectuar operaciones (`ADD`, `SUBTRACT`, `MULTIPLY`, `DIVIDE`, `MOD`, `POWER`) y guardar resultados parciales (`STORE`).

#### 3. Módulo del Simulador Simpletron (`simulador.c` - Versión M8)
Procesador virtual completo que ejecuta el código SML generado:
* **Estructura de Memoria:** Memoria principal extendida de 1000 posiciones (`double memory[1000]`) que admite valores de punto flotante.
* **Registros de Procesador:**
  * `accumulator`: Registro donde se realizan las operaciones aritmético-lógicas.
  * `instructionCounter`: Apuntador de dirección de la siguiente instrucción a ejecutar.
  * `instructionRegister`: Almacena la instrucción actual completa.
  * `operationCode`: Extrae los dos primeros dígitos que representan la operación.
  * `operand`: Extrae los tres últimos dígitos que representan la dirección de memoria destino.
* **Ciclo de Instrucción:** Ejecuta secuencialmente las fases de Búsqueda (*Fetch*), Decodificación (*Decode*), Ejecución (*Execute*) y Actualización de Contador (*Update IC*).
* **Manejo de Excepciones y Vaciado de Memoria:** Detecta errores en tiempo de ejecución (división/módulo entre cero, desbordamiento del acumulador, instrucciones no enteras u opcodes inválidos) e imprime automáticamente un vaciado completo de registros y memoria (*Memory Dump*).

---

## Instrucciones de Compilación

Asegúrate de contar con el compilador `gcc` instalado. Abre tu terminal o línea de comandos y ejecuta:

### 1. Compilar el Compilador de SIMPLE:
```bash
gcc  -o compilador compilador_simple_definitivo.c### 1. Compilar el Compilador de SIMPLE:
```

### 2. Compilar el Simulador Simpletron M8 (Mejora 8):

```bash
gcc  -o simulador simulador.c -lm
```
## Instrucciones de Ejecución
Para procesar y ejecutar cualquier programa:

#### Paso 1: Traducir el código fuente de SIMPLE a SML
Ejecuta el programa compilador indicando el archivo de entrada y el archivo de salida en SML, ejemplo:
```bash
./compilador programas/simple/programa_prueba.simple programa.simp
```
#### Paso 2: Cargar y Ejecutar en el Simulador
Asegúrate de que el archivo programa.simp generado se encuentre en el mismo directorio donde vas a correr el simulador:
```bash
./simulador
```
## Especificación del Lenguaje SIMPLE

El compilador acepta programas estructurados con las siguientes reglas sintácticas:

* **Números de línea:** Cada línea debe iniciar obligatoriamente con un número entero en orden estrictamente ascendente.
* **Variables:** Se representan obligatoriamente con una sola letra minúscula (`a` - `z`)[cite: 1].
* **Sintaxis:** Todas las palabras clave deben estar escritas en minúsculas.
* **Expresiones:** Admite expresiones aritméticas con operadores `+`, `-`, `*`, `/`, `%` (módulo), `^` (potencia) y agrupadores `()`.
* **Fin de programa:** Todo programa finaliza obligatoriamente con el comando `end`.

### Comandos Soportados:

| Comando | Sintaxis | Descripción |
| :--- | :--- | :--- |
| `rem` | `linea rem [texto]` | Comentario ignorado por el compilador. |
| `input` | `linea input var` | Solicita un valor al usuario y lo almacena en `var`. |
| `print` | `linea print var` | Imprime en pantalla el valor contenido en `var`. |
| `let` | `linea let var = expr` | Evalúa una expresión matemática e introduce el resultado en `var`. |
| `goto` | `linea goto linea_dest` | Salto incondicional hacia otra línea. |
| `if` | `linea if op1 rel op2 goto linea_dest` | Salto condicional con operadores relacionales (`==`, `!=`, `<`, `<=`, `>`, `>=`). |
| `end` | `linea end` | Marca la terminación del programa (genera `HALT`). |

## Detalles del Simulador y Repertorio SML (Formato OOAAA)

El simulador Simpletron procesa instrucciones con un formato de 5 dígitos, donde los 2 primeros dígitos corresponden al **Código de Operación (OO)** y los 3 últimos al **Operando o Dirección de Memoria (AAA)**.

### Categorías de Instrucciones:

* **Entrada / Salida:**
  * `10 READ` — Lee un valor numérico ingresado por el usuario y lo almacena en memoria.
  * `11 WRITE` — Imprime en pantalla el valor almacenado en la posición de memoria.
  * `12 READSTRING` — Lee una cadena de caracteres desde la entrada estándar.
  * `13 WRITESTRING` — Muestra en pantalla una cadena de caracteres guardada en memoria.
  * `14 NEWLINE` — Imprime un salto de línea en la consola.

* **Carga y Almacenamiento:**
  * `20 LOAD` — Carga en el Acumulador el valor contenido en una posición de memoria.
  * `21 STORE` — Guarda el valor del Acumulador en una posición de memoria específica.

* **Operaciones Aritméticas:**
  * `30 ADD` — Suma el valor de la memoria al valor del Acumulador.
  * `31 SUBTRACT` — Resta el valor de la memoria al valor del Acumulador.
  * `32 DIVIDE` — Divide el Acumulador entre el valor de la posición de memoria.
  * `33 MULTIPLY` — Multiplica el valor del Acumulador por el valor de la memoria.
  * `34 MOD` — Calcula el residuo de la división del Acumulador entre la memoria.
  * `35 POWER` — Eleva el valor del Acumulador a la potencia especificada por la memoria.

* **Control de Flujo (Saltos):**
  * `40 BRANCH` — Salto incondicional hacia una posición de memoria determinada.
  * `41 BRANCHNEG` — Salto hacia una posición de memoria si el Acumulador es negativo (`< 0`).
  * `42 BRANCHZERO` — Salto hacia una posición de memoria si el Acumulador es cero (`== 0`).

* **Terminación:**
  * `43 HALT` — Detiene inmediatamente la ejecución de la computadora virtual.


---

# Archivos principales

| Archivo | Descripción |
|----------|-------------|
|Simpletron.c|Simulador Simpletron|
|Integracion.c|Evaluación de expresiones|
|Compilador.c|Compilador de Simple a SML|
|README.md|Documentación principal|

---
