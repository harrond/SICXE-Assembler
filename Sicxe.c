#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
/////////////////각 모드///////////////////////
#define EXTENDEDADDR 0x1<<12
#define PCRERLATIVE 0x1<<13
#define BASERLATIVE 0x1<<14
#define INDEXED 0x1<15
#define IMMEDIATE 0x1<<16
#define INDIRECT 0x1<<17
#define SIMPLE 0x3<<16
//////////////////end///////////////////////////

/***************************** DECLERATE VARIABLE ****************************/
typedef struct OperationCodeTable
{
	char Mnemonic[8];
	char Format;
	unsigned short int  ManchineCode;
}SICXE_OPTAB;

typedef struct SymbolTable
{
	char Label[10];
	int Address;
}SYMBOLTABLE;

typedef struct IntermediateRecord{
	unsigned short int LineIndex;
	unsigned short int Loc;
	unsigned long int ObjectCode;
	char LabelField[32];
	char OperatorField[32];
	char OperandField[32];
}IntermediateRec;

typedef struct RegisterTable{
	char name[3];
	int number;
}R_TABLE;

int Counter;
int LOCCTR[100]; //명령이 저장되는 곳의 주소를 저장하는 배열
int LocctrCounter = 0;//위 LOCCTR의 인덱스값이 되는 변수
int ProgramLen; //프로그램의 길이를 나타내는 변수(주소)
int Index;
int j;
int ManchineCode;
int SymtabCounter = 0;
int start_address;//프로그램의 시작주소를 저장하는 변수
int program_length; //프로그램의 길이를 저장하는 변수
int ArrayIndex = 0; //프로그램의 명령을 모아서 저장하는 배열의 인덱스가 되는 변수
unsigned short int base_flag=0; //base가 설정되었는지 판단 기준이 되는 변수
long base;  //base로 설정한 값을 저장하는 변수
int litCount = 0;

unsigned short int FoundOnSymtab_flag = 0; //symtab에 값이 있는지 여부를 나타내는 변수
unsigned short int FoundOnOptab_flag = 0;//optab에 값이 있는지 여부를 나타내는 변수

char Buffer[256];//파일에서 읽어드린 한줄을 저장하는 배열
char Label[32]; //buffer에서 읽어드린 label을 저장하는 배열
char Mnemonic[32]; //buffer에서 읽어드린 operator를 저장하는 배열
char Operand[32]; //buffer에서 읽어드린 operand를 저장하는 배열

SYMBOLTABLE LITTAB[20];
SYMBOLTABLE SYMTAB[20];
IntermediateRec* IMRArray[100];


static SICXE_OPTAB OPTAB[]=
{
    {   "ADD",  '3',  0x18},
    {   "AND",  '3',  0x40},
    {  "COMP",  '3',  0x28},
    {   "DIV",  '3',  0x24},
    {     "J",  '3',  0x3C},
    {   "JEQ",  '3',  0x30},
    {   "JGT",  '3',  0x34},
    {   "JLT",  '3',  0x38},
    {  "JSUB",  '3',  0x48},
    {   "LDA",  '3',  0x00},
    {  "LDCH",  '3',  0x50},
    {   "LDL",  '3',  0x08},
    {   "LDX",  '3',  0x04},
    {   "MUL",  '3',  0x20},
    {    "OR",  '3',  0x44},
    {    "RD",  '3',  0xD8},
    {  "RSUB",  '3',  0x4C},
    {   "STA",  '3',  0x0C},
    {  "STCH",  '3',  0x54},
    {   "STL",  '3',  0x14},
    {  "STSW",  '3',  0xE8},
    {   "STX",  '3',  0x10},
    {   "SUB",  '3',  0x1C},
    {    "TD",  '3',  0xE0},
    {   "TIX",  '3',  0x2C},
    {    "WD",  '3',  0xDC},
	/******SET 1추가 구성*******/
	{  "ADDR",  '2',  0x90},
	{ "CLEAR",  '2',  0xB4},
	{ "COMPR",  '2',  0xA0},
	{  "DIVR",  '2',  0x9c},
	{   "STB",  '3',  0x1C},
	{   "STS",  '3',  0x7C},
	{  "SUBR",  '2',  0x94},
	{   "LDB",  '3',  0x68},
	{   "LDL",  '3',  0x08},
	{   "LDS",  '3',  0x6c},
	{   "LDX",  '3',  0x04},
	{   "LDT",  '3',  0x74},
	{  "TIXR",  '2',  0xB8},
	{   "TIO",  '1',  0xF8},
	{"SHIFTL",  '2',  0xA4},
	{"SHIFTR",  '2',  0xA8},
	{   "STI",  '3',  0xD4},
	{   "HIO",  '1',  0xF4},
	{  "MULR",  '2',  0x98},
	{   "LPS",  '3',  0xD0},
	{   "RMO",  '2',  0xAC},
	{   "SIO",  '1',  0xF0},
	{   "SSK", ' 3',  0xEC},
	{   "STT",  '3',  0x84},
	{   "SVC",  '2',  0xB0},
	/*****SET 2 구성*******/
	{ "ADDF", '3',0x48 },
	{ "COMPF",'3',0x88 },
	{"FLOAT",'1',0xC0},
	{ "DIVF",'3', 0x64},
	{"FIX",'1',0xC4},
	{"LDF",'3',0x70},
	{ "MULF",'3',0x60 },
	{ "NORM",'1',0xC8 },
	{ "STF", '3',0x80},
	{ "SUBF",'3',0x5C },
};

R_TABLE Registers[9] = { //레지스터와 해당하는 번호를 담은 배열
	{ "A", 0x0 }, { "X", 0x1 }, { "L", 0x2 }, { "B", 0x3 }, { "S", 0x4 }, { "T", 0x5 }, { "F", 0x6 }, { "PC", 0x8 }, { "SW", 0x9 },
};
union floatingPoint{
	double var;
	unsigned short int num[3];
};

/****************************** DFINATE FUNCTION *****************************/
char* ReadLabel(){ //buffer에서 label을 읽어드려 해당 label을 반환한다.
	j = 0;//zeroing
	while (Buffer[Index] != ' ' && Buffer[Index] != '\t' && Buffer[Index] != '\n') //공백이나올때까지 label에 한문자씩 값을 넣는다.
		Label[j++] = Buffer[Index++];
	Label[j] = '\0';//읽어 드린 label에 문자열의 끝을 나타낸는 \0을 넣는다.
	return(Label);
}

void SkipSpace(){//공백문자들을 스킵하는 함수
	while (Buffer[Index] == ' ' || Buffer[Index] =='\t') //공백문자가 나오지 안을때까지 index를 증가시킨다.
		Index++;
}

char* ReadOperator(){//buffer에서 operator를 읽어드리는 함수로 읽어드린 operater를 반환한다.
	j = 0;//zeroing
	while(Buffer[Index] != ' ' && Buffer[Index] != '\t' && Buffer[Index] != '\n') // 공백이 나올때까지 Mnemonic에 buffer에서 한문자씩 값을 넣는다.
		Mnemonic[j++] = Buffer[Index++];
	Mnemonic[j] = '\0';
	return(Mnemonic);
}

char* ReadOperand(){//buffer에서 operand를 읽어드리는 함수로 읽어드린 operand를 반환한다.
	j = 0;//zeroing
	while (Buffer[Index] != ' ' && Buffer[Index] != '\t' && Buffer[Index] != '\n') // 공백이 나올때까지 Operand에 buffer에서 한문자씩 값을 넣는다.
		Operand[j++] = Buffer[Index++];
	Operand[j] = '\0';
	return(Operand);
}

void RecordSymtab(char* label){//SYMTABLE에 label과 주소를 넣는 함수

	strcpy(SYMTAB[SymtabCounter].Label,label);  //SYMTAB에 LABEL을 넣는다.
	SYMTAB[SymtabCounter].Address = LOCCTR[LocctrCounter-1]; //SYMTAB에 해당 LABEL의 주소를 넣는다.
	SymtabCounter++;//SYMTAB의 인덱스가 되는 변수의 값을 하나 증가시킨다.
}

int SearchSymtab(char* label){//SYMTAB에서 매개변수로 받은 LABEL을 찾는 함수 찾으면 1을 리턴 아니면 0을 리턴한다. 
	FoundOnSymtab_flag = 0;

	for (int k= 0; k<=SymtabCounter; k++)	{//반복문으로 SYMTAB에 LABEL값이 들어있는지 찾는다. 
		if (!strcmp(SYMTAB[k].Label,label)){//SYMTAB[K].LABEL의 값과 매개변수로 받은 LABEL의 값이 같다면 1을 리턴한다.
			FoundOnSymtab_flag = 1;
			return (FoundOnSymtab_flag);
			break;
		}
	}
	return (FoundOnSymtab_flag); //찾지못하면 0을 리턴한다.
}

int SearchOptab(char * Mnemonic){//OPTAB에서 전달받은 Nnemonic을 찾아 찾으면 1아니면 0을 리턴하는 함수
	int size = sizeof(OPTAB)/sizeof(SICXE_OPTAB);//OPTAB의 길이를 제어 저장
	char temp[8];//임시로 문자열을 저장할 변수
	int j;
	FoundOnOptab_flag = 0;
	if (Mnemonic[0] == '+'){ //4형식일 경우  
		for (j = 1; Mnemonic[j] != '\0'; j++)//+문자를 제거하고 비교하기 위해서 Mnemonic[1]부터 temp에 한문자씩 넣는다.
			temp[j - 1] = Mnemonic[j];
		temp[j-1] = '\0';//문자열의 끝
	}
	else strcpy(temp, Mnemonic); //다른 형식인 경우에는 그냥 temp에 넣는다.(코드를 줄이기 위해 temp를 일괄적으로 사용)

	for(int i=0;i<size;i++){//size만큼 반복하면서(즉,optab의 저장된 명령어의 갯수) 찾는다면 그때의 인덱스값을 counter에 저장하고 flag의 값을 1로 바꾼다.
		if(!strcmp(temp,OPTAB[i].Mnemonic)){//같은지 검사
			Counter = i;
			FoundOnOptab_flag = 1;
			break;
		}
	}
	return (FoundOnOptab_flag); //찾으면 1아니면 0을 반환한다.
}

int StrToDec(char* c){//문자열로 입력된 십진수 값을 정수로 바꿔서 리턴하는 함수
	int dec_num = 0;
	char temp[10];
	strcpy(temp,c);

	int len = strlen(c);
	for (int k = len-1, l = 1; k>=0; k--)
	{
		dec_num = dec_num+(int)(temp[k]-'0')*l;
		l = l*10;
	}
	return (dec_num);
}

int StrToHex(char* c)//문자열로 입력된 16진수 값을 정수로 바꿔서 리턴하는 함수
{
	int hex_num = 0;
	char temp[10];
	strcpy(temp, c);

	int len = strlen(temp);
	for (int k = len-1, l = 1; k >=0; k--)
	{
		if (temp[k]>='0' && temp[k]<='9')
			hex_num = hex_num + (int)(temp[k]-'0')*l;
		else if (temp[k]>='A' && temp[k]<='F')
            hex_num = hex_num + (int)(temp[k]-'A'+10)*l;
		else if (temp[k]>='a' && temp[k]>='f')
            hex_num = hex_num + (int)(temp[k]-'a'+10)*l;
		else ;
		l = l*16;
	}
	return (hex_num);
}

int ComputeLen(char* c){//매개변수로 받은 문자열에 C' , X'가 오면 뒤에오는 문자열의 바이트 수(문자 수)를 리턴한다. 
	unsigned int b;
	char len[32];
	
	strcpy(len,c);
	if (len[0] =='C' || len[0] =='c' && len[1] =='\''){ //c는 문자를 의미
		for (b = 2; b<=strlen(len); b++){
			if (len[b] == '\''){
				b -=2;
				break;
			}
		}
	}
	if (len[0] =='X' || len[0] =='x' && len[1] =='\'')//x는 hex를 의미
		b = 1;
	return (b);
}

void CreateProgramList(){ //list파일을 만드는 함수
	int loop;
	FILE *fptr_list; 
	int in_type = 0;
	fptr_list = fopen("sicxe.list","w");

    if (fptr_list == NULL)
	{
		printf("ERROE: Unable to open the sic.list.\n");
		exit(1);
	}
	/*****************************************************list파일 출력 형식****************************************************************************/
	fprintf(fptr_list, "%-4s\t%-10s%-10s%-10s\t%s\n", "LOC", "LABEL", "OPERATOR", "OPERAND", "OBJECT CODE");
	for (loop = 0; loop<ArrayIndex; loop++)
	{
		if (SearchOptab(IMRArray[loop]->OperatorField)){ //명령어의 형식에 따라 출력을 다르게 하기 위해 형식을 뽑아냄
			in_type = (int)(OPTAB[Counter].Format - '0');
		}
		if (!strcmp(IMRArray[loop]->OperandField, "BASE") || !strcmp(IMRArray[loop]->OperandField, "NOBASE")){ //BASE나 NOBASE인 어셈블러 지시자 일경우
			fprintf(fptr_list, "\t%-10s%-10s%-10s\n", IMRArray[loop]->LabelField, IMRArray[loop]->OperatorField, IMRArray[loop]->OperandField);
			continue;
		}
		if (!strcmp(IMRArray[loop]->OperatorField,"START") || !strcmp(IMRArray[loop]->OperatorField,"RESW") || !strcmp(IMRArray[loop]->OperatorField,"RESB") || !strcmp(IMRArray[loop]->OperatorField,"END"))
		    fprintf(fptr_list, "%04x\t%-10s%-10s%-10s\n", IMRArray[loop]->Loc, IMRArray[loop]->LabelField, IMRArray[loop]->OperatorField, IMRArray[loop]->OperandField);
        else
			if (IMRArray[loop]->OperatorField[0] == '+')
				fprintf(fptr_list, "%04x\t%-9s%-10s%-10s\t%08x\n", IMRArray[loop]->Loc, IMRArray[loop]->LabelField, IMRArray[loop]->OperatorField, IMRArray[loop]->OperandField, IMRArray[loop]->ObjectCode);
			else if (IMRArray[loop]->OperandField[0] == '@' || IMRArray[loop]->OperandField[0] == '#')
				fprintf(fptr_list, "%04x\t%-10s%-9s%-10s\t%06x\n", IMRArray[loop]->Loc, IMRArray[loop]->LabelField, IMRArray[loop]->OperatorField, IMRArray[loop]->OperandField, IMRArray[loop]->ObjectCode);
			else {
				if (in_type == 3) fprintf(fptr_list, "%04x\t%-10s%-10s%-10s\t%06x\n", IMRArray[loop]->Loc, IMRArray[loop]->LabelField, IMRArray[loop]->OperatorField, IMRArray[loop]->OperandField, IMRArray[loop]->ObjectCode);
				else fprintf(fptr_list, "%04x\t%-10s%-10s%-10s\t%6x\n", IMRArray[loop]->Loc, IMRArray[loop]->LabelField, IMRArray[loop]->OperatorField, IMRArray[loop]->OperandField, IMRArray[loop]->ObjectCode);
			}
	}
	/********************************************************************************************************************************************************/
	/*************************Symbol table출력부분***********************************/
	printf("\n\n-------SYMTAB-------\n");
	printf("%-10s\t%-6s\n", "SYMBOL", "LOC");
	for (int i = 0; i < SymtabCounter; i++){
		printf("%-10s\t%-6x\n", SYMTAB[i].Label, SYMTAB[i].Address);
	}
	fprintf(fptr_list,"\n\n-------SYMTAB-------\n");
	fprintf(fptr_list, "%-10s\t%-6s\n","SYMBOL","LOC");
	for (int i = 0; i < SymtabCounter; i++){
		fprintf(fptr_list,"%-10s\t%-6x\n",SYMTAB[i].Label,SYMTAB[i].Address );
	}
	/*******************************************************************************/
	fclose(fptr_list);
}

void CreateObjectCode(){ //목적코드를 화면에 출력하고 목적코드가 담긴 파일을 생성하는 함수
	int first_address; //한 레코드의 시작주소를 저장하는 변수
	int last_address;  //한 레코드의 마지막 주소를 저장하는 변수
	int temp_address=0; //레코드를 출력할때 사용되는 변수
	int temp_objectcode[30];
	int first_index; //레코드의 처음에 들어가는 목적 코드가 담긴 배열의 인덱스로 사용된다.
	int last_index; //레코드의 마지막에 들어가는 목적 코드가 담긴 배열의 인덱스로 사용된다.
	int x,xx;//반복문에 사용
	int loop; //반복문에 사용
	int inst_Format = 0;; //명령어 형식저장


	char temp_operator[30][10];
	char temp_operand[30][10];

	FILE *fptr_obj;    
	fptr_obj = fopen("sicxe.obj","w");
    if (fptr_obj == NULL)
	{
		printf("ERROE: Unable to open the sicxe.obj.\n");
		exit(1);
	}

	printf("Creating Object Code...\n\n");

	loop = 0;
	if (!strcmp(IMRArray[loop]->OperatorField, "START"))
	{
		printf("H%-6s%06x%06x\n",IMRArray[loop]->LabelField, start_address, program_length);
		fprintf(fptr_obj,"H^%-6s^%06x^%06x\n",IMRArray[loop]->LabelField, start_address, program_length);
		loop++;
	}

	while(1)
	{

		first_address = IMRArray[loop]->Loc; //레코드의 시작주소
		last_address = IMRArray[loop]->Loc + 27; //레코드의 끝주소
		first_index = loop;

		for (x = 0, temp_address = first_address ; temp_address<=last_address; loop++)
		{
		
			if (!strcmp(IMRArray[loop]->OperatorField, "END"))//end레이블의 경우에는 반복문을 나간다.
				break;
			else if (strcmp(IMRArray[loop]->OperatorField, "RESB") && strcmp(IMRArray[loop]->OperatorField, "RESW") && strcmp(IMRArray[loop]->OperandField, "BASE") && strcmp(IMRArray[loop]->OperandField, "NOBASE")) //목적코드를 생성하지 않는 어셈블러 지시자인지 판단.
			{//레코드를 생성하기위해 임시 배열에 목적코드드와 operater, operand를 저장한다.
				temp_objectcode[x] = IMRArray[loop]->ObjectCode;
				strcpy(temp_operator[x], IMRArray[loop]->OperatorField);
				strcpy(temp_operand[x], IMRArray[loop]->OperandField);
				last_index = loop+1;
			    x++;
			}
			else ;
			temp_address = IMRArray[loop+1]->Loc;
		}
	
		printf("T%06x%02x", first_address, (IMRArray[last_index]->Loc - IMRArray[first_index]->Loc)); //텍스트 레코드의 시작부분으로 시작주소와 길이를 출력함
		fprintf(fptr_obj, "T^%06x^%02x", first_address, (IMRArray[last_index]->Loc - IMRArray[first_index]->Loc));//텍스트 레코드의 시작부분으로 시작주소와 길이를 출력함

		for (xx = 0; xx<x; xx++)//목적코드 출력부분
		{
			SearchOptab(temp_operator[xx]);
			inst_Format = (int)(OPTAB[Counter].Format - '0');
			if ((strcmp(temp_operator[xx], "BYTE")==0) && (temp_operand[xx][0]=='X' || temp_operand[xx][0]=='x')){ //byte이면서 hex값인경우 1바이트에 표현되므로 2자리로 출력되게 하였다/
				printf("%02x", temp_objectcode[xx]);
				fprintf(fptr_obj, "^%02x", temp_objectcode[xx]);
			}
			
			else if (inst_Format == 1){ //1형식인 경우
				printf("%02x", temp_objectcode[xx]);
				fprintf(fptr_obj, "^%02x", temp_objectcode[xx]);
			}
			else if (inst_Format == 2){//2형식인 경우
				printf("%04x", temp_objectcode[xx]);
				fprintf(fptr_obj, "^%04x", temp_objectcode[xx]);
			}
			else if (inst_Format == 4){//4형식의 경우 4바이트로 표현이되므로 8자리에 출력되게 하였다.
				printf("%08x", temp_objectcode[xx]);
				fprintf(fptr_obj, "^%08x", temp_objectcode[xx]);
			}
			else {
				printf("%06x", temp_objectcode[xx]);
				fprintf(fptr_obj, "^%06x", temp_objectcode[xx]);
			}
		}

		printf("\n");
		fprintf(fptr_obj,"\n");
		

		if (!strcmp(IMRArray[loop]->OperatorField, "END")){ //수정레코드를 생성하는 부분이다.
			for (int i = 0; i < ArrayIndex; i++){
				if (IMRArray[i]->OperatorField[0] == '+'&&IMRArray[i]->OperandField[0]!='#'){//4형식이고 피연산자의 값이 바로 주소가 되는 immdiate 방식이 아니라면 수정 레코드를 생성한다.
					printf("M%06x%02x\n", IMRArray[i]->Loc + 1, 5); 
					fprintf(fptr_obj,"M^%06x^%02x\n", IMRArray[i]->Loc + 1, 5);
				}
			}
			break;
		}
	}

	printf("E%06x\n\n", start_address);
	fprintf(fptr_obj, "E^%06x\n\n", start_address);
	fclose(fptr_obj);
}

/******************************* MAIN FUNCTION *******************************/
void main (void)
{
	FILE* fptr;

	char filename[20];
	char label[32]; //읽어드린 레이블을 저장
	char opcode[32]; //opcode저장
	char operand[32]; //operand저장

	
	int loc = 0;
	int line = 0;
	int loop;
	int is_empty_line;
	int is_comment;
	int loader_flag = 0;
	int instructionFormat;

	printf(" ******************************************************************************\n");
	printf(" * Program: SICXE ASSEMBYER                                                     *\n");
	printf(" *                                                                            *\n");
	printf(" * Procedure:                                                                 *\n");
	printf(" *   - Enter file name of source code.                                        *\n");
	printf(" *   - Do pass 1 process.                                                     *\n");
	printf(" *   - Do pass 2 process.                                                     *\n");
	printf(" *   - Create \"program list\" data on sic.list.(Use Notepad to read this file) *\n");
	printf(" *   - Create \"object code\" data on sic.obj.(Use Notepad to read this file)   *\n");
	printf(" *   - Also output object code to standard output device.                     *\n");
	printf(" ******************************************************************************\n");


	printf("\nEnter the file name you want to assembly (sicxe.asm):");
	scanf("%s",filename);
	fptr = fopen(filename,"r");
	if (fptr == NULL)
	{
		printf("ERROE: Unable to open the %s file.\n",filename);
		exit(1);
	}

/********************************** PASS 1 ***********************************/
	printf("Pass 1 Processing...\n");
	while (fgets(Buffer,256,fptr) != NULL)
	{
		is_empty_line = strlen(Buffer); //읽어드린 문자열의 길이.

		Index = 0;
		j = 0;
		strcpy(label,ReadLabel());

        if (Label[0] == '.')//읽어드린것이 주석인지 판단
			is_comment = 1;
		else
			is_comment = 0;


		if (is_empty_line>1 && is_comment!=1)//읽어드린 문자열이 있고 주석이 아닌경우
		{
			Index = 0;
			j = 0;
		
			IMRArray[ArrayIndex] = (IntermediateRec*)malloc(sizeof(IntermediateRec));/* [A] */
			
			IMRArray[ArrayIndex]->LineIndex = ArrayIndex; //자신의 인덱스 값을 저장한다.
			strcpy(label,ReadLabel());
			strcpy(IMRArray[ArrayIndex]->LabelField,label);//읽어온 레이블을 명령문을 저장하는 배열의 lablefild에 저장한다.
			SkipSpace(); //공백을 제거

			if (line == 0) //주석문을 제외한 첫 번째 줄일 경우
			{
				
				strcpy(opcode,ReadOperator());
				strcpy(IMRArray[ArrayIndex]->OperatorField,opcode);/* [A] */
				if (!strcmp(opcode,"START"))//레이블이 start일 경우
				{
					SkipSpace();
					strcpy(operand,ReadOperand());
					strcpy(IMRArray[ArrayIndex]->OperandField, operand);/* [A] */
					LOCCTR[LocctrCounter] = StrToHex(operand);
					start_address = LOCCTR[LocctrCounter];
				}
				else
				{
					LOCCTR[LocctrCounter] = 0;
					start_address = LOCCTR[LocctrCounter];
				}
			}
			else
			{
				
				strcpy(opcode,ReadOperator());
				
				strcpy(IMRArray[ArrayIndex]->OperatorField,opcode);
				SkipSpace();
				strcpy(operand,ReadOperand());
				strcpy(IMRArray[ArrayIndex]->OperandField,operand);
				/********************************주소 배정*************************************/
				if (strcmp(opcode,"END"))//end가 아닌 경우
				{
					if (label[0] != '\0')//레이블이 있다면
					{
						if (SearchSymtab(label))
						{
							fclose(fptr);
							printf("ERROE: Duplicate Symbol\n");
							FoundOnSymtab_flag = 0;
							exit(1);
						}
						RecordSymtab(label);
					}
					if (SearchOptab(opcode)){//op코드를 검사
						instructionFormat=(int)(OPTAB[Counter].Format - '0');
						if (instructionFormat==3){//3형식인경우 확장해서 4형식이 될수도 있으므로 검사
							if (opcode[0] == '+') instructionFormat = 4;//앞의 +가 부터있으면 4형식이므로 foramt을 4로 바꿈
						}
						LOCCTR[LocctrCounter] = loc + instructionFormat;
					}
					else if (!strcmp(opcode,"WORD"))// WORD인경우 3바이트이므로 주소를 3만큼 증가해 다음주소를 나타낸다.
						LOCCTR[LocctrCounter] = loc + 3;
					else if (!strcmp(opcode,"RESW"))//RESW인경우 WORD의 크기가 3바이트이고 예약할 만큼의 수를 곱해서 다음 주소를 나타낸다 
						LOCCTR[LocctrCounter] = loc + 3 * StrToDec(operand);
					else if (!strcmp(opcode,"RESB"))//RESB인 경우 BYTE의 크기는 1바이트이므로 예약할 만큼의 수를 더해서 다음 주소를 나낸다.
						LOCCTR[LocctrCounter] = loc + StrToDec(operand); 
					else if (!strcmp(opcode,"BYTE")) //BYTE의 경우 해당 OPERAND를 넘겨 바이트의 수를 계산한다음 현재주소에 더해 다음 주소로 나타낸다.
						LOCCTR[LocctrCounter] = loc + ComputeLen(operand);
					else if (!strcmp(opcode, "BASE")){ //BASE의 경우 명령어는 인식하지만 따로 주소를 배정받지 않으므로 다음주소를 그대로 유지하게 한다.
						LOCCTR[LocctrCounter] = loc;
					}
					else if (!strcmp(opcode, "NOBASE")){//NOBASE의 경우에도 명령어는 인식하지만 따로 주소를 배정받지 않으므로 다음주소를 그대로 유지하게 한다.
						LOCCTR[LocctrCounter] = loc;
					}
					else if (!strcmp(opcode, "LTORG")){
						LOCCTR[LocctrCounter] = loc;
					}

					else{
						printf("%s", opcode);
						fclose(fptr);
						printf("ERROE: Invalid Operation Code\n");
						exit(1);
					}
				}
			}
			loc = LOCCTR[LocctrCounter];
			IMRArray[ArrayIndex]->Loc = LOCCTR[LocctrCounter-1];
			LocctrCounter++; //주소값을 담는 배열 계살할때 인덱스
			ArrayIndex++;  //명령줄 인덱스
			line += 1; //줄수
		}
		FoundOnOptab_flag = 0;
		
	}
	program_length = LOCCTR[LocctrCounter-2]- LOCCTR[0]; //프로그램 길이

/********************************** PASS 2 ***********************************/
	printf("Pass 2 Processing...\n");

	unsigned long inst_fmt; //전체적인 목적코드를 담을 변수
	unsigned long inst_fmt_opcode; //상위 1바이트를 담을 변수
	unsigned long inst_fmt_mode; //모드들에 해당하는 값을 담을 변수
	unsigned long inst_fmt_address; //목표주소 혹은 변위를 담을 변수
	long disp; //변위를 저장할 변수
	int instructionMode_flag,k,digitflag=0; 
	char tempOperand[10];
	char *regi; //분리한 레지스터 이름을 저장하는 변수
	union floatingPoint prime;

	for (loop = 1; loop<ArrayIndex; loop++){
		/////////초기화///////////
		inst_fmt_opcode = 0;
		inst_fmt_mode = 0;
		inst_fmt_address = 0;
		instructionMode_flag = 0;
		prime.var = 0;
		//////////////////////////


		strcpy(opcode,IMRArray[loop]->OperatorField); //opcode
		IMRArray[loop]->ObjectCode = inst_fmt_opcode;
		strcpy(operand, IMRArray[loop]->OperandField);

		if (!strcmp(opcode, "BASE")){
			base_flag = 1;
			for (int search_symtab = 0; search_symtab < SymtabCounter; search_symtab++){
				if (!strcmp(operand, SYMTAB[search_symtab].Label)){
					base = (long)SYMTAB[search_symtab].Address;
				}
			}
			continue;
		}
		else if (!strcmp(opcode, "NOBASE")){
			base_flag = 0;
			continue;
		}
		/*****************************************모드 지정**************************************************/
		if (SearchOptab(opcode)){
			inst_fmt_opcode = OPTAB[Counter].ManchineCode;

			switch ((int)(OPTAB[Counter].Format - '0')){ //형식에 따라 명령어 포맷의 길이가 달라지도록.
			case 1://1형식인경우
				instructionMode_flag = 1;
				break;
			case 2: //2형식인경우
				inst_fmt_opcode <<= 8;
				instructionMode_flag = 2;
				break;
			case 3: //3,4형식인경우
				if (opcode[0] == '+') {//4형식인지 판단하여 형식값을 저장
					instructionMode_flag = 4;
					inst_fmt_mode += EXTENDEDADDR;
				
				}
				else instructionMode_flag = 3; //3형식인경우 
				inst_fmt_opcode <<= 16; //3은 16비트를 밀면 해당 자리가 나오고 4형식의 경우에도 일단 밀어서 합칠때 8비트를 더 왼쪽으로 쉬프트한다.

				if (operand[0] == '#') inst_fmt_mode += IMMEDIATE; //imediate모드일경우 모드값을 저장하는 변수에 해당 값을 저장
				else if (operand[0] == '@') inst_fmt_mode += INDIRECT; //indirect모드일경우 모드값을 저장하는 변수에 해당하는 값을 저장
				else if (operand[strlen(operand) - 2] == ',' && operand[strlen(operand) - 1] == 'X'){ //index모드인지
					inst_fmt_mode += INDEXED+SIMPLE;
					operand[strlen(operand) - 2] = '\0';//
				}
				else inst_fmt_mode += SIMPLE;//sic/xe이므로 imediate모드나 indirect가 아니면 두비트를 1로 설정한 값을 더함
			}
			/*******************************************************************************************************/

			if (operand[0] == '#' || operand[0] == '@'){ //imediate나 indirect모드 일경우 label비교를 위해 기호를 제외한 label을 임시 배열에 저장함.
				for (k = 1; operand[k] != '\0'; k++){
					tempOperand[k - 1] = operand[k];
				}
				tempOperand[k-1] = '\0';
				for (int i = 0; i < k - 1; i++){//해당 피연산자가 숫자일 경우를 판단한다.ex) #4096 
					if (!isdigit(tempOperand[i])){
						digitflag = 0;
						break;
					}
					digitflag = 1;
				}

			}
			else strcpy(tempOperand, operand); //해당 모드가 아닐경우 그냥 operand를 tempOperand에 복사한다.


			///////////////////////////////////////////////////-변위&목표 주소 계산-//////////////////////////////////////////////////////////
			if (operand[0] == '#'&&digitflag == 1){ //IMMEDIATE모드이면서 뒤에 숫자값이 바로 올 경우
				inst_fmt_address = StrToDec(tempOperand);
			}
			else if (instructionMode_flag == 2){ //2형식인 경우
				regi = strtok(operand, ","); //r1과 r2가 있는 경우가 있으므로 ,를 기준으로 나눔
				if (0 <= StrToHex(regi) <= 16&&!strcmp(opcode,"SVC")){//SVC의 경우 레지스터 값말고 0~16중의 숫자를 가지므로 예외적인 설정을 추가한다.
					inst_fmt_address = StrToHex(regi)<<4;
				}
				else{
					for (int i = 0; i < 9; i++)
						if (!strcmp(regi, Registers[i].name))
							inst_fmt_address = Registers[i].number << 4; //r1의 경우 1바이트의 자리중 앞4비트이므로 왼쪽으로 4만큼 쉬프트 한다.
				}
				regi = strtok(NULL, ",");
				if (regi != NULL){
					
					if (0 <= StrToHex(regi) <= 16 && (!strcmp(opcode, "SHIFTR") || !strcmp(opcode, "SHIFTL"))){
						inst_fmt_address += StrToHex(regi);
					}
					else{
						for (int i = 0; i < 9; i++)
							if (!strcmp(regi, Registers[i].name))
								inst_fmt_address += Registers[i].number;
					}
				}
				
			}
			else{
				
				//operand에 있는 레이블을 심볼테이블에서 찾아 변위나 목표주소를 계산하여 주소필드에 해당하는 변수에 넣는다.
				//사용한 모드를 모드를 나타내는 변수에 넣는다.
				for (int search_symtab = 0; search_symtab < SymtabCounter; search_symtab++){

					if (!strcmp(tempOperand, SYMTAB[search_symtab].Label)){ //
						
						if (instructionMode_flag == 3){//3형식인 경우
							disp = (long)SYMTAB[search_symtab].Address - IMRArray[loop + 1]->Loc;
							inst_fmt_mode += PCRERLATIVE;
							if (2047 < disp && base_flag == 1){//pc의 범위를 넘고 base가 선언 되어있다면 base 주소 지정 방식을 사용한다.
								disp = (long)SYMTAB[search_symtab].Address - base;
								inst_fmt_mode -= PCRERLATIVE + BASERLATIVE;
								if (0 < disp < 4095){ //베이스 상대 주소 지정 방식의 범위
									printf("범위초과");
									return 0;
								}
							}
							else if (-20487>disp || disp>2047){ //PC 상대 주소 지정 방식의 범위
								printf("범위초과");
								return 0;
							}
							
						}
						else if(instructionMode_flag==4) disp = (long)SYMTAB[search_symtab].Address;//4형식인 경우
						inst_fmt_address = disp; //변위
						break; //해당 레이블을 찾아서 값을 넣었으므로 더 이상 반복하지 않고 빠져나간다.
					}

				}
				
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////OBJECT 코드 종합////////////////////////////////////////////////
			if (instructionMode_flag == 1) //1형식인 경우
				inst_fmt = inst_fmt_opcode;
			else if (instructionMode_flag == 2){ //2형식인 경우
				inst_fmt = inst_fmt_opcode + inst_fmt_address;
			}
			else if (instructionMode_flag == 4){ //4형식인 경우
				inst_fmt = (inst_fmt_opcode + inst_fmt_mode) << 8;
				inst_fmt += inst_fmt_address;
				if (disp < (long)0) inst_fmt += 0x100000; //목표주소가 0보다 작은 경우 더 했을때 음수여서 앞의 자리가 하나 줄어들게 되서 다시 더해준다.
			}
			else {//3형식인 경우
				inst_fmt = inst_fmt_opcode + inst_fmt_mode + inst_fmt_address;
				if (disp<(long)0){ inst_fmt += 0x1000; } //변위가 0보다 작은 경우 더 했을때 음수여서 앞의 자리가 하나 줄어들게 되서 다시 더해준다.
			}
			////////////////////////////////////////////////////////////////////////////////////////////////
			IMRArray[loop]->ObjectCode = inst_fmt;//명령에 해당하는 OBJECT code를 넣는다. 
		}

		//////////////////////////////////////데이터의 경우 OBJECT코드//////////////////////////////////////
		else if (!strcmp(opcode, "WORD")){ //word인 경우 
			strcpy(operand,IMRArray[loop]->OperandField);
			IMRArray[loop]->ObjectCode = StrToDec(operand);
		}
		else if (!strcmp(opcode,"BYTE")){ //byte인 경우
			strcpy(operand,IMRArray[loop]->OperandField);
            IMRArray[loop]->ObjectCode = 0;

			if(operand[0]=='C' || operand[0]=='c' && operand[1]=='\''){ //byte로 저장될때 문자로 저장되는 경우
				for (int x = 2; x<=(int)(strlen(operand)-2); x++){
					IMRArray[loop]->ObjectCode=IMRArray[loop]->ObjectCode + (int)operand[x];
					IMRArray[loop]->ObjectCode<<=8;
				}
			}

            if(operand[0]=='X' || operand[0]=='x' && operand[1]=='\''){ //byte로 저장될때 16진수로 저장되는 경우
				char *operand_ptr;
				operand_ptr = &operand[2];
				*(operand_ptr+2)='\0';
				for (int x=2; x<=(int)(strlen(operand)-2); x++){
					IMRArray[loop]->ObjectCode=IMRArray[loop]->ObjectCode + StrToHex(operand_ptr);
					IMRArray[loop]->ObjectCode<<=8;
				}
			}

	    	IMRArray[loop]->ObjectCode>>=8;
		}
		else
		/* do nothing */	;		
		///////////////////////////////////////////////////////////////////////////////////////////////////
	}

	CreateProgramList();
	CreateObjectCode();
	
	for (loop = 0; loop<ArrayIndex; loop++) //동적할당한 메모리 해제
        free(IMRArray[loop]);

	printf("Compeleted Assembly\n");
	fclose(fptr);
}