#ifndef data_H
#define FILE_NAME_SIZE  25
#define MAX_OP_NAME 6
#define BUFFER 1000
#define MAX_SIZE 1000
#define MAX_LINE_LEN 80
#define NO_LZ 0
#define LZ 1
#define LINE_OFFSET 100
#define START 100
typedef enum { FALSE, TRUE } boolean;
/*Macro for an error */
#define ERROR(mes,line) fprintf(stderr, "Error occured at the line: %d . %s\n", line, mes);

/*required base for a project*/
#define BASE 4
#define NUM_OF_DIGITS 6/*max number required*/
/*for shifting*/
#define ERA_OFFSET  0
#define DEST_OFFSET 2
#define SORC_OFFSET 4
#define OPCODE_OFFSET 6
#define GROUP_OFFSET 10
/*==========*/
#define ERA_WIDTH    2 /*BITS 0-1*/ 
#define DADDR_WIDTH  2/*BITS 2-3 destination addr*/
#define SADDR_WIDTH  2/*BITS 4-5  SOURCE ADDR*/
#define OPCODE_WIDTH 4/*BITS 6-9 */
#define GROUP_CODE   2/*BITS 10-11*/
/*====*/
#define bits_mask  0x1FFC;/*12 bits and first two '0' bits for an "ERA"*/
#define bits_mask_no_era 0x1FFF/*12 bits and doesn't matter ERA*/
/*Bitfieled code line struct.*/
typedef struct bit_code_line {
	unsigned int era : ERA_WIDTH;   /*bits 0-1*/
	unsigned int dest_addr : DADDR_WIDTH; /*bits 2-3*/
	unsigned int src_addr : SADDR_WIDTH; /*bits 4-5*/
	unsigned int opcode : OPCODE_WIDTH;/*bits 6-9*/
	unsigned int gr_code : GROUP_CODE; /*bits 10-11*/
} instruction_line;

/*Full line struct.*/
typedef struct completed_line
{
	char *line_str;
	instruction_line *instruction;
	int how_many;  /*mov1 or mov2*/
	unsigned int line_number;
	char *src_opr;
	char *dest_opr;
	char line_compl;
} input_line;

/*============*/
#define MOV 0
#define CMP 1
#define ADD 2
#define SUB 3
#define NOT 4
#define CLR 5
#define LEA 6
#define INC 7
#define DEC 8
#define JMP 9
#define BNE 10
#define RED 11
#define PRN 12
#define JSR 13
#define RTS 14
#define STOP 15

/*Struct for a binary word-12 bits*/
typedef struct word
{
	unsigned int data : 12;
} word_data;

/*==========Declaration of the functions===============*/
/*Function to conert a number to another base*/
char *base_to_base_con(int, int, char *, int);
/*First run*/
void run1(input_line *, int);
/*Search for a symbol it the line*/
char * find_symb(input_line *);
/*Function to define the designation.: ".data",".string",".entry" or ".extern".*/
void def_design(input_line *, char *);
/*Treatmnet for a Data designation*/
int data_treat(input_line *);
/*Function puts num to data*/
word_data num2data(int);
/*Skip spaces*/
void skip_spaces(char **);
/*Treatment for a String designation*/
int string_treat(input_line *);
/*Function puts char to data*/
word_data char2data(char);
/*Treatment for an Entry*/
void entry_treat(input_line *);
/*Treatment for an Extern*/
void extern_treat(input_line *);
/*Defined instruction command.*/
int def_instruction(input_line *);
/*Finds how many operands per opcode*/
char how_many_oper(char *);
/*Clean the instruction at the node*/
void clean_instruct(instruction_line *);
/*Function to duplicating the string and returning address with str.*/
char *duplicate_str(const char *);
/*Install the assembly commands to table.*/
void instal_com_table();
/*Function to take care of the legal and iligal addresses.*/
void address_checker(instruction_line *, int);
/*Function creats binary code.*/
word_data converte(instruction_line);
/*Test for the opernads in asssembly command.*/
void checking2(input_line *);
/* Second run*/
int run2(input_line *, int, char *);
/*Function to rememeber the last operands*/
void insert_to_static(char*, char*, int, int);
/*Helper function for second run for printing data to the suitable files.*/
void operand_extractor(char *, int, FILE *, FILE *, int *, int, boolean);
/*Convert for a symbol table*/
word_data conv_symb(int);
/*Convert num to data*/
word_data num2data_for_data(int);
/*checks for last operand*/
boolean check_for_last_oper(void);
/*convert num for extern*/
word_data num2data_for_ext(int);
/*Function for power.*/
int power(int, int);

#endif



