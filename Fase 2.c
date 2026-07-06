#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define TAM 200

int operador(char c);
int operando(char c);
int abre(char c);
int cierra(char c);
int mismoGrupo(char a, char b);
int prioridad(char op);
int comparar(char pila, char actual);
void convertirPostfijo(char cadena[]);

int operador(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

int operando(char c) {
    return isalnum(c);
}

int abre(char c) {
    return c == '(' || c == '[' || c == '{';
}

int cierra(char c) {
    return c == ')' || c == ']' || c == '}';
}

int mismoGrupo(char a, char b) {
    return (a == '(' && b == ')') ||
           (a == '[' && b == ']') ||
           (a == '{' && b == '}');
}

int prioridad(char op) {

    switch(op){

        case '(':
        case '[':
        case '{':
            return 1;

        case '+':
        case '-':
            return 2;

        case '*':
        case '/':
            return 3;

        case '^':
            return 4;

        default:
            return 0;
    }
}

int comparar(char pila, char actual) {

    if (abre(pila))
        return 0;

    if (cierra(actual))
        return 1;

    return prioridad(pila) >= prioridad(actual);
}

void convertirPostfijo(char cadena[]) {

    char stack[TAM];
    char salida[TAM];

    int cima = -1;
    int leer = 0;
    int escribir = 0;

    char actual;
    char elemento;

    while (cadena[leer] != '\0') {

        actual = cadena[leer];

        if (actual == ' ' || actual == '\n') {
            leer++;
            continue;
        }

        if (operando(actual)) {

            salida[escribir++] = actual;
        }

        else if (abre(actual)) {

            stack[++cima] = actual;
        }

        else if (operador(actual) || cierra(actual)) {

            while (cima != -1 && comparar(stack[cima], actual)) {

                elemento = stack[cima--];

                if (!abre(elemento) && !cierra(elemento)) {
                    salida[escribir++] = elemento;
                }
            }

            if (cierra(actual)) {

                if (cima != -1 && mismoGrupo(stack[cima], actual)) {
                    cima--;
                }
                else {
                    printf("Error: agrupadores incorrectos.\n");
                    return;
                }
            }
            else {

                stack[++cima] = actual;
            }
        }

        leer++;
    }

    while (cima != -1) {

        elemento = stack[cima--];

        if (abre(elemento) || cierra(elemento)) {

            printf("Error: agrupadores incorrectos.\n");
            return;
        }

        salida[escribir++] = elemento;
    }

    salida[escribir] = '\0';

    printf("\nExpresion postfija: %s\n", salida);
}

int main() {

    char entrada[TAM];

    while (1) {

        printf("\nIngrese una expresion(o escriba salir): ");
        fgets(entrada, TAM, stdin);

        entrada[strcspn(entrada, "\n")] = '\0';

        if (strcmp(entrada, "salir") == 0) {
            printf("Programa terminado.\n");
            break;
        }

        convertirPostfijo(entrada);
    }

    return 0;
}
