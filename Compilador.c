#include <stdio.h>
#include <ctype.h>
#include <string.h>

int main()
{
    char linea[100];
    char token[50];
    int i, j;

    printf("Ingrese una linea de Simpletron:\n");
    fgets(linea, sizeof(linea), stdin);

    i = 0;

    while (linea[i] != '\0' && linea[i] != '\n')
    {
        /* Ignorar espacios */
        if (isspace(linea[i]))
        {
            i++;
        }
        else if (isalpha(linea[i]))
        {
            j = 0;

            while (isalpha(linea[i]))
            {
                token[j] = linea[i];
                j++;
                i++;
            }

            token[j] = '\0';

            printf("%s\n", token);
        }

        else if (isdigit(linea[i]))
        {
            j = 0;

            while (isdigit(linea[i]))
            {
                token[j] = linea[i];
                j++;
                i++;
            }

            token[j] = '\0';

            printf("%s\n", token);
        }

        else if (linea[i] == '<')
        {
            if (linea[i + 1] == '=')
            {
                printf("<=\n");
                i += 2;
            }
            else
            {
                printf("<\n");
                i++;
            }
        }

        else if (linea[i] == '>')
        {
            if (linea[i + 1] == '=')
            {
                printf(">=\n");
                i += 2;
            }
            else
            {
                printf(">\n");
                i++;
            }
        }

        else if (linea[i] == '=')
        {
            if (linea[i + 1] == '=')
            {
                printf("==\n");
                i += 2;
            }
            else
            {
                printf("=\n");
                i++;
            }
        }
        else if (linea[i] == '+')
        {
            printf("+\n");
            i++;
        }

        else if (linea[i] == '-')
        {
            printf("-\n");
            i++;
        }

        else if (linea[i] == '*')
        {
            printf("*\n");
            i++;
        }

        else if (linea[i] == '/')
        {
            printf("/\n");
            i++;
        }
        else
        {
            printf("Token desconocido: %c\n", linea[i]);
            i++;
        }
    }

    return 0;
}