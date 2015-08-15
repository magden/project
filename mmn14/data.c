/*DATA.C*/
#include<stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "data.h"

/*Masks for data shifting.*/
unsigned char masks_def[] = { 0x0, 0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F };

char *duplicate_str(const char *str)
{
	int size = strlen(str) + 1;
	char *nString = (char*)malloc(size);

	if (!nString)
	{
		printf("Error occured: memory failed to be allocated (func-data.c->duplicate_str)\n");
		return NULL;
	}
	else
		strcpy(nString, str);

	return nString;
}
/*Skip the spaces at the line.*/
void skip_spaces(char **str)
{
	while (((**str) == ' ' || (**str) == '\t') && (**str != '\n'))
		(*str)++;
}


/*==========Helper functions=============*/
/*Convert num to data*/
word_data num2data_for_data(int num)
{
	word_data temp;
	temp.data = 0;
	temp.data = num&bits_mask_no_era;
	return temp;
}
word_data num2data(int num)
{
	word_data temp;

	temp.data = 0;
	if (num >= 0)
	{
		num <<= 2;
		temp.data = num & bits_mask;
	}
	else
	{
		num <<= 2;
		temp.data = num&bits_mask;
		/*temp.data = (num * -1) & bits_mask;
		temp.data = ~(temp.data);
		(temp.data)++;*/
	}
	return temp;
}
/*convert num for extern*/
word_data num2data_for_ext(int num)
{
	word_data temp;
	temp.data = 0;
	temp.data = num&bits_mask_no_era;
	return temp;
}

/*Convert char to data*/
word_data char2data(char c)
{
	word_data temp;
	temp.data = 0;
	temp.data = c;
	return temp;
}

/*Clean the instruction at the node*/
void clean_instruct(instruction_line * point)
{
	point->era = 0;
	point->dest_addr = 0;
	point->src_addr = 0;
	point->opcode = 0;
	point->gr_code = 0;
}
/*Function creats binary code.*/
word_data converte(instruction_line inst_line)
{
	word_data tmp;
	tmp.data = 0;
	tmp.data |= (masks_def[ERA_WIDTH] << ERA_OFFSET)  & (inst_line.era << ERA_OFFSET);
	tmp.data |= (masks_def[DADDR_WIDTH] << DEST_OFFSET)   & (inst_line.dest_addr << DEST_OFFSET);
	tmp.data |= (masks_def[SADDR_WIDTH] << SORC_OFFSET)  & (inst_line.src_addr << SORC_OFFSET);
	tmp.data |= (masks_def[OPCODE_WIDTH] << OPCODE_OFFSET) & (inst_line.opcode << OPCODE_OFFSET);
	tmp.data |= (masks_def[GROUP_CODE] << GROUP_OFFSET)&(inst_line.gr_code << GROUP_OFFSET);
	return tmp;
}

/*Helps to  follow the data in the data array */
static word_data * point_data;




