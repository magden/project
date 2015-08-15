#ifndef SYMB_TABLE_H
#define HASHSIZE 200 
/*Struct for the node of symbtable*/
typedef struct hash_node * hashp;
typedef struct int_list
{
	struct int_list * next;
	struct int_list * previous;
	char *name;
	int num;
	int mem_words;
} hash_node;
/*Create new node for the symbtable list.*/
hash_node *add_node(char *name, int defn, int mem_w, hash_node *hashtab[]);
/*Search node.*/
hash_node *search_node(char *s, hash_node *hashtab[]);
/*Function helps to find an assembly command*/
hash_node * find_command(char *);
/*Function to duplicating the string and returning address with str.*/
char *duplicate_str(const char *);

#endif 
