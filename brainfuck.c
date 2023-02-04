#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#define DEBUG

#define BUFSIZE 1000
#define ERRMSGLEN 100
#define NOERROR 1
#define ERROR 0

//structs
typedef struct INSTRUCTION{
	int jumpIndex;
	char cmd;

}INSTRUCTION;

typedef struct BFDATA{
	char* dataPointer;
	char memory[BUFSIZE];
	INSTRUCTION instructions[BUFSIZE];
	int instLen;

	char errorMsg[ERRMSGLEN];
}BFDATA;



//declarations
int preprocessCmds(FILE* fp, BFDATA* data);
int determineJumps(BFDATA* data);
int processCmds(BFDATA* data);
int parseCMD(char cmd, BFDATA* data, int* index);
int incPointer(BFDATA* data);
int decPointer(BFDATA* data);
int incByte(BFDATA* data);
int decByte(BFDATA* data);
int outputByte(BFDATA* data);
int acceptByte(BFDATA* data);
int openBraket(BFDATA* data, int jump, int* index);
int closeBraket(BFDATA* data, int jump, int* index);
BFDATA* bfDataFactory();
void freeBFDATA(BFDATA* data);
void errorProcessing(BFDATA* data);



//reads file and puts cmds in an array within data struct
int preprocessCmds(FILE* fp, BFDATA* data){
	char cmd;
	int instIndex = 0;
	while((cmd = fgetc(fp)) != EOF){
		switch (cmd){	//sanitizes all non brainfuck chars
			case '>':
			case '<':
			case '+':
			case '-':
			case '.':
			case ',':
			case '[':
			case ']':
				data->instructions[instIndex].cmd = cmd;
				data->instructions[instIndex].jumpIndex = -1; //init neg val
				data->instLen++;
				instIndex++;
			
		}
	}

	#ifdef DEBUG
		printf("PREPROCESSCMDS\n");
		printf("\tInstruction Len = %d\n", data->instLen);
		int i;
		printf("\tInstructions:\n\t\t");
		for(i = 0; i < data->instLen; ++i){
			printf("%c ", data->instructions[i].cmd);
		}
		printf("\n");
	#endif 	
	
	return determineJumps(data);
}

//determines indexs for jumps between "[" and "]"
int determineJumps(BFDATA* data){
	int stack[BUFSIZE/2] = {0}; //can never have more than half be one type of bracket
	int stackIndex = 0;
	int len = data->instLen;

	int i;
	for(i = 0; i<len; ++i){
		//'['
		if(data->instructions[i].cmd == '['){
			stack[stackIndex] = i;	//store index of '['
			stackIndex++;
		}

		//']'
		if(data->instructions[i].cmd == ']'){
			//if to many end brackets error processing or they occur before and open bracket
			if(stackIndex == 0){
				strcpy(data->errorMsg, "Close brakets not correct");
				return ERROR;
			}

			--stackIndex;
			data->instructions[stack[stackIndex]].jumpIndex = i; //stores end bracked index for open bracket
			data->instructions[i].jumpIndex = stack[stackIndex];   //stores open bracket index for end bracket
			stack[stackIndex] = 0; 
		}

	
	}

	//error checking if there are any left over open brackets
	if(stackIndex > 0){
		strcpy(data->errorMsg, "To may open brakets");
		return ERROR;
	}

	#ifdef DEBUG
		printf("\nDETERMINEJUMPS\n");
		printf("\t(current location, jump location):\n\t\t");
		for(i = 0; i < data->instLen; ++i){
			printf("%c", data->instructions[i].cmd);
			if(data->instructions[i].jumpIndex > 0 ){
				printf("(%d,%d)", i, data->instructions[i].jumpIndex);
			}
			printf(" ");
		}
		printf("\n");
	#endif 	

	return NOERROR;
}


int processCmd(BFDATA* data){
	#ifdef DEBUG
		printf("\nPROCESSCMD\n\tDataPointer Value: %d\n", *data->dataPointer);	
	#endif 	

	int i;
	int len = data->instLen;
	for(i = 0; i<len; ++i){
		#ifdef DEBUG
			printf("\tCMD: %c\n", data->instructions[i].cmd);
		#endif 	
		parseCMD(data->instructions[i].cmd, data, &i);
	}
	return NOERROR;
}

int parseCMD(char cmd, BFDATA* data, int* index){
	switch(cmd){
		case '>':
			return incPointer(data);
		case '<':
			return decPointer(data);
		case '+':
			return incByte(data);
		case '-':
			return decByte(data);
		case '.':
			return outputByte(data);
		case ',':
			return acceptByte(data);
		case '[':
			return openBraket(data, data->instructions[*index].jumpIndex, index);
		case ']':
			return closeBraket(data, data->instructions[*index].jumpIndex, index);	
	}

	return NOERROR;

} 



int incPointer(BFDATA* data){
	++data->dataPointer;

	#ifdef DEBUG
		printf("\t\tINCPOINTER");
		printf("\tDataPointer Value: %d\n", *data->dataPointer);
	#endif 

	return NOERROR;
}

int decPointer(BFDATA* data){
	--data->dataPointer;

	#ifdef DEBUG
		printf("\t\tDECPOINTER ");
		printf("\tDataPointer Value: %d\n", *data->dataPointer);
	#endif 

	return NOERROR;
}

int incByte(BFDATA* data){
	++*(data->dataPointer);

	#ifdef DEBUG
		printf("\t\tINCBYTE ");
		printf("\tDataPointer Value: %d\n", *data->dataPointer);
	#endif 

	return NOERROR;
}

int decByte(BFDATA* data){
	--*(data->dataPointer);

	#ifdef DEBUG
		printf("\t\tDECBYTE ");
		printf("\tDataPointer Value: %d\n", *data->dataPointer);
	#endif 

	return NOERROR;
}

int outputByte(BFDATA* data){
	#ifdef DEBUG
		printf("\t\tOUTPUTBYTE\n");
	#endif 

	putchar(*(data->dataPointer));
	return NOERROR;
}

int acceptByte(BFDATA* data){
	#ifdef DEBUG
		printf("\t\tACCEPTBYTE\n");
	#endif 

	*(data->dataPointer) = getchar();
	return NOERROR;
}

int openBraket(BFDATA* data, int jump, int* index){
	#ifdef DEBUG
		printf("\t\tOPENBRAKET ");
		printf("\tDataPointer Value: %d\n", *data->dataPointer);
	#endif 

	//if byte at data pointer is zero, jump to after close bracket
	if(*(data->dataPointer) == 0){	
		int prev = *index;
		*index = jump;
		#ifdef DEBUG
			printf("\t\t\tJUMPED, from %d to %d\n",prev, *index);
		#endif 
		return NOERROR;
	} 

	#ifdef DEBUG
		printf("\t\t\tNO JUMP\n");
	#endif 


	return NOERROR;
}

int closeBraket(BFDATA* data, int jump, int* index){
	#ifdef DEBUG
		printf("\t\tOPENBRAKET ");
		printf("\tDataPointer Value: %d\n", *data->dataPointer);
	#endif 

	//if byte at data pointer is non zero, jump to after open bracket
	if(*(data->dataPointer) != 0){
		int prev = *index;
		*index = jump;
		#ifdef DEBUG
			printf("\t\t\tJUMPED, from %d to %d\n",prev, *index);
		#endif 

		return NOERROR;
	} 

	#ifdef DEBUG
		printf("\t\t\tNO JUMP\n");
	#endif 
	return NOERROR;
}


BFDATA* bfDataFactory(){
	//create 
	BFDATA* retData = NULL;
	retData = (BFDATA*) malloc(sizeof(BFDATA));
	if(retData == NULL){
		return NULL;
	}

	//initalize
	retData->dataPointer = retData->memory;
	retData->instLen = 0;
	memset(retData->memory, 0, BUFSIZE * sizeof(char));
	memset(retData->instructions, 0, BUFSIZE * sizeof(INSTRUCTION));
	memset(retData->errorMsg, 0, ERRMSGLEN * sizeof(char));

	return retData;
}

void freeBFDATA(BFDATA* data){
	free(data);
}

void errorProcessing(BFDATA* data){
	fprintf(stderr,"%s/n", data->errorMsg);
	freeBFDATA(data);
	exit(1);
}

int main(int argc, char* argv[]){
	if(argc != 2){
		fprintf(stderr,"Incorrect ARGS/n");
		exit(1);
	}

	//initialize data struct
	BFDATA* data = bfDataFactory(); 
	if(data == NULL){
		fprintf(stderr,"MALLOC FAILURE/n");
		exit(1);
	}


	//preprocess commands and determine jumps
	FILE* fp = fopen(argv[1], "r");
	if(!preprocessCmds(fp, data)){
		errorProcessing(data);
	}
	fclose(fp);

	//process commands
	if(!processCmd(data)){
		errorProcessing(data);
	}

	//wrap-up
	freeBFDATA(data);
	
	return 0;
}
