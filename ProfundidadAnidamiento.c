#include <stdio.h>
#include <string.h>

#define MAX 100

/*--------------- PILA ---------------*/

typedef struct
{
    char datos[MAX];
    int tope;
} Pila;

void iniciar(Pila *p)
{
    p->tope = -1;
}

int isEmpty(Pila *p)
{
    return p->tope == -1;
}

void push(Pila *p, char c)
{
    if (p->tope < MAX - 1)
    {
        p->datos[++(p->tope)] = c;
    }
}

char pop(Pila *p)
{
    if (!isEmpty(p))
    {
        return p->datos[(p->tope)--];
    }

    return '\0';
}

/*--------------- FUNCION AUXILIAR ---------------*/

int equivalente(char abre, char cierra)
{
    if (abre == '(' && cierra == ')')
        return 1;

    if (abre == '[' && cierra == ']')
        return 1;

    if (abre == '{' && cierra == '}')
        return 1;

    return 0;
}

/*--------------- FUNCION PEDIDA ---------------*/

int profundidad(char expresion[])
{
    Pila pila;
    iniciar(&pila);

    int i = 0;
    char symb;
    char temp;

    while (expresion[i] != '\0')
    {
        /* 2.1 Copiar el siguiente caracter */
        symb = expresion[i];

        /* 2.2 Si es simbolo de apertura */
        if (symb == '(' || symb == '[' || symb == '{')
        {
            push(&pila, symb);
        }

        /* 2.3 Si es simbolo de cierre */
        else if (symb == ')' || symb == ']' || symb == '}')
        {
            if (isEmpty(&pila))
            {
                return 0;
            }

            temp = pop(&pila);

            if (!equivalente(temp, symb))
            {
                return 0;
            }
        }

        i++;
    }

    if (!isEmpty(&pila))
    {
        return 0;
    }

    return 1;
}

/*--------------- MAIN ---------------*/

int main()
{
    char expresion[MAX];

    printf("Ingrese una expresion: ");
    fgets(expresion, MAX, stdin);

    expresion[strcspn(expresion, "\n")] = '\0';

    if (profundidad(expresion))
        printf("1\n");
    else
        printf("0\n");

    return 0;
}