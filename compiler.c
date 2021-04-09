#include <stdio.h>
#include <stdlib.h>

// Macros for character classification.
#include <ctype.h>

#define MAXIDLEN 33;

/* 				SCANNER				*/
typedef enum token_types {
	BEGIN, END, READ, WRITE, ID,
	INTLITERAL, LPAREN, RPAREN, SEMICOLON,
	COMMA, ASSIGNOP, PLUSOP, MINUSOP, SCANEOF
} token;

typedef struct {
	char *token;
	int size;
} buffer;

typedef struct {
	int num;
	int offset;
} line_t;

// Archivo de input
FILE *input;

// Archivo de output
FILE *output;
char *output_name;

// Palabras reservadas
const char *reserved_words[] = {"begin", "end", "read", "write"};

line_t line_info = {1, 0};

token scanner(void) {
	int in_char, c;
	clear_buffer();

	if (feof(input)) {
		return SCANEOF;
	}

	while((in_char = getc(input)) != EOF) {
		// Hay que ignorar espacios vacios
		if (isspace())
	}
}

/*				PARSER				*/

/*				SEMANTICS			*/

/*				ERRORS				*/