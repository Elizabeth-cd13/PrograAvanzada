#include <stdio.h>
#include <math.h>

#define MAX 100

/* -----PILA ----- */

typedef struct
{
    float datos[MAX];
    int tope;
} Pila;

void iniciar(Pila *p)
{
    p->tope = -1;
}

int vacia(Pila *p)
{
    return p->tope == -1;
}

void push(Pila *p, float valor)
{
    if (p->tope < MAX - 1)
    {
        p->datos[++(p->tope)] = valor;
    }
}

float pop(Pila *p)
{
    if (!vacia(p))
    {
        return p->datos[(p->tope)--];
    }

    return 0;
}

/* -------FUNCIONES------- */

/* Convierte un caracter numerico a float */
float convierte(char numero)
{
    return (float)(numero - '0');
}

/* Evalua una operacion */
float evalua(float opnd1, char sign, float opnd2)
{
    switch(sign)
    {
        case '+':
            return opnd1 + opnd2;

        case '-':
            return opnd1 - opnd2;

        case '*':
            return opnd1 * opnd2;

        case '/':
            return opnd1 / opnd2;

        case '^':
            return pow(opnd1, opnd2);

        default:
            return 0;
    }
}

/* Evaluacion postfija */
float evaluar(char expresion[])
{
    Pila pila;
    iniciar(&pila);

    int i = 0;
    char symb;

    float opnd1;
    float opnd2;
    float value;

    while(expresion[i] != '\0')
    {
        symb = expresion[i];

        if(symb >= '0' && symb <= '9')
        {
            push(&pila, convierte(symb));
        }
        else
        {
            opnd2 = pop(&pila);
            opnd1 = pop(&pila);

            value = evalua(opnd1, symb, opnd2);

            push(&pila, value);
        }

        i++;
    }

    return pop(&pila);
}

int main()
{
    char expresion[MAX];

    printf("Ingrese una expresion postfija: ");
    scanf("%s", expresion);

    printf("\nResultado = %.1f\n", evaluar(expresion));

    return 0;
}