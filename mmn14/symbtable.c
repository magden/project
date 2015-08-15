#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symbtable.h"
#include "data.h"

/*Hash code generator. (from the book)*/
unsigned int hash_code_gen(char *s)
{
	unsigned int hashnum;

	for (hashnum = 0; *s != '\0'; s++)
		hashnum = *s + 31 * hashnum;

	return hashnum % HASHSIZE;
}

/*Helper function for searching node in the hashtable.*/
hash_node *search_node(char *s, hash_node *hashtab[])
{
	hash_node * tmp_node;
/*	hashp ptmp;*/
	for (tmp_node = hashtab[hash_code_gen(s)]; tmp_node != NULL; tmp_node = tmp_node->next)
		if (strcmp(s, tmp_node->name) == 0)
			return tmp_node;

	return NULL;
}

/*Helper function to add new node to the hashtable struct.*/
hash_node *add_node(char *s, int defn, int num, hash_node *hash_table[])
{
	hash_node *new_node;
	/*hashp pnew_node;*/
	unsigned int hashval;

	if ((new_node = search_node(s, hash_table)) == NULL)
	{
		new_node = (hash_node *)malloc(sizeof(* new_node));

		if (new_node == NULL || (new_node->name = duplicate_str(s)) == NULL)
			return NULL;

		else
		{
			hashval = hash_code_gen(s);
			new_node->next = hash_table[hashval];
			hash_table[hashval] = new_node;
		}
	}

	new_node->num = defn;
	new_node->mem_words = num;

	/*Returns pointer to a new created node.*/
	return new_node;
}

/*Clean the node.*/
void clean_node(hash_node *tmp_node_in)
{
	tmp_node_in->mem_words = 0;
	tmp_node_in->name = 0;
	tmp_node_in->next = 0;
	tmp_node_in->num = 0;
	tmp_node_in->previous = 0;
}
