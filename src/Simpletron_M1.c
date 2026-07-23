#include <stdio.h>

#define MEMSIZE 100

#define READ 10
#define WRITE 11
#define LOAD 20
#define STORE 21
#define ADD 30
#define SUBTRACT 31
#define DIVIDE 32
#define MULTIPLY 33
#define BRANCH 40
#define BRANCHNEG 41
#define BRANCHZERO 42
#define HALT 43

int cargarPrograma(int memory[]){
    FILE *archivo;
    int input, posicion=0;

    archivo=fopen("programa.simp","r");

    if(archivo!=NULL){
        printf("*** Archivo programa.simp encontrado ***\n");
        printf("*** Cargando programa desde archivo ***\n\n");

        while(posicion<MEMSIZE && fscanf(archivo,"%d",&input)==1){
            if(input==9999) break;

            if(input<-9999 || input>9999){
                printf("ERROR: Valor invalido en la posicion %02d\n",posicion);
                fclose(archivo);
                return -1;
            }

            memory[posicion++]=input;
        }

        if(!feof(archivo) && input!=9999){
            printf("ERROR: El archivo contiene una linea invalida\n");
            fclose(archivo);
            return -1;
        }

        fclose(archivo);
        printf("*** Programa cargado correctamente ***\n");
        return posicion;
    }

    printf("*** No se encontro programa.simp ***\n");
    printf("*** Se usara la carga interactiva ***\n\n");

    while(posicion<MEMSIZE){
        printf("%02d ? ",posicion);

        if(scanf("%d",&input)!=1){
            printf("ERROR: Entrada invalida\n");
            return -1;
        }

        if(input==9999) break;

        if(input<-9999 || input>9999){
            printf("ERROR: La palabra debe estar entre -9999 y 9999\n");
            continue;
        }

        memory[posicion++]=input;
    }

    return posicion;
}

void dump(int memory[], int accumulator,int instructionCounter,
          int instructionRegister,int operationCode,int operand){
    int i,j;
    printf("\nREGISTROS\n");
    printf("Accumulator           %+05d\n",accumulator);
    printf("InstructionCounter      %02d\n",instructionCounter);
    printf("InstructionRegister  %+05d\n",instructionRegister);
    printf("OperationCode           %02d\n",operationCode);
    printf("Operand                 %02d\n",operand);

    printf("\nMEMORIA\n      ");
    for(i=0;i<10;i++) printf("%6d",i);
    printf("\n");
    for(i=0;i<10;i++){
        printf("%2d   ",i*10);
        for(j=0;j<10;j++) printf("%+05d ",memory[i*10+j]);
        printf("\n");
    }
}

int main(void){
    int memory[MEMSIZE]={0};
    int accumulator=0,instructionCounter=0,instructionRegister=0;
    int operationCode=0,operand=0;

    printf("*** Bienvenido a Simpletron ***\n");
    printf("*** Introduzca su programa una instruccion ***\n");
    printf("*** (o palabra de datos) a la vez en la linea***\n");
    printf("*** de texto de entrada. Yo indicare el numero ***\n");
    printf("*** de posicion y una interrogacion (?). Usted ***\n");
    printf("*** tecleara entonces la palabra para esa ***\n");
    printf("*** posicion. Haga clic en el boton LISTO para***\n");
    printf("*** dejar de introducir su programa ***\n");
    printf("*** Capture instrucciones (9999 para terminar) ***\n\n");

    if(cargarPrograma(memory)<0) return 1;

    printf("\nSe comenzo a cargar el programa, comienza la ejecucion del programa\n\n");
    instructionCounter=0;

    while(1){
        if(instructionCounter<0 || instructionCounter>=MEMSIZE){
            printf("ERROR: Contador de instruccion fuera de rango\n");
            return 1;
        }

        instructionRegister=memory[instructionCounter];
        operationCode=instructionRegister/100;
        operand=instructionRegister%100;

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
                printf("%d\n",memory[operand]);
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
