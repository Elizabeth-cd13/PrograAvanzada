#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX 200

/*----pila---------*/

char pila[MAX];
int tope = -1;

int isEmpty()
{
    return tope == -1;
}

void push(char dato)
{
    if(tope < MAX - 1)
        pila[++tope] = dato;
}

char pop()
{
    if(!isEmpty())
        return pila[tope--];

    return '\0';
}

char stackTop()
{
    if(!isEmpty())
        return pila[tope];

    return '\0';
}

/*-----inclusion de float-------*/

float pilaf[MAX];
int topef = -1;

int isEmptyf()
{
    return topef == -1;
}

void pushf(float dato)
{
    if(topef < MAX - 1)
        pilaf[++topef] = dato;
}

float popf()
{
    if(!isEmptyf())
        return pilaf[topef--];

    return 0;
}

/*---funciones de parentesis-----*/

int apertura(char c)
{
    return c == '(' || c == '[' || c == '{';
}
int cierre(char c)
{
    return c == ')' || c == ']' || c == '}';
}
int equivalente(char a,char b)
{
    return (a=='('&&b==')') ||
           (a=='['&&b==']') ||
           (a=='{'&&b=='}');
}
int operador(char c)
{
    return strchr("+-*/^", c) != NULL;
}
int operando(char c)
{
    return isdigit(c);
}

int expresionValida(char exp[])
{
    int i;

    if(strlen(exp)==0)
        return 0;

    for(i=0; exp[i]!='\0'; i++)
    {
        if(exp[i]==' ')
            continue;

        if(!isdigit(exp[i]) &&
           !operador(exp[i]) &&
           !apertura(exp[i]) &&
           !cierre(exp[i]))
        {
            return 0;
        }

        if(operador(exp[i]) && operador(exp[i+1]))
            return 0;
    }

    return 1;
}


int profundidad(char expresion[])
{
    int i = 0;
    char simbolo, temp;

    tope = -1;

    while(expresion[i] != '\0')
    {
        simbolo = expresion[i];

        if(apertura(simbolo))
        {
            push(simbolo);
        }

        else if(cierre(simbolo))
        {
            if(isEmpty())
                return 0;

            temp = pop();

            if(!equivalente(temp, simbolo))
                return 0;
        }

        i++;
    }

    if(!isEmpty())
        return 0;

    return 1;
}


int jerarquia(char op)
{
    switch(op)
    {
        case '^':
            return 3;

        case '*':
        case '/':
            return 2;

        case '+':
        case '-':
            return 1;

        default:
            return 0;
    }
}

int prec(char op1, char op2)
{
    return jerarquia(op1) >= jerarquia(op2);
}
void postfijo(char infija[], char post[])
{
    int i = 0;
    int j = 0;
    char simbolo;

    tope = -1;

    while(infija[i] != '\0')
    {
        simbolo = infija[i];

        /* Ignorar espacios */
        if(simbolo == ' ')
        {
            i++;
            continue;
        }

        /* Si es un numero */
        if(isdigit(simbolo))
        {
            while(isdigit(infija[i]))
            {
                post[j++] = infija[i++];
            }

            /* Separador entre operandos */
            post[j++] = ' ';
            continue;
        }

        /* Si abre parentesis */
        if(apertura(simbolo))
        {
            push(simbolo);
        }

        /* Si cierra parentesis */
        else if(cierre(simbolo))
        {
            while(!isEmpty() && !equivalente(stackTop(), simbolo))
            {
                post[j++] = pop();
                post[j++] = ' ';
            }

            if(!isEmpty())
                pop();
        }

        /* Si es operador */
        else if(operador(simbolo))
        {
            while(!isEmpty() &&
                  operador(stackTop()) &&
                  prec(stackTop(), simbolo))
            {
                post[j++] = pop();
                post[j++] = ' ';
            }

            push(simbolo);
        }

        i++;
    }

    while(!isEmpty())
    {
        post[j++] = pop();
        post[j++] = ' ';
    }

    post[j] = '\0';
}


float evaluarPostfijo(char post[])
{
    int i = 0;
    float op1, op2;
    float numero;

    topef = -1;

    while(post[i] != '\0')
    {
        if(post[i] == ' ')
        {
            i++;
            continue;
        }

        if(isdigit(post[i]))
        {
            numero = 0;

            while(isdigit(post[i]))
            {
                numero = numero * 10 + (post[i] - '0');
                i++;
            }

            pushf(numero);
            continue;
        }

        if(operador(post[i]))
        {
            if(topef < 1)
            {
                printf("Error: expresion invalida.\n");
                return 0;
            }

            op2 = popf();
            op1 = popf();

            switch(post[i])
            {
                case '+':
                    pushf(op1 + op2);
                    break;

                case '-':
                    pushf(op1 - op2);
                    break;

                case '*':
                    pushf(op1 * op2);
                    break;

                case '/':
                    if(op2 == 0)
                    {
                        printf("Error: division entre cero.\n");
                        return 0;
                    }
                    pushf(op1 / op2);
                    break;

                case '^':
                    pushf(pow(op1, op2));
                    break;
            }
        }

        i++;
    }

    return popf();
}

int main()
{
    char infija[MAX];
    char post[MAX];
    float resultado;

    printf("Ingrese la expresion: ");
    fgets(infija, MAX, stdin);

    /* Eliminar salto de linea */
    infija[strcspn(infija, "\n")] = '\0';

    /* Expresion vacia */
    if(strlen(infija) == 0)
    {
        printf("\nError: No se ingreso ninguna expresion.\n");
        return 0;
    }

    /* Validar caracteres */
    if(!expresionValida(infija))
    {
        printf("\nError: La expresion contiene errores.\n");
        return 0;
    }

    /* Validar parentesis */
    if(!profundidad(infija))
    {
        printf("\nError: Los simbolos de agrupacion no coinciden.\n");
        return 0;
    }

    printf("\nExpresion correcta.\n");

    /* Convertir a postfijo */
    postfijo(infija, post);

    printf("\nExpresion postfija:\n");
    printf("%s\n", post);

    /* Evaluar */
    resultado = evaluarPostfijo(post);

    printf("\nEvaluacion:\n");

    if(resultado == (int)resultado)
        printf("%d\n", (int)resultado);
    else
        printf("%.2f\n", resultado);

    return 0;
}