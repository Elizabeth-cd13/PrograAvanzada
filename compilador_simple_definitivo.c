

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MEMORY_SIZE 1000
#define MAX_LINE_LENGTH 1024
#define MAX_TOKENS 256
#define TOKEN_LENGTH 64
#define STACK_SIZE 256

/* Codigos de operacion de Simpletron_8 */
#define READ        10
#define WRITE       11
#define READSTRING  12
#define WRITESTRING 13
#define NEWLINE     14
#define LOAD        20
#define STORE       21
#define ADD         30
#define SUBTRACT    31
#define DIVIDE      32
#define MULTIPLY    33
#define MOD         34
#define POWER       35
#define BRANCH      40
#define BRANCHNEG   41
#define BRANCHZERO  42
#define HALT        43

typedef struct tableEntry {
    int symbol;
    char type;      /* C = constante, L = numero de linea, V = variable */
    int location;
    struct tableEntry *next;
} TableEntry;

typedef struct {
    char values[STACK_SIZE][TOKEN_LENGTH];
    int top;
} TokenStack;

typedef struct {
    int values[STACK_SIZE];
    int top;
} IntStack;

/* Memoria final compatible con Simpletron_M8. */
static double memory[MEMORY_SIZE];

/*
 * flags[i] vale -1 cuando memory[i] no tiene referencia pendiente.
 * En otro caso contiene el numero de linea SIMPLE por resolver.
 */
static int flags[MEMORY_SIZE];

static TableEntry *symbolTable = NULL;

/* El codigo crece desde 000 y los datos desde 999. */
static int instructionCounter = 0;
static int dataCounter = MEMORY_SIZE - 1;

static int compilationError = 0;
static int previousLineNumber = -1;
static int endFound = 0;

/* ---------------- Prototipos generales ---------------- */

static void initializeCompiler(void);
static void freeSymbolTable(void);

static TableEntry *findSymbol(int symbol, char type);
static TableEntry *insertSymbol(int symbol, char type, int location);
static void printSymbolTable(void);

static int allocateData(void);
static int getVariableLocation(char variable);
static int getConstantLocation(int constant);
static int getLineLocation(int lineNumber);

static void emitInstruction(int opcode, int operand);
static void emitUnresolvedBranch(int opcode, int lineNumber);

static int tokenizeLine(char *line, char *tokens[]);
static int isIntegerToken(const char *token);
static int isLowercaseVariable(const char *token);
static int isArithmeticOperator(const char *token);
static int isRelationalOperator(const char *token);
static int isOpeningGroup(const char *token);
static int isClosingGroup(const char *token);
static int groupsMatch(const char *opening, const char *closing);

static int precedence(const char *operatorToken);
static int isRightAssociative(const char *operatorToken);

static void tokenPush(TokenStack *stack, const char *token);
static int tokenPop(TokenStack *stack, char *destination);
static const char *tokenPeek(const TokenStack *stack);
static int tokenStackEmpty(const TokenStack *stack);

static void intPush(IntStack *stack, int value);
static int intPop(IntStack *stack, int *value);
static int intStackEmpty(const IntStack *stack);

static int infixToPostfix(
    char *tokens[],
    int start,
    int end,
    char postfix[][TOKEN_LENGTH],
    int *postfixCount
);

static int compilePostfixExpression(
    char postfix[][TOKEN_LENGTH],
    int postfixCount
);

static void firstPass(FILE *sourceFile);
static void compileLine(char *line);

static void compileInput(char *tokens[], int tokenCount, int lineNumber);
static void compilePrint(char *tokens[], int tokenCount, int lineNumber);
static void compileGoto(char *tokens[], int tokenCount, int lineNumber);
static void compileIf(char *tokens[], int tokenCount, int lineNumber);
static void compileLet(char *tokens[], int tokenCount, int lineNumber);
static void compileEnd(char *tokens[], int tokenCount, int lineNumber);

static int getOperandLocation(const char *token, int lineNumber);

static void secondPass(void);
static int writeSMLFile(const char *filename);
static void printMemorySummary(void);

/* ---------------- Programa principal ---------------- */

int main(int argc, char *argv[])
{
    FILE *sourceFile;
    char sourceFilename[256];
    char outputFilename[256] = "programa.simp";

    initializeCompiler();

    if (argc >= 2) {
        strncpy(sourceFilename, argv[1], sizeof(sourceFilename) - 1);
        sourceFilename[sizeof(sourceFilename) - 1] = '\0';
    } else {
        printf("Nombre del archivo SIMPLE: ");

        if (scanf("%255s", sourceFilename) != 1) {
            printf("Error: no se pudo leer el nombre del archivo.\n");
            return 1;
        }
    }

    if (argc >= 3) {
        strncpy(outputFilename, argv[2], sizeof(outputFilename) - 1);
        outputFilename[sizeof(outputFilename) - 1] = '\0';
    }

    sourceFile = fopen(sourceFilename, "r");

    if (sourceFile == NULL) {
        printf("Error: no se pudo abrir '%s'.\n", sourceFilename);
        return 1;
    }

    printf("\n=== PRIMERA PASADA ===\n");
    firstPass(sourceFile);
    fclose(sourceFile);

    if (!compilationError && !endFound) {
        printf("Error: el programa no contiene el comando end.\n");
        compilationError = 1;
    }

    if (!compilationError) {
        printf("=== SEGUNDA PASADA ===\n");
        secondPass();
    }

    if (compilationError) {
        printf("\nLa compilacion termino con errores.\n");
        freeSymbolTable();
        return 1;
    }

    if (!writeSMLFile(outputFilename)) {
        printf("Error: no se pudo crear '%s'.\n", outputFilename);
        freeSymbolTable();
        return 1;
    }

    printf("\nCompilacion completada correctamente.\n");
    printf("Archivo SML generado: %s\n", outputFilename);
    printf("Instrucciones generadas: %d\n", instructionCounter);
    printf("Primera posicion de datos utilizada: %03d\n", dataCounter + 1);

    printSymbolTable();
    printMemorySummary();

    freeSymbolTable();
    return 0;
}

/* ---------------- Inicializacion ---------------- */

static void initializeCompiler(void)
{
    int i;

    for (i = 0; i < MEMORY_SIZE; ++i) {
        memory[i] = 0.0;
        flags[i] = -1;
    }

    symbolTable = NULL;
    instructionCounter = 0;
    dataCounter = MEMORY_SIZE - 1;
    compilationError = 0;
    previousLineNumber = -1;
    endFound = 0;
}

/* ---------------- Tabla de simbolos ---------------- */

static TableEntry *findSymbol(int symbol, char type)
{
    TableEntry *current = symbolTable;

    while (current != NULL) {
        if (current->symbol == symbol && current->type == type) {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

static TableEntry *insertSymbol(int symbol, char type, int location)
{
    TableEntry *existing = findSymbol(symbol, type);
    TableEntry *newEntry;
    TableEntry *current;

    if (existing != NULL) {
        return existing;
    }

    newEntry = (TableEntry *)malloc(sizeof(TableEntry));

    if (newEntry == NULL) {
        printf("Error: no se pudo reservar memoria para symbolTable.\n");
        compilationError = 1;
        return NULL;
    }

    newEntry->symbol = symbol;
    newEntry->type = type;
    newEntry->location = location;
    newEntry->next = NULL;

    if (symbolTable == NULL) {
        symbolTable = newEntry;
    } else {
        current = symbolTable;

        while (current->next != NULL) {
            current = current->next;
        }

        current->next = newEntry;
    }

    return newEntry;
}

static void freeSymbolTable(void)
{
    TableEntry *current = symbolTable;

    while (current != NULL) {
        TableEntry *nextEntry = current->next;
        free(current);
        current = nextEntry;
    }

    symbolTable = NULL;
}

static void printSymbolTable(void)
{
    TableEntry *current = symbolTable;

    printf("\nTABLA DE SIMBOLOS\n");
    printf("%-12s %-6s %-10s\n", "SIMBOLO", "TIPO", "UBICACION");

    while (current != NULL) {
        if (current->type == 'V') {
            printf("%-12c %-6c %03d\n",
                   (char)current->symbol,
                   current->type,
                   current->location);
        } else {
            printf("%-12d %-6c %03d\n",
                   current->symbol,
                   current->type,
                   current->location);
        }

        current = current->next;
    }
}

/* ---------------- Administracion de datos ---------------- */

static int allocateData(void)
{
    int location;

    /*
     * instructionCounter apunta a la siguiente posicion libre de codigo.
     * dataCounter apunta a la siguiente posicion libre de datos.
     */
    if (dataCounter <= instructionCounter) {
        printf("Error: el codigo y los datos agotaron la memoria de Simpletron.\n");
        compilationError = 1;
        return -1;
    }

    location = dataCounter;
    --dataCounter;

    return location;
}

static int getVariableLocation(char variable)
{
    TableEntry *entry = findSymbol((int)variable, 'V');
    int location;

    if (entry != NULL) {
        return entry->location;
    }

    location = allocateData();

    if (location == -1) {
        return -1;
    }

    memory[location] = 0.0;
    entry = insertSymbol((int)variable, 'V', location);

    return entry == NULL ? -1 : entry->location;
}

static int getConstantLocation(int constant)
{
    TableEntry *entry = findSymbol(constant, 'C');
    int location;

    if (entry != NULL) {
        return entry->location;
    }

    location = allocateData();

    if (location == -1) {
        return -1;
    }

    memory[location] = (double)constant;
    entry = insertSymbol(constant, 'C', location);

    return entry == NULL ? -1 : entry->location;
}

static int getLineLocation(int lineNumber)
{
    TableEntry *entry = findSymbol(lineNumber, 'L');

    return entry == NULL ? -1 : entry->location;
}

/* ---------------- Generacion SML ---------------- */

static void emitInstruction(int opcode, int operand)
{
    if (opcode < 0 || opcode > 99) {
        printf("Error interno: opcode fuera de rango: %d.\n", opcode);
        compilationError = 1;
        return;
    }

    if (operand < 0 || operand >= MEMORY_SIZE) {
        printf("Error interno: operando fuera de rango: %d.\n", operand);
        compilationError = 1;
        return;
    }

    if (instructionCounter >= dataCounter) {
        printf("Error: las instrucciones alcanzaron el area de datos.\n");
        compilationError = 1;
        return;
    }

    memory[instructionCounter] = (double)(opcode * 1000 + operand);
    ++instructionCounter;
}

static void emitUnresolvedBranch(int opcode, int lineNumber)
{
    int destination = getLineLocation(lineNumber);

    if (destination != -1) {
        emitInstruction(opcode, destination);
        return;
    }

    if (instructionCounter >= dataCounter) {
        printf("Error: no hay memoria para generar una bifurcacion.\n");
        compilationError = 1;
        return;
    }

    /* Operando 000 temporal, por ejemplo +40000. */
    memory[instructionCounter] = (double)(opcode * 1000);
    flags[instructionCounter] = lineNumber;
    ++instructionCounter;
}

/* ---------------- Analisis lexico ---------------- */

static int tokenizeLine(char *line, char *tokens[])
{
    int count = 0;
    char *token = strtok(line, " \t\r\n");

    while (token != NULL) {
        if (count >= MAX_TOKENS) {
            printf("Error: una linea contiene demasiados tokens.\n");
            compilationError = 1;
            return count;
        }

        tokens[count++] = token;
        token = strtok(NULL, " \t\r\n");
    }

    return count;
}

static int isIntegerToken(const char *token)
{
    int index = 0;

    if (token == NULL || token[0] == '\0') {
        return 0;
    }

    if (token[0] == '+' || token[0] == '-') {
        ++index;

        if (token[index] == '\0') {
            return 0;
        }
    }

    while (token[index] != '\0') {
        if (!isdigit((unsigned char)token[index])) {
            return 0;
        }

        ++index;
    }

    return 1;
}

static int isLowercaseVariable(const char *token)
{
    return token != NULL &&
           token[0] >= 'a' &&
           token[0] <= 'z' &&
           token[1] == '\0';
}

static int isArithmeticOperator(const char *token)
{
    return strcmp(token, "+") == 0 ||
           strcmp(token, "-") == 0 ||
           strcmp(token, "*") == 0 ||
           strcmp(token, "/") == 0 ||
           strcmp(token, "%") == 0 ||
           strcmp(token, "^") == 0;
}

static int isRelationalOperator(const char *token)
{
    return strcmp(token, "<") == 0 ||
           strcmp(token, "<=") == 0 ||
           strcmp(token, ">") == 0 ||
           strcmp(token, ">=") == 0 ||
           strcmp(token, "==") == 0 ||
           strcmp(token, "!=") == 0;
}

static int isOpeningGroup(const char *token)
{
    return strcmp(token, "(") == 0 ||
           strcmp(token, "[") == 0 ||
           strcmp(token, "{") == 0;
}

static int isClosingGroup(const char *token)
{
    return strcmp(token, ")") == 0 ||
           strcmp(token, "]") == 0 ||
           strcmp(token, "}") == 0;
}

static int groupsMatch(const char *opening, const char *closing)
{
    return (strcmp(opening, "(") == 0 && strcmp(closing, ")") == 0) ||
           (strcmp(opening, "[") == 0 && strcmp(closing, "]") == 0) ||
           (strcmp(opening, "{") == 0 && strcmp(closing, "}") == 0);
}

/* ---------------- Pilas ---------------- */

static void tokenPush(TokenStack *stack, const char *token)
{
    if (stack->top >= STACK_SIZE - 1) {
        printf("Error: desbordamiento de la pila de operadores.\n");
        compilationError = 1;
        return;
    }

    ++stack->top;
    strncpy(stack->values[stack->top], token, TOKEN_LENGTH - 1);
    stack->values[stack->top][TOKEN_LENGTH - 1] = '\0';
}

static int tokenPop(TokenStack *stack, char *destination)
{
    if (stack->top < 0) {
        return 0;
    }

    strcpy(destination, stack->values[stack->top]);
    --stack->top;

    return 1;
}

static const char *tokenPeek(const TokenStack *stack)
{
    return stack->top < 0 ? NULL : stack->values[stack->top];
}

static int tokenStackEmpty(const TokenStack *stack)
{
    return stack->top < 0;
}

static void intPush(IntStack *stack, int value)
{
    if (stack->top >= STACK_SIZE - 1) {
        printf("Error: desbordamiento de la pila de operandos.\n");
        compilationError = 1;
        return;
    }

    stack->values[++stack->top] = value;
}

static int intPop(IntStack *stack, int *value)
{
    if (stack->top < 0) {
        return 0;
    }

    *value = stack->values[stack->top--];
    return 1;
}

static int intStackEmpty(const IntStack *stack)
{
    return stack->top < 0;
}

/* ---------------- Infija a postfija ---------------- */

static int precedence(const char *operatorToken)
{
    if (strcmp(operatorToken, "^") == 0) {
        return 3;
    }

    if (strcmp(operatorToken, "*") == 0 ||
        strcmp(operatorToken, "/") == 0 ||
        strcmp(operatorToken, "%") == 0) {
        return 2;
    }

    if (strcmp(operatorToken, "+") == 0 ||
        strcmp(operatorToken, "-") == 0) {
        return 1;
    }

    return 0;
}

static int isRightAssociative(const char *operatorToken)
{
    return strcmp(operatorToken, "^") == 0;
}

static int infixToPostfix(
    char *tokens[],
    int start,
    int end,
    char postfix[][TOKEN_LENGTH],
    int *postfixCount
)
{
    TokenStack operators;
    char popped[TOKEN_LENGTH];
    int index;
    int expectingOperand = 1;

    operators.top = -1;
    *postfixCount = 0;

    if (start >= end) {
        printf("Error: expresion vacia en let.\n");
        return 0;
    }

    for (index = start; index < end; ++index) {
        const char *token = tokens[index];

        if (isLowercaseVariable(token) || isIntegerToken(token)) {
            if (!expectingOperand) {
                printf("Error: faltó un operador antes de '%s'.\n", token);
                return 0;
            }

            if (*postfixCount >= STACK_SIZE) {
                printf("Error: expresion demasiado larga.\n");
                return 0;
            }

            strncpy(postfix[*postfixCount], token, TOKEN_LENGTH - 1);
            postfix[*postfixCount][TOKEN_LENGTH - 1] = '\0';
            ++(*postfixCount);
            expectingOperand = 0;
        } else if (isOpeningGroup(token)) {
            if (!expectingOperand) {
                printf("Error: faltó un operador antes de '%s'.\n", token);
                return 0;
            }

            tokenPush(&operators, token);
            expectingOperand = 1;
        } else if (isClosingGroup(token)) {
            const char *top;

            if (expectingOperand) {
                printf("Error: agrupador de cierre inesperado '%s'.\n", token);
                return 0;
            }

            top = tokenPeek(&operators);

            while (top != NULL && !isOpeningGroup(top)) {
                tokenPop(&operators, popped);
                strcpy(postfix[(*postfixCount)++], popped);
                top = tokenPeek(&operators);
            }

            if (top == NULL || !groupsMatch(top, token)) {
                printf("Error: simbolos de agrupacion desbalanceados.\n");
                return 0;
            }

            tokenPop(&operators, popped);
            expectingOperand = 0;
        } else if (isArithmeticOperator(token)) {
            const char *top;

            if (expectingOperand) {
                printf("Error: operador inesperado '%s'.\n", token);
                return 0;
            }

            top = tokenPeek(&operators);

            while (top != NULL &&
                   isArithmeticOperator(top) &&
                   ((!isRightAssociative(token) &&
                     precedence(top) >= precedence(token)) ||
                    (isRightAssociative(token) &&
                     precedence(top) > precedence(token)))) {
                tokenPop(&operators, popped);
                strcpy(postfix[(*postfixCount)++], popped);
                top = tokenPeek(&operators);
            }

            tokenPush(&operators, token);
            expectingOperand = 1;
        } else {
            printf("Error: token invalido en expresion: '%s'.\n", token);
            return 0;
        }

        if (compilationError) {
            return 0;
        }
    }

    if (expectingOperand) {
        printf("Error: la expresion termina con un operador.\n");
        return 0;
    }

    while (!tokenStackEmpty(&operators)) {
        tokenPop(&operators, popped);

        if (isOpeningGroup(popped) || isClosingGroup(popped)) {
            printf("Error: simbolos de agrupacion desbalanceados.\n");
            return 0;
        }

        strcpy(postfix[(*postfixCount)++], popped);
    }

    return 1;
}

/* ---------------- Postfija a instrucciones SML ---------------- */

static int compilePostfixExpression(
    char postfix[][TOKEN_LENGTH],
    int postfixCount
)
{
    IntStack operands;
    int index;

    operands.top = -1;

    for (index = 0; index < postfixCount; ++index) {
        const char *token = postfix[index];

        if (isLowercaseVariable(token)) {
            int location = getVariableLocation(token[0]);

            if (location == -1) {
                return -1;
            }

            intPush(&operands, location);
        } else if (isIntegerToken(token)) {
            long parsedValue;
            char *endPointer;
            int location;

            parsedValue = strtol(token, &endPointer, 10);

            if (*endPointer != '\0' ||
                parsedValue < INT_MIN ||
                parsedValue > INT_MAX) {
                printf("Error: constante fuera del rango de int: %s.\n", token);
                compilationError = 1;
                return -1;
            }

            location = getConstantLocation((int)parsedValue);

            if (location == -1) {
                return -1;
            }

            intPush(&operands, location);
        } else if (isArithmeticOperator(token)) {
            int rightLocation;
            int leftLocation;
            int temporaryLocation;

            if (!intPop(&operands, &rightLocation) ||
                !intPop(&operands, &leftLocation)) {
                printf("Error: expresion postfija invalida.\n");
                compilationError = 1;
                return -1;
            }

            temporaryLocation = allocateData();

            if (temporaryLocation == -1) {
                return -1;
            }

            memory[temporaryLocation] = 0.0;

            emitInstruction(LOAD, leftLocation);

            if (strcmp(token, "+") == 0) {
                emitInstruction(ADD, rightLocation);
            } else if (strcmp(token, "-") == 0) {
                emitInstruction(SUBTRACT, rightLocation);
            } else if (strcmp(token, "*") == 0) {
                emitInstruction(MULTIPLY, rightLocation);
            } else if (strcmp(token, "/") == 0) {
                /*
                 * Cuando el divisor es una constante cero puede
                 * detectarse durante la compilacion.
                 */
                if (memory[rightLocation] == 0.0 &&
                    findSymbol(0, 'C') != NULL &&
                    findSymbol(0, 'C')->location == rightLocation) {
                    printf("Error: division entre la constante cero.\n");
                    compilationError = 1;
                    return -1;
                }

                emitInstruction(DIVIDE, rightLocation);
            } else if (strcmp(token, "%") == 0) {
                if (memory[rightLocation] == 0.0 &&
                    findSymbol(0, 'C') != NULL &&
                    findSymbol(0, 'C')->location == rightLocation) {
                    printf("Error: modulo entre la constante cero.\n");
                    compilationError = 1;
                    return -1;
                }

                emitInstruction(MOD, rightLocation);
            } else if (strcmp(token, "^") == 0) {
                emitInstruction(POWER, rightLocation);
            }

            emitInstruction(STORE, temporaryLocation);
            intPush(&operands, temporaryLocation);
        } else {
            printf("Error interno: token postfijo desconocido '%s'.\n", token);
            compilationError = 1;
            return -1;
        }

        if (compilationError) {
            return -1;
        }
    }

    {
        int resultLocation;

        if (!intPop(&operands, &resultLocation) ||
            !intStackEmpty(&operands)) {
            printf("Error: expresion aritmetica mal formada.\n");
            compilationError = 1;
            return -1;
        }

        return resultLocation;
    }
}

/* ---------------- Primera pasada ---------------- */

static void firstPass(FILE *sourceFile)
{
    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), sourceFile) != NULL) {
        compileLine(line);

        if (compilationError) {
            return;
        }
    }
}

static void compileLine(char *line)
{
    char lineCopy[MAX_LINE_LENGTH];
    char *tokens[MAX_TOKENS];
    int tokenCount;
    long parsedLine;
    char *endPointer;
    int lineNumber;

    strncpy(lineCopy, line, sizeof(lineCopy) - 1);
    lineCopy[sizeof(lineCopy) - 1] = '\0';

    tokenCount = tokenizeLine(lineCopy, tokens);

    if (compilationError || tokenCount == 0) {
        return;
    }

    if (!isIntegerToken(tokens[0])) {
        printf("Error: cada enunciado debe iniciar con un numero de linea.\n");
        compilationError = 1;
        return;
    }

    parsedLine = strtol(tokens[0], &endPointer, 10);

    if (*endPointer != '\0' ||
        parsedLine < 0 ||
        parsedLine > INT_MAX) {
        printf("Error: numero de linea invalido: %s.\n", tokens[0]);
        compilationError = 1;
        return;
    }

    lineNumber = (int)parsedLine;

    if (lineNumber <= previousLineNumber) {
        printf("Error: los numeros de linea deben estar en orden ascendente. "
               "Se encontro %d despues de %d.\n",
               lineNumber,
               previousLineNumber);
        compilationError = 1;
        return;
    }

    previousLineNumber = lineNumber;

    if (findSymbol(lineNumber, 'L') != NULL) {
        printf("Error: numero de linea repetido: %d.\n", lineNumber);
        compilationError = 1;
        return;
    }

    if (endFound) {
        printf("Error: existe codigo despues del comando end, en la linea %d.\n",
               lineNumber);
        compilationError = 1;
        return;
    }

    /*
     * Todo numero de linea se registra, incluso si corresponde a rem.
     * Su location es la siguiente instruccion SML que se generaria.
     */
    if (insertSymbol(lineNumber, 'L', instructionCounter) == NULL) {
        return;
    }

    if (tokenCount < 2) {
        printf("Error: falta el comando en la linea %d.\n", lineNumber);
        compilationError = 1;
        return;
    }

    /*
     * Los comandos deben escribirse exactamente en minusculas.
     * En rem se permite cualquier texto despues del comando.
     */
    if (strcmp(tokens[1], "rem") == 0) {
        return;
    } else if (strcmp(tokens[1], "input") == 0) {
        compileInput(tokens, tokenCount, lineNumber);
    } else if (strcmp(tokens[1], "print") == 0) {
        compilePrint(tokens, tokenCount, lineNumber);
    } else if (strcmp(tokens[1], "goto") == 0) {
        compileGoto(tokens, tokenCount, lineNumber);
    } else if (strcmp(tokens[1], "if") == 0) {
        compileIf(tokens, tokenCount, lineNumber);
    } else if (strcmp(tokens[1], "let") == 0) {
        compileLet(tokens, tokenCount, lineNumber);
    } else if (strcmp(tokens[1], "end") == 0) {
        compileEnd(tokens, tokenCount, lineNumber);
    } else {
        printf("Error: comando desconocido o no escrito en minusculas "
               "'%s' en la linea %d.\n",
               tokens[1],
               lineNumber);
        compilationError = 1;
    }
}

/* ---------------- Compilacion de comandos ---------------- */

static void compileInput(char *tokens[], int tokenCount, int lineNumber)
{
    int location;

    if (tokenCount != 3 || !isLowercaseVariable(tokens[2])) {
        printf("Error en linea %d: formato esperado 'linea input variable'.\n",
               lineNumber);
        compilationError = 1;
        return;
    }

    location = getVariableLocation(tokens[2][0]);

    if (location != -1) {
        emitInstruction(READ, location);
    }
}

static void compilePrint(char *tokens[], int tokenCount, int lineNumber)
{
    int location;

    if (tokenCount != 3 || !isLowercaseVariable(tokens[2])) {
        printf("Error en linea %d: formato esperado 'linea print variable'.\n",
               lineNumber);
        compilationError = 1;
        return;
    }

    location = getVariableLocation(tokens[2][0]);

    if (location != -1) {
        emitInstruction(WRITE, location);
    }
}

static void compileGoto(char *tokens[], int tokenCount, int lineNumber)
{
    long destination;
    char *endPointer;

    if (tokenCount != 3 || !isIntegerToken(tokens[2])) {
        printf("Error en linea %d: formato esperado 'linea goto destino'.\n",
               lineNumber);
        compilationError = 1;
        return;
    }

    destination = strtol(tokens[2], &endPointer, 10);

    if (*endPointer != '\0' ||
        destination < 0 ||
        destination > INT_MAX) {
        printf("Error en linea %d: destino goto invalido '%s'.\n",
               lineNumber,
               tokens[2]);
        compilationError = 1;
        return;
    }

    emitUnresolvedBranch(BRANCH, (int)destination);
}

static int getOperandLocation(const char *token, int lineNumber)
{
    if (isLowercaseVariable(token)) {
        return getVariableLocation(token[0]);
    }

    if (isIntegerToken(token)) {
        long value;
        char *endPointer;

        value = strtol(token, &endPointer, 10);

        if (*endPointer != '\0' ||
            value < INT_MIN ||
            value > INT_MAX) {
            printf("Error en linea %d: constante fuera del rango de int '%s'.\n",
                   lineNumber,
                   token);
            compilationError = 1;
            return -1;
        }

        return getConstantLocation((int)value);
    }

    printf("Error en linea %d: operando invalido '%s'.\n",
           lineNumber,
           token);
    compilationError = 1;
    return -1;
}

static void compileIf(char *tokens[], int tokenCount, int lineNumber)
{
    int leftLocation;
    int rightLocation;
    long destination;
    char *endPointer;
    const char *relation;

    /*
     * Formato:
     * numero if operando operador operando goto linea
     */
    if (tokenCount != 7 ||
        !isRelationalOperator(tokens[3]) ||
        strcmp(tokens[5], "goto") != 0 ||
        !isIntegerToken(tokens[6])) {
        printf("Error en linea %d: formato esperado "
               "'linea if op1 operador op2 goto destino'.\n",
               lineNumber);
        compilationError = 1;
        return;
    }

    leftLocation = getOperandLocation(tokens[2], lineNumber);
    rightLocation = getOperandLocation(tokens[4], lineNumber);

    if (compilationError) {
        return;
    }

    destination = strtol(tokens[6], &endPointer, 10);

    if (*endPointer != '\0' ||
        destination < 0 ||
        destination > INT_MAX) {
        printf("Error en linea %d: destino if/goto invalido '%s'.\n",
               lineNumber,
               tokens[6]);
        compilationError = 1;
        return;
    }

    relation = tokens[3];

    /*
     * Igual:
     * left - right == 0
     */
    if (strcmp(relation, "==") == 0) {
        emitInstruction(LOAD, leftLocation);
        emitInstruction(SUBTRACT, rightLocation);
        emitUnresolvedBranch(BRANCHZERO, (int)destination);
    }

    /*
     * Diferente:
     * Si left-right es cero se salta el BRANCH destino.
     * En otro caso se bifurca al destino.
     */
    else if (strcmp(relation, "!=") == 0) {
        int skipAddress;

        emitInstruction(LOAD, leftLocation);
        emitInstruction(SUBTRACT, rightLocation);

        skipAddress = instructionCounter + 2;

        if (skipAddress >= MEMORY_SIZE) {
            printf("Error: direccion interna de salto fuera de rango.\n");
            compilationError = 1;
            return;
        }

        emitInstruction(BRANCHZERO, skipAddress);
        emitUnresolvedBranch(BRANCH, (int)destination);
    }

    /*
     * Menor:
     * left - right < 0
     */
    else if (strcmp(relation, "<") == 0) {
        emitInstruction(LOAD, leftLocation);
        emitInstruction(SUBTRACT, rightLocation);
        emitUnresolvedBranch(BRANCHNEG, (int)destination);
    }

    /*
     * Menor o igual:
     * left - right < 0 o == 0
     */
    else if (strcmp(relation, "<=") == 0) {
        emitInstruction(LOAD, leftLocation);
        emitInstruction(SUBTRACT, rightLocation);
        emitUnresolvedBranch(BRANCHNEG, (int)destination);
        emitUnresolvedBranch(BRANCHZERO, (int)destination);
    }

    /*
     * Mayor:
     * right - left < 0
     */
    else if (strcmp(relation, ">") == 0) {
        emitInstruction(LOAD, rightLocation);
        emitInstruction(SUBTRACT, leftLocation);
        emitUnresolvedBranch(BRANCHNEG, (int)destination);
    }

    /*
     * Mayor o igual:
     * right - left < 0 o == 0
     */
    else if (strcmp(relation, ">=") == 0) {
        emitInstruction(LOAD, rightLocation);
        emitInstruction(SUBTRACT, leftLocation);
        emitUnresolvedBranch(BRANCHNEG, (int)destination);
        emitUnresolvedBranch(BRANCHZERO, (int)destination);
    }
}

static void compileLet(char *tokens[], int tokenCount, int lineNumber)
{
    char postfix[STACK_SIZE][TOKEN_LENGTH];
    int postfixCount = 0;
    int resultLocation;
    int destinationLocation;

    if (tokenCount < 5 ||
        !isLowercaseVariable(tokens[2]) ||
        strcmp(tokens[3], "=") != 0) {
        printf("Error en linea %d: formato esperado "
               "'linea let variable = expresion'.\n",
               lineNumber);
        compilationError = 1;
        return;
    }

    if (!infixToPostfix(
            tokens,
            4,
            tokenCount,
            postfix,
            &postfixCount)) {
        printf("Error en la expresion de la linea %d.\n", lineNumber);
        compilationError = 1;
        return;
    }

    resultLocation = compilePostfixExpression(postfix, postfixCount);

    if (resultLocation == -1) {
        return;
    }

    destinationLocation = getVariableLocation(tokens[2][0]);

    if (destinationLocation == -1) {
        return;
    }

    /*
     * Se conserva el procedimiento descrito en el proyecto:
     * cargar el resultado final y almacenarlo en la variable izquierda.
     */
    emitInstruction(LOAD, resultLocation);
    emitInstruction(STORE, destinationLocation);
}

static void compileEnd(char *tokens[], int tokenCount, int lineNumber)
{
    (void)tokens;

    if (tokenCount != 2) {
        printf("Error en linea %d: end no recibe operandos.\n", lineNumber);
        compilationError = 1;
        return;
    }

    emitInstruction(HALT, 0);
    endFound = 1;
}

/* ---------------- Segunda pasada ---------------- */

static void secondPass(void)
{
    int index;

    for (index = 0; index < MEMORY_SIZE; ++index) {
        if (flags[index] != -1) {
            int lineNumber = flags[index];
            TableEntry *lineEntry = findSymbol(lineNumber, 'L');
            int instruction;
            int opcode;

            if (lineEntry == NULL) {
                printf("Error: referencia hacia la linea inexistente %d "
                       "en la instruccion %03d.\n",
                       lineNumber,
                       index);
                compilationError = 1;
                return;
            }

            instruction = (int)memory[index];
            opcode = instruction / 1000;

            /*
             * Conserva el opcode e inserta location en el operando AAA.
             */
            memory[index] = (double)(opcode * 1000 + lineEntry->location);
            flags[index] = -1;
        }
    }
}

/* ---------------- Archivo final ---------------- */

static int writeSMLFile(const char *filename)
{
    FILE *outputFile;
    int index;

    outputFile = fopen(filename, "w");

    if (outputFile == NULL) {
        return 0;
    }

    /*
     * Simpletron_M8 lee hasta EOF. Se guardan las 1000 posiciones
     * para conservar instrucciones, constantes, variables y temporales.
     */
    for (index = 0; index < MEMORY_SIZE; ++index) {
        double value = memory[index];

        /*
         * Las instrucciones y constantes enteras se escriben sin decimal.
         * El formato sigue siendo valido para fscanf("%lf").
         */
        if (value == (double)((long long)value)) {
            fprintf(outputFile, "%.0f\n", value);
        } else {
            fprintf(outputFile, "%.10g\n", value);
        }
    }

    fclose(outputFile);
    return 1;
}

static void printMemorySummary(void)
{
    int index;

    printf("\nCODIGO SML GENERADO\n");

    for (index = 0; index < instructionCounter; ++index) {
        printf("%03d: %+06.0f\n", index, memory[index]);
    }

    printf("\nDATOS UTILIZADOS\n");

    for (index = dataCounter + 1; index < MEMORY_SIZE; ++index) {
        printf("%03d: %g\n", index, memory[index]);
    }
}
