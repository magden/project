
#include <stdio.h>
#include <stdlib.h>
#include "symbtable.h"
#include "data.h"

int main(int argc, char **argv){
	int arg_num;
	char fileName[FILE_NAME_SIZE];
	FILE *fp;

	/*Run all provided arguments and execute the assembly.*/
	for (arg_num = 1; arg_num < argc; arg_num++)
	{
		int lineNum, buf = BUFFER;
		input_line *file_lines, *temp;                  /*input_line= struct for every line in input file*/
		sprintf(fileName, "%s.as", argv[arg_num]);
		/*Open file for reading.*/
		fp = fopen(fileName, "r");
		/*Test for a file appeareance.*/
		if (fp == NULL)
		{
			ERROR("This file is missing.\n", 0);
			return 0;
		}
		else
			printf("Just opened.\n");

		file_lines = (input_line*)malloc(sizeof(input_line) * BUFFER);
		lineNum = 0;
		file_lines[lineNum].line_str = (char*)malloc(sizeof(char) * MAX_LINE_LEN);


		/*  Read each line from the provided file and put into array.*/

		while (fgets(file_lines[lineNum].line_str, MAX_LINE_LEN, fp))
		{

			/*If more memmory needed.	*/
			if (lineNum == buf)
			{
				buf += BUFFER;
				temp = (input_line*)realloc(file_lines, sizeof(input_line) * buf);

				if (temp)
					file_lines = temp;

				else
				{
					ERROR("The memory cannot be relocated hence closing.",0);
					return 0;
				}
			}
			file_lines[lineNum].line_number = lineNum + 1;
			file_lines[++lineNum].line_str = malloc(sizeof(char) * MAX_LINE_LEN);
		}
		free(file_lines[lineNum].line_str);/* Free the malloc mem after [++lineNum], 'cuz it's needless line*/

		/*At this moment all the input assembly program is at an array file_lines[lineNum]
		And now will perform	2 runs for read the as file, and convert to machine language */
		instal_com_table(); /*build symbol table with basical assembly commands */
		run1(file_lines, lineNum);
		run2(file_lines, lineNum, argv[arg_num]);

		free(file_lines);

		fclose(fp);


	}
	return 0;
}
