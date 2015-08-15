#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "data.h"
#include "symbtable.h"
#include <math.h>


/*Data symbols list.*/
hash_node *data_table[HASHSIZE];
/*Instustructions list.*/
hash_node *instructions_table[HASHSIZE];
/*Externals list.*/
hash_node *extern_table[HASHSIZE];
/*Entry list.*/
hash_node *entry_table[HASHSIZE];
/*Helper variable definitions for file runner.*/
hash_node *opNode[HASHSIZE];/*Table of assembly commands*/
hash_node *tmp_node; /*Temp helper node.*/
hash_node *tmp_node_in;/*Temp helper node*/
boolean error_flag = FALSE, extern_flag = FALSE, entry_flag = FALSE; /*Helper flags*/
int IC = 100;/*The memory starts from 100*/
int DC, entryCount, externCount, i;
word_data data_array[MAX_SIZE]; /*Array  for a data.*/
char * array_entry[MAX_SIZE];/*Array for entry*/
int mis;/*how many times this assembly command must occured. MOV1 or MOV2 etc..*/
char  last_dest_opr[MAX_OP_NAME];/*Remember the last operand for may be $$*/
boolean reg;/*if the operand is register*/
boolean online_num; /*if the operand like #*/

/* Second run*/
int run2(input_line * input, int linenum, char *input_fileName)
{
	char addr_array[NUM_OF_DIGITS+1], instr_array[NUM_OF_DIGITS+2], fileName[FILE_NAME_SIZE];/*TEMP ARRAYS FOR CODE SAVING*/
	int  x, line_count = 0;
	FILE *ob, *ext, *ent;
	/*Files opening*/
	sprintf(fileName, "%s.ob", input_fileName);
	ob = fopen(fileName, "w");
	if (ob == NULL)
	{
		printf("Error! Can't create %s file.\n", fileName);
		return 0;
	}
	sprintf(fileName, "%s.ext", input_fileName);
	ext = fopen(fileName, "w");
	if (ext == NULL)
	{
		printf("Error! Can't create %s file.\n", fileName);
		return 0;
	}
	sprintf(fileName, "%s.ent", input_fileName);
	ent = fopen(fileName, "w");
	if (ent == NULL)
	{
		printf("Error! Can't create %s file.\n", fileName);
		return 0;
	}
	/*Prints how many instructions and data.*/
	fprintf(ob, "\t%s   %s\n", base_to_base_con((IC - START - DC), BASE, addr_array, 0), base_to_base_con(DC, BASE, instr_array, NO_LZ));
	/*line_count++;*/
	for (i = 0; i < linenum; i++)
	{
		if (!input[i].line_compl)
		{
			address_checker(input[i].instruction, input[i].line_number);

			/*Check if a command like MOV1 or MOV2, add1 or add2 etc... */
			x = input[i].how_many;
			while (x != 0)
			{
				/* The first line of of instruction(not operands).*/
				fprintf(ob, "%s\t%s\t\n", base_to_base_con(line_count + LINE_OFFSET, BASE, addr_array, NO_LZ), base_to_base_con(converte(*(input[i].instruction)).data, BASE, instr_array, LZ));
				line_count++;

				/*If two registers are in the instruction. In machine language they must be in ONE line.*/
				if ((input[i].instruction->dest_addr == 3) && (input[i].instruction->src_addr == 3))
				{
					int dest, src;
					int tmp1 = 0;
					int tmp2 = 0;
					int fin;
					int y = MAX_OP_NAME;
					int z = 1;

					if (input[i].src_opr[0] == '$')
					{
						if (check_for_last_oper()){
							while (z != MAX_OP_NAME) /*this loop is converting back the last reg value(in base4) to int*/
							{
								tmp1 += (last_dest_opr[y - z] - '0')*(int)(power(4, z - 1));
								z++;
							}
						}
						else
						{
							ERROR("The $$ can't be used at this line.Because no were operands before this instruction.", input[i].line_number);
							error_flag = TRUE;
						}
					}
					else
					{
						src = (input[i].src_opr[1]) - '0';
						tmp1 = src << 7;
						strcpy(last_dest_opr, base_to_base_con(tmp1, BASE, instr_array, LZ));/*Save this reg for may be next $$*/
					}
					dest = (input[i].dest_opr[1]) - '0';
					tmp2 = dest << 2;
					fin = tmp1 | tmp2;
					fprintf(ob, "%s\t%s\n", base_to_base_con(line_count + LINE_OFFSET, BASE, addr_array, NO_LZ), base_to_base_con(fin, BASE, instr_array, LZ));
					line_count++;
				}
				else
				{
					if (input[i].instruction->gr_code == 2)
					{
						if (input[i].src_opr)
							operand_extractor(input[i].src_opr, input[i].instruction->src_addr, ob, ext, &line_count, input[i].line_number, TRUE);
						if (input[i].dest_opr)/*The last argumet of the funct"TRUE" means that is's source operand(not dest)  */
							operand_extractor(input[i].dest_opr, input[i].instruction->dest_addr, ob, ext, &line_count, input[i].line_number, FALSE);
					}
					if (input[i].instruction->gr_code == 1)
						operand_extractor(input[i].dest_opr, input[i].instruction->dest_addr, ob, ext, &line_count, input[i].line_number, TRUE);

				}
				x--;
			}
		}
	}

	/*Write .ob file*/
	for (i = 0; i < DC; i++)
	{
		fprintf(ob, "%s\t%s\n", base_to_base_con(line_count + LINE_OFFSET, BASE, addr_array, NO_LZ), base_to_base_con(data_array[i].data, BASE, instr_array, LZ));
		line_count++;
	}

	/*Write .ent file*/
	for (i = 0; i<entryCount; i++)
	{

		if ((tmp_node = search_node(array_entry[i], instructions_table)))
			fprintf(ent, "\t%s\t\t%s\n", array_entry[i], base_to_base_con(tmp_node->num, BASE, addr_array, NO_LZ));

		else if ((tmp_node = search_node(array_entry[i], data_table)))
			fprintf(ent, "\t%s\t%s\n", array_entry[i], base_to_base_con(tmp_node->num, BASE, addr_array, NO_LZ));

		else
		{
			fprintf(stderr, "ERROR: Ca't fund address for the entry %s.\n", array_entry[i]);
			error_flag = TRUE;
		}
	}

	/*Close all files*/
	fclose(ob);
	fclose(ext);
	fclose(ent);

	/*Delete all created files in case of failure*/
	if (error_flag)
	{
		sprintf(fileName, "%s.ob", input_fileName);
		remove(fileName);
		sprintf(fileName, "%s.ext", input_fileName);
		remove(fileName);
		sprintf(fileName, "%s.ent", input_fileName);
		remove(fileName);
	}

	/*Tester if no externs provided.*/
	else if (externCount == 0)
	{
		sprintf(fileName, "%s.ext", input_fileName);
		remove(fileName);
	}
	/*Tester if no entrys provided.*/
	else if (entryCount == 0)
	{
		sprintf(fileName, "%s.ent", input_fileName);
		remove(fileName);
	}
	return 0;
}


/*Function to  test the instructions.*/
void address_checker(instruction_line *inst_line, int line_number)
{
	/*For each Code*/
	switch (inst_line->opcode)
	{
	case MOV:
	case ADD:
	case SUB:

		if ((inst_line->dest_addr == 0))/* || (inst_line->dest_addr == 2))*/
		{
			ERROR("MOV,ADD or SUB at this line is invalid.", line_number)
				error_flag = TRUE;
		}
		break;
	case NOT:
	case CLR:
	case INC:
	case DEC:
		if ((inst_line->dest_addr == 0) || (inst_line->dest_addr == 2) || (inst_line->src_addr != 0))
		{
			ERROR("NOT,CLR,INC or DEC at this line is invalid.", line_number);
			error_flag = TRUE;
		}
		break;
	case JMP:
	case BNE:
	case RED:
		if ((inst_line->dest_addr == 0) || (inst_line->src_addr != 0))
		{
			ERROR("The JMP,BNE or RED at this line is invalid.", line_number)
				error_flag = TRUE;
		}
		break;

	case RTS:
	case STOP:
		if ((inst_line->dest_addr != 0) || (inst_line->src_addr != 0))
		{
			ERROR("The STOP or RTS at this line is invalid.", line_number)
				error_flag = TRUE;
		}
		break;

	case LEA:
		if ((inst_line->dest_addr == 0) || (inst_line->dest_addr == 2) || (inst_line->src_addr != 1))
		{
			ERROR("The LEA instruction is invalid.", line_number)
				error_flag = TRUE;
		}
		break;

	case PRN:
		if (inst_line->src_addr != 0)
		{
			ERROR("The PRN instruction is invalid.", line_number)
				error_flag = TRUE;
		}
		break;

	case JSR:
		if ((inst_line->dest_addr != 1) || (inst_line->src_addr != 0))
		{
			ERROR("The JSR instruction is invalid.", line_number)
				error_flag = TRUE;
		}
		break;
	default: break;
	}
}



char *base_to_base_con(int num, int base, char *res, int lead_zero)
{
	int counter = 0, i;
	char temp[NUM_OF_DIGITS + 1];

	for (i = 0; i < NUM_OF_DIGITS; i++)
	{
		res[i] = '0';
		temp[i] = '0';
	}

	while (num != 0)
	{
		temp[counter] = (num % base) + '0';
		num /= base;
		counter++;
	}

	for (i = 0; i < NUM_OF_DIGITS; i++){
		res[i] = temp[NUM_OF_DIGITS - 1 - i];
	}

	res[i] = '\0';/*ADD THE NULL CHAR IN THE END:*/

	if (lead_zero)
		return res;
	else
	{
		return res + NUM_OF_DIGITS - counter;
		printf("\n");
	}
}
/*Helper function for second run for printing data to the suitable files.*/
void operand_extractor(char *opr, int addnum, FILE *ob_fp, FILE *ext_fp, int *line_count, int org_line_num, boolean source)
{
	char base_result[NUM_OF_DIGITS + 1], base_result1[NUM_OF_DIGITS + 1];
	/*char *tmp;*/
	int sign = 1, sum = 0;
	int i = 0;/*for array scan*/
	switch (addnum)
	{
	case 0:
		opr++;
		if (opr[0] == '$')
		{
			if (check_for_last_oper())
			{
				fprintf(ob_fp, "%s\t%s\t\n", base_to_base_con((*line_count) + LINE_OFFSET, BASE, base_result, NO_LZ), last_dest_opr);
				(*line_count)++;
				break;
			}

			else
				ERROR("Can't use $$ operand,if before no were destenition operands.", *line_count);
			break;
		}
		if (opr[0] == '-')
		{
			sign = -1;
			opr++;
		}
		while (isdigit(opr[0]))
		{
			sum *= 10;
			sum += opr[0] - '0';
			opr++;
		}

		/*Add number to the .ob file*/
		fprintf(ob_fp, "%s\t%s\t\n", base_to_base_con((*line_count) + LINE_OFFSET, BASE, base_result, NO_LZ), base_to_base_con(num2data(sum * sign).data, BASE, base_result1, LZ));
		(*line_count)++;
		if (source)
		{
			strcpy(last_dest_opr, (base_to_base_con(num2data(sum * sign).data, BASE, base_result1, LZ)));
		}
		break;
	case 1:
		if ((tmp_node = search_node(opr, data_table)))
		{
			/*Write address of a symbol to the .ob file.*/

			fprintf(ob_fp, "%s\t%s\t\n", base_to_base_con((*line_count) + LINE_OFFSET, BASE, base_result, NO_LZ), base_to_base_con((conv_symb(tmp_node->num)).data, BASE, base_result1, LZ));
			if (source)
			{

				strcpy(last_dest_opr, base_to_base_con((conv_symb(tmp_node->num)).data, BASE, base_result1, LZ));
			}

			(*line_count)++;
		}

		else if ((tmp_node = search_node(opr, instructions_table)))
		{
			/*Write address of a symbol to the .ob file.*/
			fprintf(ob_fp, "%s\t%s\t\n", base_to_base_con((*line_count) + LINE_OFFSET, BASE, base_result, NO_LZ), base_to_base_con(conv_symb(tmp_node->num).data, BASE, base_result1, LZ));
			(*line_count)++;
			if (source)
			{
				strcpy(last_dest_opr, base_to_base_con((conv_symb(tmp_node->num)).data, BASE, base_result1, LZ));
			}
		}
		else if ((tmp_node = search_node(opr, extern_table)))
		{
			fprintf(ob_fp, "%s\t%s\t\n", base_to_base_con((*line_count) + LINE_OFFSET, BASE, base_result, NO_LZ), base_to_base_con(num2data_for_ext(tmp_node->num).data, BASE, base_result1, LZ));
			if (source)
			{
				strcpy(last_dest_opr, base_to_base_con(num2data_for_ext(tmp_node->num).data, BASE, base_result1, LZ));
			}
			/*Write external to the .ext file.*/
			fprintf(ext_fp, "%s\t%s\n", opr, base_to_base_con(*line_count + LINE_OFFSET, BASE, base_result, NO_LZ));
			(*line_count)++;
		}
		else
		{
			ERROR("Cannot find symbol.", org_line_num)
				error_flag = TRUE;
		}
		break;

	case 2:
	{
		boolean last_desten_oper = FALSE;
		for (; i < MAX_OP_NAME; i++)
		{
			/*Test if the array is empty.If it's empty that's mean never has been destenition operand,so $$ can't be here.*/
			if (last_dest_opr[i] != '\0')
			{
				fprintf(ob_fp, "%s\t%s\t\n", base_to_base_con((*line_count) + LINE_OFFSET, BASE, base_result, NO_LZ), last_dest_opr);
				(*line_count)++;
				last_desten_oper = TRUE;
				break;

			}
		}
		if (!last_desten_oper)
		{
			ERROR("The $$ can't be in this command.Upper instruction hasn't dest operand.", org_line_num);
			error_flag = TRUE;
			break;
		}
		break;
	}

	case 3:
	{
		int temp;
		if (source)
		{
			temp = (opr[1]) - '0';
			temp <<= 7;
			fprintf(ob_fp, "%s\t%s\t\n", base_to_base_con((*line_count) + LINE_OFFSET, BASE, base_result, NO_LZ), base_to_base_con(temp, BASE, base_result1, LZ));
			(*line_count)++;
			strcpy(last_dest_opr, base_to_base_con(temp, BASE, base_result1, LZ));
			break;
		}
		else
		{
			temp = (opr[1]) - '0';
			temp <<= 2;
			fprintf(ob_fp, "%s\t%s\t\n", base_to_base_con((*line_count) + LINE_OFFSET, BASE, base_result, NO_LZ), base_to_base_con(temp, BASE, base_result1, LZ));
			(*line_count)++;
			break;
		}
	}

	}


}

/*checks for last operand*/
boolean check_for_last_oper()
{
	int z = 0;
	boolean last_destern_oper = FALSE;
	for (; z <= MAX_OP_NAME; z++)
	{
		/*Test if the array is empty.If it's empty that's mean never before has been destenition operand,so && can't be here.*/
		if (last_dest_opr[z] != '\0')
		{
			last_destern_oper = TRUE;
		}
	}
	if (last_destern_oper)
		return TRUE;
	else
		return FALSE;
}

/*Convert for a symbol table*/
word_data conv_symb(int i)
{
	word_data tmp;
	int z;
	z = ((i * 4) + 2);
	tmp.data = z;
	return tmp;
}

/*a)checkig- If i'ts a symbol (tavit)
b)if it's ".string",".data",".entry",".extern"
c)if it's one of the assembly commands
d)comment?
else error*/
void run1(input_line * input, int linenum)
{
	/*
	//int DC = 0;
	//int externCount = 0;
	//int entryCount = 0;*/
	int i;
	char * symb;
	int tmp_IC = 0;
	int symb_IC = 0;
	for (i = 0; i < linenum; i++)
	{

		symb_IC = IC;
		symb = find_symb(input + i); /*To "decide" if the symbol exists, if does symb= TAVIT*/
		skip_spaces(&(input + i)->line_str);
		if (*((input + i)->line_str) == '.')
		{
			def_design(&input[i], symb);/*define designation.*/
		}
		else
		{
			tmp_IC = def_instruction(&input[i]);
			if (symb)
				tmp_node = add_node(symb, symb_IC, tmp_IC, instructions_table);
		}

	}

}
/*Defined instruction command.
count how many instruction lines should be
Fit the instruction struct at each "input" struct
return number of instruction lines */
int def_instruction(input_line * input_def)
{
	char amount_of_operands;
	char * str_opcode = NULL;
	char * str_operand1 = NULL;/*will help to point to the first operand*/
	char * str_operand2 = NULL;/*will help to point to the second operand*/
	input_def->src_opr = NULL;/*zeroize sorce and destenition operands*/
	input_def->dest_opr = NULL;
	int tempCounter = 0;
	skip_spaces(&(input_def->line_str));
	str_opcode = strcpy(malloc(strlen(input_def->line_str)), input_def->line_str);
	str_opcode = strtok(str_opcode, " \t\n ");
	input_def->src_opr = (char *)malloc(sizeof(input_def->line_str));
	if (!input_def->src_opr)
	{
		ERROR("Can't allocate input_def->src.", input_def->line_number);
		error_flag = TRUE;
		return 0;
	}
	input_def->src_opr = NULL;/*zeroize sorce and destenition operands*/
	/*If comments*/
	if ((!str_opcode) || (str_opcode[0] == ';') || (str_opcode[0] == '\n'))
	{
		input_def->line_compl = TRUE;
		return 0;
	}
	tmp_node_in = find_command(str_opcode);
	if (!tmp_node_in)
	{
		ERROR("Wrong instruction entered.", input_def->line_number);
		error_flag = TRUE;
		input_def->line_compl = TRUE;
		return 0;
	}
	if (mis == 1)
	{
		input_def->how_many = 1;
	}
	else
		input_def->how_many = 2;
	input_def->instruction = (instruction_line *)malloc(sizeof(instruction_line) * 5);
	if (!input_def->instruction)
	{
		ERROR("Can't allocate the memory for this instruction.", input_def->line_number);
		error_flag = TRUE;
		return 0;
	}
	else
	{
		clean_instruct(input_def->instruction);
		input_def->line_compl = FALSE;
		tempCounter++;
		input_def->instruction->opcode = tmp_node_in->num;
		amount_of_operands = how_many_oper(tmp_node_in->name);
		input_def->instruction->gr_code = amount_of_operands;
	}

	while (isalpha(input_def->line_str[0]) || isdigit(input_def->line_str[0]))
	{
		(input_def->line_str)++;
	}
	skip_spaces(&(input_def->line_str));
	str_operand1 = strcpy(malloc(10), input_def->line_str);
	str_operand2 = strcpy(malloc(10), input_def->line_str);
	str_operand1 = strtok(str_operand1, " \t,\n ");

	/*If  TWO operands!*/
	if (amount_of_operands == 2)
	{

		input_def->src_opr = (char *)malloc(sizeof(str_operand1));
		strcpy(input_def->src_opr, str_operand1);
		switch (str_operand1[0])
		{
		case '#':
			tempCounter++;
			input_def->instruction->src_addr = 0;
			reg = FALSE;
			online_num = TRUE;
			break;
		case 'r':
			tempCounter++;
			(str_operand1)++;
			/*Register number checker.*/
			{
				if ((str_operand1[0] >= '0') && (str_operand1[0] <= '7'))
				{
					input_def->instruction->src_addr = 3;
					reg = TRUE;
					online_num = FALSE;
					(str_operand1)++;
				}
				else
				{
					ERROR("It's not legal register number.", (*input_def).line_number)
						error_flag = TRUE;
					reg = FALSE;
					online_num = FALSE;
				}
				break;
			}
		case '$':
			tempCounter++;
			/*reg = FALSE;*/
			(str_operand1)++;
			if (str_operand1[0] == '$')
			{
				if ((reg) || (online_num))
				{
					if (reg)
						input_def->instruction->src_addr = 3;
					else
						input_def->instruction->src_addr = 0;
					break;
				}

				else
					input_def->instruction->src_addr = 2;
				break;
			}
			else
				input_def->instruction->src_addr = 1;
			break;
		default:
			tempCounter++;
			input_def->instruction->src_addr = 1;
			reg = FALSE;
			online_num = FALSE;
			break;
		}
		skip_spaces(&(str_operand2));
		while ((isalpha(str_operand2[0])) || (isdigit(str_operand2[0])) || (str_operand2[0] == '$') || (str_operand2[0] == '#') || (str_operand2[0] == '-'))
		{
			(str_operand2)++;
		}
		skip_spaces(&(str_operand2));
		/*skip_spaces(&(input_def->line_str));*/
		if (str_operand2[0] != ',')
		{
			ERROR("Between two operands must be Comma.", input_def->line_number);
			error_flag = TRUE;
		}
		(str_operand2)++;
		skip_spaces(&(str_operand2));
		str_operand2 = strtok(str_operand2, " \t,\n ");
		if (!str_operand2)
		{
			ERROR("This opcode must be with 2 operands", input_def->line_number);
			error_flag = TRUE;
			return 0;
		}
		input_def->dest_opr = (char *)malloc(sizeof(str_operand2));
		strcpy(input_def->dest_opr, str_operand2);
		switch (str_operand2[0])
		{
		case '#':
			tempCounter++;
			input_def->instruction->dest_addr = 0;
			break;
		case 'r':
			(str_operand2)++;
			/*Register number checker.*/
			if ((str_operand2[0] >= '0') && (str_operand2[0] <= '7'))
			{

				input_def->instruction->dest_addr = 3;
			}
			else
			{
				ERROR("IT'S NOT A LEGAL REGISTER NUMBER", (*input_def).line_number)
					error_flag = TRUE;
				break;
			}
			if ((input_def->instruction->src_addr == 2) && (reg))
			{
				break;
			}
			if (input_def->instruction->src_addr == 3)
			{
				break;
			}
			else tempCounter++;
			break;

		case '$':
			/*	tempCounter++;*/
			(str_operand2)++;
			if (str_operand2[0] == '$')
			{
				input_def->instruction->dest_addr = 2;
			}
			else
				input_def->instruction->dest_addr = 1;
			break;
		default:
			input_def->instruction->dest_addr = 1;
			tempCounter++;
			/*while (input_def->line_str != 0 && *(input_def->line_str) != ' '&&*(input_def->line_str++) != ',') {}*/
			break;
		}
	}

	/* If only ONE operand!*/
	else
		if (amount_of_operands == 1)
		{
			input_def->src_opr = 0;
			input_def->instruction->src_addr = 0;
			input_def->dest_opr = (char *)malloc(sizeof(str_operand1));
			strcpy(input_def->dest_opr, str_operand1);
			skip_spaces(&(input_def->line_str));
			switch (input_def->line_str[0])
			{
			case '#':
				tempCounter++;
				input_def->instruction->dest_addr = 0;
				reg = FALSE;
				online_num = TRUE;
				break;
			case 'r':
				tempCounter++;
				(input_def->line_str)++;
				/*Register number checker.*/
				{
					if ((input_def->line_str[0] >= '0') && (input_def->line_str[0] <= '7'))
					{
						input_def->instruction->dest_addr = 3;
						reg = TRUE;
						online_num = FALSE;
					}
					else
					{
						ERROR("It's not legal register number.", (*input_def).line_number)
							error_flag = TRUE;
					}
					break;
				}
			case '$':
				tempCounter++;
				reg = FALSE;
				online_num = FALSE;
				(input_def->line_str)++;
				if (input_def->line_str[0] == '$')
				{
					input_def->instruction->dest_addr = 2;
				}
				else
					input_def->instruction->dest_addr = 1;
				break;
			default:
				tempCounter++;
				reg = FALSE;
				online_num = FALSE;
				input_def->instruction->dest_addr = 1;
				break;
				/*" ,.-!?()\t\n\'0'"*/
			}

		}

	if (mis == 2)
	{
		tempCounter *= 2;
	}
	IC += tempCounter;
	return tempCounter;

}

/*Finds how many operands per opcode*/
char how_many_oper(char * opc)
{
	if (strcmp(opc, "rts") == 0 || strcmp(opc, "stop") == 0)
		return 0;

	else if (strcmp(opc, "mov") == 0 || strcmp(opc, "cmp") == 0 || strcmp(opc, "add") == 0 || strcmp(opc, "sub") == 0 || strcmp(opc, "lea") == 0)
		return 2;

	/*not,clr,inc,dec,jmp,bne,red,prn,jsr second group.*/
	else
		return 1;
}


/*Function helps to find an assembly command*/
hash_node * find_command(char * s)
{
	mis = 0;
	hash_node * temp_node;
	if (!strcmp(s, "mov1") || !strcmp(s, "mov2"))
	{
		if (!strcmp(s, "mov1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("mov", opNode);
		return temp_node;
	}

	else if (!strcmp(s, "cmp1") || !strcmp(s, "cmp2"))
	{
		if (!strcmp(s, "cmp1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("cmp", opNode);
		return temp_node;
	}

	else if (!strcmp(s, "add1") || !strcmp(s, "add2"))
	{
		if (!strcmp(s, "add1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("add", opNode);
		return temp_node;
	}

	else if (!strcmp(s, "add1") || !strcmp(s, "add2"))
	{
		if (!strcmp(s, "add1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("add", opNode);
		return temp_node;
	}
	else if (!strcmp(s, "sub1") || !strcmp(s, "sub2"))
	{
		if (!strcmp(s, "sub1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("sub", opNode);
		return temp_node;
	}
	else if (!strcmp(s, "not1") || !strcmp(s, "not2"))
	{
		if (!strcmp(s, "not1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("not", opNode);
		return temp_node;
	}
	else if (!strcmp(s, "clr1") || !strcmp(s, "clr2"))
	{
		if (!strcmp(s, "clr1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("clr", opNode);
		return temp_node;
	}
	else if (!strcmp(s, "lea1") || !strcmp(s, "lea2"))
	{
		if (!strcmp(s, "lea1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("lea", opNode);
		return temp_node;
	}
	else if (!strcmp(s, "inc1") || !strcmp(s, "inc2"))
	{
		if (!strcmp(s, "inc1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("inc", opNode);
		return temp_node;
	}
	else if (!strcmp(s, "dec1") || !strcmp(s, "dec2"))
	{
		if (!strcmp(s, "dec1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("dec", opNode);
		return temp_node;
	}
	else if (!strcmp(s, "jmp1") || !strcmp(s, "jmp2"))
	{
		if (!strcmp(s, "jmp1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("jmp", opNode);
		return temp_node;
	}
	else if (!strcmp(s, "bne1") || !strcmp(s, "bne2"))
	{
		if (!strcmp(s, "bne1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("bne", opNode);
		return temp_node;
	}
	else if (!strcmp(s, "red1") || !strcmp(s, "red2"))
	{
		if (!strcmp(s, "red1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("red", opNode);
		return temp_node;
	}
	else if (!strcmp(s, "prn1") || !strcmp(s, "prn2"))
	{
		if (!strcmp(s, "prn1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("prn", opNode);
		return temp_node;
	}
	else if (!strcmp(s, "jsr1") || !strcmp(s, "jsr2"))
	{
		if (!strcmp(s, "jsr1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("jsr", opNode);
		return temp_node;
	}
	else if (!strcmp(s, "rts1") || !strcmp(s, "rts2"))
	{
		if (!strcmp(s, "rts1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("rts", opNode);
		return temp_node;
	}
	else if (!strcmp(s, "stop1") || !strcmp(s, "stop2"))
	{
		if (!strcmp(s, "stop1"))
		{
			mis = 1;
		}
		else
			mis = 2;
		temp_node = search_node("stop", opNode);
		return temp_node;
	}
	else return NULL;

}

/*Function to define the designation.: ".data",".string",".entry" or ".extern".*/
void def_design(input_line * input, char * symb)
{
	if (!(strncmp(input->line_str, ".data", strlen(".data"))))/*If data.*/
	{
		int memd;
		input->line_str += strlen(".data");
		memd = data_treat(input);
		if (symb)
			tmp_node = add_node(symb, IC, memd, data_table);
		IC += memd;
	}

	if (!(strncmp(input->line_str, ".string", strlen(".string")))) /*If string.*/
	{
		int mems = 0;
		input->line_str += strlen(".string");
		mems = string_treat(input);
		if (symb)
		{
			tmp_node = add_node(symb, IC, mems, data_table);
		}
		IC += mems;
	}
	if (!(strncmp(input->line_str, ".entry", strlen(".entry"))))/*If .entry.*/
	{
		input->line_str += strlen(".entry");
		entry_treat(input);
		return;
	}
	if (!(strncmp(input->line_str, ".extern", strlen(".extern"))))/*If .extern.*/
	{
		input->line_str += strlen(".extern");
		extern_treat(input);
	}

}

/*Treatment for an Extern*/
void extern_treat(input_line * input)
{
	skip_spaces(&(input->line_str));
	input->line_str = strtok(input->line_str, " ,.-!?()\t\n\'0'");/*because we need only one word*/
	if (!(input->line_str))
	{
		ERROR("Must be symbol after \".extern\".", input->line_number);
		error_flag = TRUE;
		return;
	}

	if (!isalpha(input->line_str[0]))
	{
		ERROR("The symbol after \".extern\" must start by char!", input->line_number)
			error_flag = TRUE;
		return;
	}

	if (!((tmp_node = search_node(input->line_str, opNode)) == NULL))
	{
		ERROR("The symbol after \".extern\" can't be like assembly command.", input->line_number)
			error_flag = TRUE;
		return;
	}

	add_node(input->line_str, 1, 0, extern_table);
	externCount++;
	skip_spaces(&(input->line_str));
	input->line_str = strtok(NULL, " ,.-!?()\t\n\'0'");

	if (input->line_str)
	{
		ERROR("The \".extern\" must have only ONE symbol!", input->line_number);
		return;
	}

}


/*Treatment for an Entry*/
void entry_treat(input_line * input)
{
	skip_spaces(&(input->line_str));
	input->line_str = strtok(input->line_str, " ,.-!?()\t\n\'0'");/*because we need only one word*/
	if (!(input->line_str))
	{
		ERROR("Must be symbol after \".entry\".", input->line_number);
		error_flag = TRUE;
		return;
	}
	if (!isalpha(input->line_str[0]))
	{
		ERROR("The symbol after \".entry\" must start by char!", input->line_number)
			error_flag = TRUE;
		return;
	}
	if (!((tmp_node = search_node(input->line_str, opNode)) == NULL))
	{
		ERROR("The symbol after \".entry\" can't be like assembly command.", input->line_number)
			error_flag = TRUE;
		return;
	}

	else
		/*add_node(input->line_str, IC, 0, extern_table);*/
		array_entry[entryCount++] = duplicate_str(input->line_str);
	skip_spaces(&(input->line_str));
	input->line_str = strtok(NULL, " ,.-!?()\t\n\'0'");

	if (input->line_str)
	{
		ERROR("The \".entry\" must have only ONE symbol!", input->line_number);
		return;
	}

}


/*Treatment for a String designation*/
int string_treat(input_line * input)
{
	int mems = 0;
	skip_spaces(&(input->line_str));
	(input->line_str)++; /*Skip the /" */
	while (((input->line_str[0]) != '\0') && ((input->line_str[0]) != '\"'))
	{

		skip_spaces(&(input->line_str));
		data_array[DC++] = char2data(input->line_str[0]);
		(input->line_str)++;
		mems++;
	}
	data_array[DC++] = char2data('\0');
	return (mems + 1);
}

/*Treatmnet for a Data designation.Return amount of numbers.*/
int data_treat(input_line * input)
{
	int mem = 0;
	char expecting_number = 0;
	while ((input->line_str[0] != '\0') && ((input->line_str[0]) != '\n'))
	{
		skip_spaces(&(input->line_str));
		int sum = 0;
		int sign = 1;
		if (input->line_str[0] == '-')
		{
			sign = -1;
			(input->line_str)++;
		}
		if (!isdigit(input->line_str[0]))
		{
			ERROR("Wrong format! In .DATA must be a numbers", input->line_number)
				error_flag = TRUE;
			return 0;
		}
		else
		{

			while (isdigit(input->line_str[0]))
			{
				sum *= 10;
				sum += ((input->line_str[0]) - '0');
				(input->line_str)++;
			}
			data_array[DC++] = num2data_for_data(sum*sign);
			(input->line_str)++;
			mem++;

		}

		skip_spaces(&(input->line_str));
		if (input->line_str[0] == ',')
		{
			expecting_number = 1;
			(input->line_str)++;
		}
		else
			expecting_number = 0;
	}
	if (expecting_number == 1)
	{
		ERROR("Expecting for number after ',' sign", input->line_number)
			error_flag = TRUE;
		return 0;
	}
	return mem;
}

/*If symbol exists , the function returns the pointer to this symbol (TAVIT)*/
char * find_symb(input_line * input)
{
	/*hash_node  * temp_node; */
	char * start = (input->line_str);
	skip_spaces(&start);
	while (*(input->line_str))
	{
		if (*(input->line_str) == ':')
		{
			*((input->line_str)++) = '\0';        /*for a reason to save only the name of the symbol*/
			if (!isalpha(start[0]))
			{
				ERROR("The symbol must start by char.", input->line_number)
					error_flag = TRUE;
				return NULL;
			}
			if (!((tmp_node = search_node(start, opNode)) == NULL))
			{
				ERROR("The symbol can't be like assembly command.", input->line_number)
					error_flag = TRUE;
				return NULL;
			}
			return start;
		}
		(input->line_str)++;
	}
	input->line_str = start;

	return NULL;
}
void instal_com_table()
{
	tmp_node = add_node("mov", MOV, 0, opNode);
	tmp_node = add_node("cmp", CMP, 0, opNode);
	tmp_node = add_node("add", ADD, 0, opNode);
	tmp_node = add_node("sub", SUB, 0, opNode);
	tmp_node = add_node("not", NOT, 0, opNode);
	tmp_node = add_node("clr", CLR, 0, opNode);
	tmp_node = add_node("lea", LEA, 0, opNode);
	tmp_node = add_node("inc", INC, 0, opNode);
	tmp_node = add_node("dec", DEC, 0, opNode);
	tmp_node = add_node("jmp", JMP, 0, opNode);
	tmp_node = add_node("bne", BNE, 0, opNode);
	tmp_node = add_node("red", RED, 0, opNode);
	tmp_node = add_node("prn", PRN, 0, opNode);
	tmp_node = add_node("jsr", JSR, 0, opNode);
	tmp_node = add_node("rts", RTS, 0, opNode);
	tmp_node = add_node("stop", STOP, 0, opNode);
}
int power(int x, int y)
{
	int z = 1;
	int i = 0;
	for (; i < y; i++)
	{
		z*= x;
	}
	return z;
}
