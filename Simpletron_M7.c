#include <stdio.h>
#include <limits.h>

#define MEMSIZE 1000

#define READ 10
#define WRITE 11
#define READSTRING 12
#define WRITESTRING 13
#define NEWLINE 14
#define LOAD 20
#define STORE 21
#define ADD 30
#define SUBTRACT 31
#define DIVIDE 32
#define MULTIPLY 33
#define MOD 34
#define POWER 35
#define BRANCH 40
#define BRANCHNEG 41
#define BRANCHZERO 42
#define HALT 43

int cargarPrograma(int memory[]){
    FILE *archivo;
    int input, posicion=0, resultado;

    archivo=fopen("programa.simp","r");

    if(archivo!=NULL){
        printf("*** Archivo programa.simp encontrado ***\n");
        printf("*** Cargando programa desde archivo ***\n\n");

        while(posicion<MEMSIZE){
            resultado=fscanf(archivo,"%d",&input);

            if(resultado==EOF) break;

            if(resultado!=1){
                printf("ERROR: El archivo contiene una linea invalida\n");
                fclose(archivo);
                return -1;
            }

            if(input==99999) break;

            if(input<-99999 || input>99999){
                printf("ERROR: Valor invalido en la posicion %03d\n",posicion);
                fclose(archivo);
                return -1;
            }

            memory[posicion++]=input;
        }

        fclose(archivo);
        printf("*** Programa cargado correctamente ***\n");
        return posicion;
    }

    printf("*** No se encontro programa.simp ***\n");
    printf("*** Se usara la carga interactiva ***\n\n");

    while(posicion<MEMSIZE){
        printf("%03d ? ",posicion);

        if(scanf("%d",&input)!=1){
            printf("ERROR: Entrada invalida\n");
            return -1;
        }

        if(input==99999) break;

        if(input<-99999 || input>99999){
            printf("ERROR: La palabra debe estar entre -99999 y 99999\n");
            continue;
        }

        memory[posicion++]=input;
    }

    return posicion;
}

int calcularPotencia(int base,int exponente,int *resultado){
    long long potencia=1;
    int i;

    if(exponente<0) return 0;

    for(i=0;i<exponente;i++){
        potencia*=base;

        if(potencia>INT_MAX || potencia<INT_MIN) return 0;
    }

    *resultado=(int)potencia;
    return 1;
}

int leerCadena(int memory[],int direccion){
    char cadena[100];
    int longitud,i;

    printf("? ");

    if(fgets(cadena,sizeof(cadena),stdin)==NULL){
        printf("ERROR: No se pudo leer la cadena\n");
        return 0;
    }

    if(cadena[0]=='\n'){
        if(fgets(cadena,sizeof(cadena),stdin)==NULL){
            printf("ERROR: No se pudo leer la cadena\n");
            return 0;
        }
    }

    longitud=0;
    while(cadena[longitud]!='\0' && cadena[longitud]!='\n') longitud++;

    if(longitud>99){
        printf("ERROR: La cadena no puede superar 99 caracteres\n");
        return 0;
    }

    if(direccion+longitud>=MEMSIZE){
        printf("ERROR: No hay espacio suficiente para guardar la cadena\n");
        return 0;
    }

    memory[direccion]=longitud*1000;

    for(i=0;i<longitud;i++)
        memory[direccion+i+1]=(i+1)*1000+(unsigned char)cadena[i];

    return 1;
}

int escribirCadena(int memory[],int direccion){
    int longitud,i,ascii;

    if(direccion<0 || direccion>=MEMSIZE){
        printf("ERROR: Direccion de cadena invalida\n");
        return 0;
    }

    longitud=memory[direccion]/1000;

    if(longitud<0 || longitud>99 || direccion+longitud>=MEMSIZE){
        printf("ERROR: Cadena almacenada incorrectamente\n");
        return 0;
    }

    for(i=1;i<=longitud;i++){
        ascii=memory[direccion+i]%1000;

        if(ascii<0 || ascii>255){
            printf("\nERROR: Codigo ASCII invalido\n");
            return 0;
        }

        putchar(ascii);
    }

    return 1;
}

void dump(int memory[], int accumulator,int instructionCounter,
          int instructionRegister,int operationCode,int operand){
    int i,j;

    printf("\nREGISTROS\n");
    printf("Accumulator            %+06d\n",accumulator);
    printf("InstructionCounter       %03d\n",instructionCounter);
    printf("InstructionRegister    %+06d\n",instructionRegister);
    printf("OperationCode             %02d\n",operationCode);
    printf("Operand                  %03d\n",operand);

    printf("\nMEMORIA\n        ");
    for(i=0;i<10;i++) printf("%7d",i);
    printf("\n");

    for(i=0;i<MEMSIZE;i+=10){
        printf("%03d   ",i);

        for(j=0;j<10;j++)
            printf("%+06d ",memory[i+j]);

        printf("\n");
    }
}

int main(void){
    int memory[MEMSIZE]={0};
    int accumulator=0,instructionCounter=0,instructionRegister=0;
    int operationCode=0,operand=0;
    int resultadoPotencia;

    printf("*** Bienvenido a Simpletron ***\n");
    printf("*** La memoria contiene 1000 posiciones ***\n");
    printf("*** Las instrucciones usan 2 digitos de operacion ***\n");
    printf("*** y 3 digitos para la direccion de memoria ***\n");
    printf("*** Capture 99999 para terminar la carga ***\n\n");

    if(cargarPrograma(memory)<0) return 1;

    printf("\nSe comenzo a cargar el programa, comienza la ejecucion del programa\n\n");
    instructionCounter=0;

    while(1){
        if(instructionCounter<0 || instructionCounter>=MEMSIZE){
            printf("ERROR: Contador de instruccion fuera de rango\n");
            return 1;
        }

        instructionRegister=memory[instructionCounter];
        operationCode=instructionRegister/1000;
        operand=instructionRegister%1000;

        if(operand<0 || operand>=MEMSIZE){
            printf("ERROR: Operando fuera de rango: %d\n",operand);
            return 1;
        }

        switch(operationCode){
            case READ:
                printf("? ");
                if(scanf("%d",&memory[operand])!=1){
                    printf("ERROR: Entrada invalida\n");
                    return 1;
                }
                break;
            case WRITE:
                printf("%d",memory[operand]);
                break;
            case READSTRING:
                if(!leerCadena(memory,operand)) return 1;
                break;
            case WRITESTRING:
                if(!escribirCadena(memory,operand)) return 1;
                break;
            case NEWLINE:
                printf("\n");
                break;
            case LOAD:
                accumulator=memory[operand];
                break;
            case STORE:
                memory[operand]=accumulator;
                break;
            case ADD:
                accumulator+=memory[operand];
                break;
            case SUBTRACT:
                accumulator-=memory[operand];
                break;
            case DIVIDE:
                if(memory[operand]==0){
                    printf("ERROR: Division entre cero\n");
                    return 1;
                }
                accumulator/=memory[operand];
                break;
            case MULTIPLY:
                accumulator*=memory[operand];
                break;
            case MOD:
                if(memory[operand]==0){
                    printf("ERROR: Modulo entre cero\n");
                    return 1;
                }
                accumulator%=memory[operand];
                break;
            case POWER:
                if(!calcularPotencia(accumulator,memory[operand],&resultadoPotencia)){
                    printf("ERROR: Exponente negativo o resultado fuera de rango\n");
                    return 1;
                }
                accumulator=resultadoPotencia;
                break;
            case BRANCH:
                instructionCounter=operand;
                continue;
            case BRANCHNEG:
                if(accumulator<0){
                    instructionCounter=operand;
                    continue;
                }
                break;
            case BRANCHZERO:
                if(accumulator==0){
                    instructionCounter=operand;
                    continue;
                }
                break;
            case HALT:
                printf("\n*** Termino la ejecucion de Simpletron ***\n");
                dump(memory,accumulator,instructionCounter,instructionRegister,operationCode,operand);
                return 0;
            default:
                printf("ERROR: Codigo de operacion invalido: %d\n",operationCode);
                return 1;
        }

        instructionCounter++;
    }
}
