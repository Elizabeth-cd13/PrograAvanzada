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
    int operationCode=0,operand=0,input;

    printf("*** Bienvenido a Simpletron ***\n");
    printf("*** Introduzca su programa una instruccion ***\n");
    printf("*** (o palabra de datos) a la vez en la linea***\n");
    printf("*** de texto de entrada. Yo indicare el numero ***\n");
    printf("*** de posicion y una interrogacion (?). Usted ***\n");
    printf("*** tecleara entonces la palabra para esa ***\n");
    printf("*** posicion. Haga clic en el boton LISTO para***\n");
    printf("*** dejar de introducir su programa ***\n");
    printf("*** Capture instrucciones (9999 para terminar) ***\n\n");

    while(instructionCounter<MEMSIZE){
        printf("%02d ? ",instructionCounter);
        scanf("%d",&input);
        if(input==9999) break;
        memory[instructionCounter++]=input;
    }

    printf("\nSe comenzó a cargar el programa, comienza la ejecución del programa\n\n");
    instructionCounter=0;

    while(1){
        instructionRegister=memory[instructionCounter];
        operationCode=instructionRegister/100;
        operand=instructionRegister%100;

        switch(operationCode){
            case READ:
                printf("? ");
                scanf("%d",&memory[operand]);
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