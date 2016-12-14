#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
/////////////////�� ���///////////////////////
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
int LOCCTR[100]; //����� ����Ǵ� ���� �ּҸ� �����ϴ� �迭
int LocctrCounter = 0;//�� LOCCTR�� �ε������� �Ǵ� ����
int ProgramLen; //���α׷��� ���̸� ��Ÿ���� ����(�ּ�)
int Index;
int j;
int ManchineCode;
int SymtabCounter = 0;
int start_address;//���α׷��� �����ּҸ� �����ϴ� ����
int program_length; //���α׷��� ���̸� �����ϴ� ����
int ArrayIndex = 0; //���α׷��� ����� ��Ƽ� �����ϴ� �迭�� �ε����� �Ǵ� ����
unsigned short int base_flag=0; //base�� �����Ǿ����� �Ǵ� ������ �Ǵ� ����
long base;  //base�� ������ ���� �����ϴ� ����
int litCount = 0;

unsigned short int FoundOnSymtab_flag = 0; //symtab�� ���� �ִ��� ���θ� ��Ÿ���� ����
unsigned short int FoundOnOptab_flag = 0;//optab�� ���� �ִ��� ���θ� ��Ÿ���� ����

char Buffer[256];//���Ͽ��� �о�帰 ������ �����ϴ� �迭
char Label[32]; //buffer���� �о�帰 label�� �����ϴ� �迭
char Mnemonic[32]; //buffer���� �о�帰 operator�� �����ϴ� �迭
char Operand[32]; //buffer���� �о�帰 operand�� �����ϴ� �迭

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
	/******SET 1�߰� ����*******/
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
	/*****SET 2 ����*******/
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

R_TABLE Registers[9] = { //�������Ϳ� �ش��ϴ� ��ȣ�� ���� �迭
	{ "A", 0x0 }, { "X", 0x1 }, { "L", 0x2 }, { "B", 0x3 }, { "S", 0x4 }, { "T", 0x5 }, { "F", 0x6 }, { "PC", 0x8 }, { "SW", 0x9 },
};
union floatingPoint{
	double var;
	unsigned short int num[3];
};

/****************************** DFINATE FUNCTION *****************************/
char* ReadLabel(){ //buffer���� label�� �о��� �ش� label�� ��ȯ�Ѵ�.
	j = 0;//zeroing
	while (Buffer[Index] != ' ' && Buffer[Index] != '\t' && Buffer[Index] != '\n') //�����̳��ö����� label�� �ѹ��ھ� ���� �ִ´�.
		Label[j++] = Buffer[Index++];
	Label[j] = '\0';//�о� �帰 label�� ���ڿ��� ���� ��Ÿ���� \0�� �ִ´�.
	return(Label);
}

void SkipSpace(){//���鹮�ڵ��� ��ŵ�ϴ� �Լ�
	while (Buffer[Index] == ' ' || Buffer[Index] =='\t') //���鹮�ڰ� ������ ���������� index�� ������Ų��.
		Index++;
}

char* ReadOperator(){//buffer���� operator�� �о�帮�� �Լ��� �о�帰 operater�� ��ȯ�Ѵ�.
	j = 0;//zeroing
	while(Buffer[Index] != ' ' && Buffer[Index] != '\t' && Buffer[Index] != '\n') // ������ ���ö����� Mnemonic�� buffer���� �ѹ��ھ� ���� �ִ´�.
		Mnemonic[j++] = Buffer[Index++];
	Mnemonic[j] = '\0';
	return(Mnemonic);
}

char* ReadOperand(){//buffer���� operand�� �о�帮�� �Լ��� �о�帰 operand�� ��ȯ�Ѵ�.
	j = 0;//zeroing
	while (Buffer[Index] != ' ' && Buffer[Index] != '\t' && Buffer[Index] != '\n') // ������ ���ö����� Operand�� buffer���� �ѹ��ھ� ���� �ִ´�.
		Operand[j++] = Buffer[Index++];
	Operand[j] = '\0';
	return(Operand);
}

void RecordSymtab(char* label){//SYMTABLE�� label�� �ּҸ� �ִ� �Լ�

	strcpy(SYMTAB[SymtabCounter].Label,label);  //SYMTAB�� LABEL�� �ִ´�.
	SYMTAB[SymtabCounter].Address = LOCCTR[LocctrCounter-1]; //SYMTAB�� �ش� LABEL�� �ּҸ� �ִ´�.
	SymtabCounter++;//SYMTAB�� �ε����� �Ǵ� ������ ���� �ϳ� ������Ų��.
}

int SearchSymtab(char* label){//SYMTAB���� �Ű������� ���� LABEL�� ã�� �Լ� ã���� 1�� ���� �ƴϸ� 0�� �����Ѵ�. 
	FoundOnSymtab_flag = 0;

	for (int k= 0; k<=SymtabCounter; k++)	{//�ݺ������� SYMTAB�� LABEL���� ����ִ��� ã�´�. 
		if (!strcmp(SYMTAB[k].Label,label)){//SYMTAB[K].LABEL�� ���� �Ű������� ���� LABEL�� ���� ���ٸ� 1�� �����Ѵ�.
			FoundOnSymtab_flag = 1;
			return (FoundOnSymtab_flag);
			break;
		}
	}
	return (FoundOnSymtab_flag); //ã�����ϸ� 0�� �����Ѵ�.
}

int SearchOptab(char * Mnemonic){//OPTAB���� ���޹��� Nnemonic�� ã�� ã���� 1�ƴϸ� 0�� �����ϴ� �Լ�
	int size = sizeof(OPTAB)/sizeof(SICXE_OPTAB);//OPTAB�� ���̸� ���� ����
	char temp[8];//�ӽ÷� ���ڿ��� ������ ����
	int j;
	FoundOnOptab_flag = 0;
	if (Mnemonic[0] == '+'){ //4������ ���  
		for (j = 1; Mnemonic[j] != '\0'; j++)//+���ڸ� �����ϰ� ���ϱ� ���ؼ� Mnemonic[1]���� temp�� �ѹ��ھ� �ִ´�.
			temp[j - 1] = Mnemonic[j];
		temp[j-1] = '\0';//���ڿ��� ��
	}
	else strcpy(temp, Mnemonic); //�ٸ� ������ ��쿡�� �׳� temp�� �ִ´�.(�ڵ带 ���̱� ���� temp�� �ϰ������� ���)

	for(int i=0;i<size;i++){//size��ŭ �ݺ��ϸ鼭(��,optab�� ����� ��ɾ��� ����) ã�´ٸ� �׶��� �ε������� counter�� �����ϰ� flag�� ���� 1�� �ٲ۴�.
		if(!strcmp(temp,OPTAB[i].Mnemonic)){//������ �˻�
			Counter = i;
			FoundOnOptab_flag = 1;
			break;
		}
	}
	return (FoundOnOptab_flag); //ã���� 1�ƴϸ� 0�� ��ȯ�Ѵ�.
}

int StrToDec(char* c){//���ڿ��� �Էµ� ������ ���� ������ �ٲ㼭 �����ϴ� �Լ�
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

int StrToHex(char* c)//���ڿ��� �Էµ� 16���� ���� ������ �ٲ㼭 �����ϴ� �Լ�
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

int ComputeLen(char* c){//�Ű������� ���� ���ڿ��� C' , X'�� ���� �ڿ����� ���ڿ��� ����Ʈ ��(���� ��)�� �����Ѵ�. 
	unsigned int b;
	char len[32];
	
	strcpy(len,c);
	if (len[0] =='C' || len[0] =='c' && len[1] =='\''){ //c�� ���ڸ� �ǹ�
		for (b = 2; b<=strlen(len); b++){
			if (len[b] == '\''){
				b -=2;
				break;
			}
		}
	}
	if (len[0] =='X' || len[0] =='x' && len[1] =='\'')//x�� hex�� �ǹ�
		b = 1;
	return (b);
}

void CreateProgramList(){ //list������ ����� �Լ�
	int loop;
	FILE *fptr_list; 
	int in_type = 0;
	fptr_list = fopen("sicxe.list","w");

    if (fptr_list == NULL)
	{
		printf("ERROE: Unable to open the sic.list.\n");
		exit(1);
	}
	/*****************************************************list���� ��� ����****************************************************************************/
	fprintf(fptr_list, "%-4s\t%-10s%-10s%-10s\t%s\n", "LOC", "LABEL", "OPERATOR", "OPERAND", "OBJECT CODE");
	for (loop = 0; loop<ArrayIndex; loop++)
	{
		if (SearchOptab(IMRArray[loop]->OperatorField)){ //��ɾ��� ���Ŀ� ���� ����� �ٸ��� �ϱ� ���� ������ �̾Ƴ�
			in_type = (int)(OPTAB[Counter].Format - '0');
		}
		if (!strcmp(IMRArray[loop]->OperandField, "BASE") || !strcmp(IMRArray[loop]->OperandField, "NOBASE")){ //BASE�� NOBASE�� ����� ������ �ϰ��
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
	/*************************Symbol table��ºκ�***********************************/
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

void CreateObjectCode(){ //�����ڵ带 ȭ�鿡 ����ϰ� �����ڵ尡 ��� ������ �����ϴ� �Լ�
	int first_address; //�� ���ڵ��� �����ּҸ� �����ϴ� ����
	int last_address;  //�� ���ڵ��� ������ �ּҸ� �����ϴ� ����
	int temp_address=0; //���ڵ带 ����Ҷ� ���Ǵ� ����
	int temp_objectcode[30];
	int first_index; //���ڵ��� ó���� ���� ���� �ڵ尡 ��� �迭�� �ε����� ���ȴ�.
	int last_index; //���ڵ��� �������� ���� ���� �ڵ尡 ��� �迭�� �ε����� ���ȴ�.
	int x,xx;//�ݺ����� ���
	int loop; //�ݺ����� ���
	int inst_Format = 0;; //��ɾ� ��������


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

		first_address = IMRArray[loop]->Loc; //���ڵ��� �����ּ�
		last_address = IMRArray[loop]->Loc + 27; //���ڵ��� ���ּ�
		first_index = loop;

		for (x = 0, temp_address = first_address ; temp_address<=last_address; loop++)
		{
		
			if (!strcmp(IMRArray[loop]->OperatorField, "END"))//end���̺��� ��쿡�� �ݺ����� ������.
				break;
			else if (strcmp(IMRArray[loop]->OperatorField, "RESB") && strcmp(IMRArray[loop]->OperatorField, "RESW") && strcmp(IMRArray[loop]->OperandField, "BASE") && strcmp(IMRArray[loop]->OperandField, "NOBASE")) //�����ڵ带 �������� �ʴ� ����� ���������� �Ǵ�.
			{//���ڵ带 �����ϱ����� �ӽ� �迭�� �����ڵ��� operater, operand�� �����Ѵ�.
				temp_objectcode[x] = IMRArray[loop]->ObjectCode;
				strcpy(temp_operator[x], IMRArray[loop]->OperatorField);
				strcpy(temp_operand[x], IMRArray[loop]->OperandField);
				last_index = loop+1;
			    x++;
			}
			else ;
			temp_address = IMRArray[loop+1]->Loc;
		}
	
		printf("T%06x%02x", first_address, (IMRArray[last_index]->Loc - IMRArray[first_index]->Loc)); //�ؽ�Ʈ ���ڵ��� ���ۺκ����� �����ּҿ� ���̸� �����
		fprintf(fptr_obj, "T^%06x^%02x", first_address, (IMRArray[last_index]->Loc - IMRArray[first_index]->Loc));//�ؽ�Ʈ ���ڵ��� ���ۺκ����� �����ּҿ� ���̸� �����

		for (xx = 0; xx<x; xx++)//�����ڵ� ��ºκ�
		{
			SearchOptab(temp_operator[xx]);
			inst_Format = (int)(OPTAB[Counter].Format - '0');
			if ((strcmp(temp_operator[xx], "BYTE")==0) && (temp_operand[xx][0]=='X' || temp_operand[xx][0]=='x')){ //byte�̸鼭 hex���ΰ�� 1����Ʈ�� ǥ���ǹǷ� 2�ڸ��� ��µǰ� �Ͽ���/
				printf("%02x", temp_objectcode[xx]);
				fprintf(fptr_obj, "^%02x", temp_objectcode[xx]);
			}
			
			else if (inst_Format == 1){ //1������ ���
				printf("%02x", temp_objectcode[xx]);
				fprintf(fptr_obj, "^%02x", temp_objectcode[xx]);
			}
			else if (inst_Format == 2){//2������ ���
				printf("%04x", temp_objectcode[xx]);
				fprintf(fptr_obj, "^%04x", temp_objectcode[xx]);
			}
			else if (inst_Format == 4){//4������ ��� 4����Ʈ�� ǥ���̵ǹǷ� 8�ڸ��� ��µǰ� �Ͽ���.
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
		

		if (!strcmp(IMRArray[loop]->OperatorField, "END")){ //�������ڵ带 �����ϴ� �κ��̴�.
			for (int i = 0; i < ArrayIndex; i++){
				if (IMRArray[i]->OperatorField[0] == '+'&&IMRArray[i]->OperandField[0]!='#'){//4�����̰� �ǿ������� ���� �ٷ� �ּҰ� �Ǵ� immdiate ����� �ƴ϶�� ���� ���ڵ带 �����Ѵ�.
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
	char label[32]; //�о�帰 ���̺��� ����
	char opcode[32]; //opcode����
	char operand[32]; //operand����

	
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
		is_empty_line = strlen(Buffer); //�о�帰 ���ڿ��� ����.

		Index = 0;
		j = 0;
		strcpy(label,ReadLabel());

        if (Label[0] == '.')//�о�帰���� �ּ����� �Ǵ�
			is_comment = 1;
		else
			is_comment = 0;


		if (is_empty_line>1 && is_comment!=1)//�о�帰 ���ڿ��� �ְ� �ּ��� �ƴѰ��
		{
			Index = 0;
			j = 0;
		
			IMRArray[ArrayIndex] = (IntermediateRec*)malloc(sizeof(IntermediateRec));/* [A] */
			
			IMRArray[ArrayIndex]->LineIndex = ArrayIndex; //�ڽ��� �ε��� ���� �����Ѵ�.
			strcpy(label,ReadLabel());
			strcpy(IMRArray[ArrayIndex]->LabelField,label);//�о�� ���̺��� ��ɹ��� �����ϴ� �迭�� lablefild�� �����Ѵ�.
			SkipSpace(); //������ ����

			if (line == 0) //�ּ����� ������ ù ��° ���� ���
			{
				
				strcpy(opcode,ReadOperator());
				strcpy(IMRArray[ArrayIndex]->OperatorField,opcode);/* [A] */
				if (!strcmp(opcode,"START"))//���̺��� start�� ���
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
				/********************************�ּ� ����*************************************/
				if (strcmp(opcode,"END"))//end�� �ƴ� ���
				{
					if (label[0] != '\0')//���̺��� �ִٸ�
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
					if (SearchOptab(opcode)){//op�ڵ带 �˻�
						instructionFormat=(int)(OPTAB[Counter].Format - '0');
						if (instructionFormat==3){//3�����ΰ�� Ȯ���ؼ� 4������ �ɼ��� �����Ƿ� �˻�
							if (opcode[0] == '+') instructionFormat = 4;//���� +�� ���������� 4�����̹Ƿ� foramt�� 4�� �ٲ�
						}
						LOCCTR[LocctrCounter] = loc + instructionFormat;
					}
					else if (!strcmp(opcode,"WORD"))// WORD�ΰ�� 3����Ʈ�̹Ƿ� �ּҸ� 3��ŭ ������ �����ּҸ� ��Ÿ����.
						LOCCTR[LocctrCounter] = loc + 3;
					else if (!strcmp(opcode,"RESW"))//RESW�ΰ�� WORD�� ũ�Ⱑ 3����Ʈ�̰� ������ ��ŭ�� ���� ���ؼ� ���� �ּҸ� ��Ÿ���� 
						LOCCTR[LocctrCounter] = loc + 3 * StrToDec(operand);
					else if (!strcmp(opcode,"RESB"))//RESB�� ��� BYTE�� ũ��� 1����Ʈ�̹Ƿ� ������ ��ŭ�� ���� ���ؼ� ���� �ּҸ� ������.
						LOCCTR[LocctrCounter] = loc + StrToDec(operand); 
					else if (!strcmp(opcode,"BYTE")) //BYTE�� ��� �ش� OPERAND�� �Ѱ� ����Ʈ�� ���� ����Ѵ��� �����ּҿ� ���� ���� �ּҷ� ��Ÿ����.
						LOCCTR[LocctrCounter] = loc + ComputeLen(operand);
					else if (!strcmp(opcode, "BASE")){ //BASE�� ��� ��ɾ�� �ν������� ���� �ּҸ� �������� �����Ƿ� �����ּҸ� �״�� �����ϰ� �Ѵ�.
						LOCCTR[LocctrCounter] = loc;
					}
					else if (!strcmp(opcode, "NOBASE")){//NOBASE�� ��쿡�� ��ɾ�� �ν������� ���� �ּҸ� �������� �����Ƿ� �����ּҸ� �״�� �����ϰ� �Ѵ�.
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
			LocctrCounter++; //�ּҰ��� ��� �迭 ����Ҷ� �ε���
			ArrayIndex++;  //����� �ε���
			line += 1; //�ټ�
		}
		FoundOnOptab_flag = 0;
		
	}
	program_length = LOCCTR[LocctrCounter-2]- LOCCTR[0]; //���α׷� ����

/********************************** PASS 2 ***********************************/
	printf("Pass 2 Processing...\n");

	unsigned long inst_fmt; //��ü���� �����ڵ带 ���� ����
	unsigned long inst_fmt_opcode; //���� 1����Ʈ�� ���� ����
	unsigned long inst_fmt_mode; //���鿡 �ش��ϴ� ���� ���� ����
	unsigned long inst_fmt_address; //��ǥ�ּ� Ȥ�� ������ ���� ����
	long disp; //������ ������ ����
	int instructionMode_flag,k,digitflag=0; 
	char tempOperand[10];
	char *regi; //�и��� �������� �̸��� �����ϴ� ����
	union floatingPoint prime;

	for (loop = 1; loop<ArrayIndex; loop++){
		/////////�ʱ�ȭ///////////
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
		/*****************************************��� ����**************************************************/
		if (SearchOptab(opcode)){
			inst_fmt_opcode = OPTAB[Counter].ManchineCode;

			switch ((int)(OPTAB[Counter].Format - '0')){ //���Ŀ� ���� ��ɾ� ������ ���̰� �޶�������.
			case 1://1�����ΰ��
				instructionMode_flag = 1;
				break;
			case 2: //2�����ΰ��
				inst_fmt_opcode <<= 8;
				instructionMode_flag = 2;
				break;
			case 3: //3,4�����ΰ��
				if (opcode[0] == '+') {//4�������� �Ǵ��Ͽ� ���İ��� ����
					instructionMode_flag = 4;
					inst_fmt_mode += EXTENDEDADDR;
				
				}
				else instructionMode_flag = 3; //3�����ΰ�� 
				inst_fmt_opcode <<= 16; //3�� 16��Ʈ�� �и� �ش� �ڸ��� ������ 4������ ��쿡�� �ϴ� �о ��ĥ�� 8��Ʈ�� �� �������� ����Ʈ�Ѵ�.

				if (operand[0] == '#') inst_fmt_mode += IMMEDIATE; //imediate����ϰ�� ��尪�� �����ϴ� ������ �ش� ���� ����
				else if (operand[0] == '@') inst_fmt_mode += INDIRECT; //indirect����ϰ�� ��尪�� �����ϴ� ������ �ش��ϴ� ���� ����
				else if (operand[strlen(operand) - 2] == ',' && operand[strlen(operand) - 1] == 'X'){ //index�������
					inst_fmt_mode += INDEXED+SIMPLE;
					operand[strlen(operand) - 2] = '\0';//
				}
				else inst_fmt_mode += SIMPLE;//sic/xe�̹Ƿ� imediate��峪 indirect�� �ƴϸ� �κ�Ʈ�� 1�� ������ ���� ����
			}
			/*******************************************************************************************************/

			if (operand[0] == '#' || operand[0] == '@'){ //imediate�� indirect��� �ϰ�� label�񱳸� ���� ��ȣ�� ������ label�� �ӽ� �迭�� ������.
				for (k = 1; operand[k] != '\0'; k++){
					tempOperand[k - 1] = operand[k];
				}
				tempOperand[k-1] = '\0';
				for (int i = 0; i < k - 1; i++){//�ش� �ǿ����ڰ� ������ ��츦 �Ǵ��Ѵ�.ex) #4096 
					if (!isdigit(tempOperand[i])){
						digitflag = 0;
						break;
					}
					digitflag = 1;
				}

			}
			else strcpy(tempOperand, operand); //�ش� ��尡 �ƴҰ�� �׳� operand�� tempOperand�� �����Ѵ�.


			///////////////////////////////////////////////////-����&��ǥ �ּ� ���-//////////////////////////////////////////////////////////
			if (operand[0] == '#'&&digitflag == 1){ //IMMEDIATE����̸鼭 �ڿ� ���ڰ��� �ٷ� �� ���
				inst_fmt_address = StrToDec(tempOperand);
			}
			else if (instructionMode_flag == 2){ //2������ ���
				regi = strtok(operand, ","); //r1�� r2�� �ִ� ��찡 �����Ƿ� ,�� �������� ����
				if (0 <= StrToHex(regi) <= 16&&!strcmp(opcode,"SVC")){//SVC�� ��� �������� ������ 0~16���� ���ڸ� �����Ƿ� �������� ������ �߰��Ѵ�.
					inst_fmt_address = StrToHex(regi)<<4;
				}
				else{
					for (int i = 0; i < 9; i++)
						if (!strcmp(regi, Registers[i].name))
							inst_fmt_address = Registers[i].number << 4; //r1�� ��� 1����Ʈ�� �ڸ��� ��4��Ʈ�̹Ƿ� �������� 4��ŭ ����Ʈ �Ѵ�.
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
				
				//operand�� �ִ� ���̺��� �ɺ����̺��� ã�� ������ ��ǥ�ּҸ� ����Ͽ� �ּ��ʵ忡 �ش��ϴ� ������ �ִ´�.
				//����� ��带 ��带 ��Ÿ���� ������ �ִ´�.
				for (int search_symtab = 0; search_symtab < SymtabCounter; search_symtab++){

					if (!strcmp(tempOperand, SYMTAB[search_symtab].Label)){ //
						
						if (instructionMode_flag == 3){//3������ ���
							disp = (long)SYMTAB[search_symtab].Address - IMRArray[loop + 1]->Loc;
							inst_fmt_mode += PCRERLATIVE;
							if (2047 < disp && base_flag == 1){//pc�� ������ �Ѱ� base�� ���� �Ǿ��ִٸ� base �ּ� ���� ����� ����Ѵ�.
								disp = (long)SYMTAB[search_symtab].Address - base;
								inst_fmt_mode -= PCRERLATIVE + BASERLATIVE;
								if (0 < disp < 4095){ //���̽� ��� �ּ� ���� ����� ����
									printf("�����ʰ�");
									return 0;
								}
							}
							else if (-20487>disp || disp>2047){ //PC ��� �ּ� ���� ����� ����
								printf("�����ʰ�");
								return 0;
							}
							
						}
						else if(instructionMode_flag==4) disp = (long)SYMTAB[search_symtab].Address;//4������ ���
						inst_fmt_address = disp; //����
						break; //�ش� ���̺��� ã�Ƽ� ���� �־����Ƿ� �� �̻� �ݺ����� �ʰ� ����������.
					}

				}
				
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////OBJECT �ڵ� ����////////////////////////////////////////////////
			if (instructionMode_flag == 1) //1������ ���
				inst_fmt = inst_fmt_opcode;
			else if (instructionMode_flag == 2){ //2������ ���
				inst_fmt = inst_fmt_opcode + inst_fmt_address;
			}
			else if (instructionMode_flag == 4){ //4������ ���
				inst_fmt = (inst_fmt_opcode + inst_fmt_mode) << 8;
				inst_fmt += inst_fmt_address;
				if (disp < (long)0) inst_fmt += 0x100000; //��ǥ�ּҰ� 0���� ���� ��� �� ������ �������� ���� �ڸ��� �ϳ� �پ��� �Ǽ� �ٽ� �����ش�.
			}
			else {//3������ ���
				inst_fmt = inst_fmt_opcode + inst_fmt_mode + inst_fmt_address;
				if (disp<(long)0){ inst_fmt += 0x1000; } //������ 0���� ���� ��� �� ������ �������� ���� �ڸ��� �ϳ� �پ��� �Ǽ� �ٽ� �����ش�.
			}
			////////////////////////////////////////////////////////////////////////////////////////////////
			IMRArray[loop]->ObjectCode = inst_fmt;//��ɿ� �ش��ϴ� OBJECT code�� �ִ´�. 
		}

		//////////////////////////////////////�������� ��� OBJECT�ڵ�//////////////////////////////////////
		else if (!strcmp(opcode, "WORD")){ //word�� ��� 
			strcpy(operand,IMRArray[loop]->OperandField);
			IMRArray[loop]->ObjectCode = StrToDec(operand);
		}
		else if (!strcmp(opcode,"BYTE")){ //byte�� ���
			strcpy(operand,IMRArray[loop]->OperandField);
            IMRArray[loop]->ObjectCode = 0;

			if(operand[0]=='C' || operand[0]=='c' && operand[1]=='\''){ //byte�� ����ɶ� ���ڷ� ����Ǵ� ���
				for (int x = 2; x<=(int)(strlen(operand)-2); x++){
					IMRArray[loop]->ObjectCode=IMRArray[loop]->ObjectCode + (int)operand[x];
					IMRArray[loop]->ObjectCode<<=8;
				}
			}

            if(operand[0]=='X' || operand[0]=='x' && operand[1]=='\''){ //byte�� ����ɶ� 16������ ����Ǵ� ���
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
	
	for (loop = 0; loop<ArrayIndex; loop++) //�����Ҵ��� �޸� ����
        free(IMRArray[loop]);

	printf("Compeleted Assembly\n");
	fclose(fptr);
}