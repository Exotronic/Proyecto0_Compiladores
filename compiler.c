#include <stdio.h>
#include <stdlib.h>

// Macros for character classification.
#include <ctype.h>

#define MAXIDLEN 33;

/* 		================		SCANNER			================	*/
typedef enum token_types {
	BEGIN, END, READ, WRITE, ID,
	INTLITERAL, LPAREN, RPAREN, SEMICOLON,
	COMMA, ASSIGNOP, PLUSOP, MINUSOP, SCANEOF
} token;

// Esto funciona como un arraylist de char
typedef struct {
	char *token;
	int size;
} buffer;

typedef struct {
	int num;		//Numero de linea
	int offset;		//Caracter de linea
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
		if (isspace(in_char)) {
			if (in_char == '\n') {
				line_info.num++
				line_info.offset = ftell(input);
			}
			continue;
		}

		// Hay que identificar ID y las palabras reservadas
		else if (isalpha(in_char)) {
			buffer_char(in_char);

			for(c = getc(input); isalnum(c) || c == '_'; c = getc(input)) {
				buffer_char(c);
			}

			ungetc(c, input);
			return check_reserved();
		}

		// Identificar caracteres numericos
		else if (isdigit(in_char)) {
			buffer_char(in_char);

			for (c = getc(input); isdigit(c); c = getc(input)) {
				buffer_char(c);
			}

			ungetc(c, input);
			return INTLITERAL;
		}

		// Identificar '('
		else if (in_char == '(') {
			buffer_char(in_char);
			return LPAREN;
		}

		// Identificar ')'
		else if (in_char == ')') {
			buffer_char(in_char);
			return RPAREN;
		}

		// Identificar ';'
		else if (in_char == ';') {
			buffer_char(in_char);
			return SEMICOLON;
		}

		// Identificar ','
		else if (in_char == ',') {
			buffer_char(in_char);
			return COMMA;
		}

		// Identificar '+'
		else if (in_char == '+') {
			buffer_char(in_char);
			return PLUSOP;
		}

		// Identificar ':='
		else if (in_char == ':') {
			buffer_char(in_char);
			//buscar ':='

			c = getc(input);
			if (c == '=') {
				buffer_char(c);
				return ASSIGNOP;
			} else {
				ungetc(c, input);
				lexical_error();
			}
		}

		//Identificar '-' o '--'
		else if (in_char == '-') {
			// Buscar '--', es un comentario.
			c = getc(input);

			if (c == '-') {
				do { in_char = getc(input) } while (in_char != '\n')
			} else {
				buffer_char(in_char);
				ungetc(c, input);
				return MINUSOP;
			}
		} 

		// No es un token valido. Error lexico.
		else {
			lexical_error();
		}
	}
}

// Esta funcion prepara el token_buffer
void buffer_char(char c) {
	if (token_buffer.token != NULL) {
		token_buffer.size++;
		token_buffer.token = (char *)realloc(token_buffer.token, token_buffer.size * sizeof(char));
	} else {
		token_buffer = (char *)calloc(token_buffer.size, sizeof(char));
		token_buffer.size = 1;
	}
	token_buffer.token[token_buffer.size - 1] = c;
}

// Limpiar el buffer
void clear_buffer(void) {
	free(token_buffer.token);
	token_buffer.token = NULL;
}

// Esta funcion revisa si es una palabra reservada o un ID.
token check_reserved(void) {
	for (int i = 0; i < 4; ++i) {
		if (!strcmp(reserved_words[i], token_buffer.token)) {
			switch (i) {
				case 0:
					return BEGIN;
					break;
				case 1:
					return END;
					break;
				case 2:
					return READ;
					break;
				case 3:
					return WRITE;
					break;
			}
		}
	}

	return ID;
}

/*		================		PARSER		================		*/
void system_goal(void) {
	/* <system goal> ::= <program> SCANEOF */
	program();
	match(SCANEOF);
}

void program(void) {
	/* <program> ::= BEGIN <statement list> END */
	start();

	match(BEGIN);
	statement_list();
	match(END);
}

void statement_list(void) {
	/*
	 *	<statement list> ::= <statement> { <statement> }
	 */
	statement();
	while (true) {
		switch (next_token()) {
			case ID:
			case READ:
			case WRITE:
				statement();
				break;
			default:
				return;
		}
	}
}

// Verifica instrucciones.
void statement(void) {
	token tok = next_token();

	switch (tok) {
		case ID:
			// <statement> ::= ID := <expression> ;
			match(ID);
			expr_rec id = process_id(token_buffer.token);
			match(ASSIGNOP);
			expression(&id, 0);
			match(SEMICOLON);
			break;
		case READ:
			// <statement> ::= READ ( <id list> ) ;
			match(READ);
			match(LPAREN);
			id_list();
			match(RPAREN);
			match(SEMICOLON);
			break;
		case WRITE:
			// <statement> ::= WRITE ( <expr list> ) ;
			match(WRITE);
			match(LPAREN);
			expr_list();
			match(RPAREN);
			match(SEMICOLON);
			break;
		default:
			syntax_error();
			break;
	}
}

// Verifica la lista de id para "read".
void id_list(void) {
	// <id list> ::= ID { , ID }
	match(ID);

	if(!lookup(token_buffer.token)) {
		semantic_error();
	}

	expr_rec id = process_id(token_buffer.token);
	read_id(id);

	while (next_token() == COMMA) {
		match(COMMA);
		match(ID);

		if (!lookup(token_buffer.token)) {
			semantic_error();
		}

		id = process_id(token_buffer.token);
		read_id(id);
	}
}

// Expresiones
void expression(expr_rec *target) {
	/*
	 *	<expression> ::= <primary> { <add op> <primary> }
	 */

	token t;
	int print_flag = 0;

	// Si el target es NULL es porque se tiene que imprimir.
	if (target == NULL) {
		char *tmp = get_temp();
		expr_rec tmp_expr = process_temp(tmp);

		target = &tmp_expr

		print_flag = 1;
	}

	expr_rec op1 = primary(*target);
	for (t = next_token(); t == PLUSOP || t == MINUSOP; t = next_token()) {
		op_rec op = add_op();
		expr_rec op2 = primary(*target);
		op1 = gen_infix(op1, op, op2);
	}

	if (op1.kind == IDEXPR) {
		char *tmp = get_temp();
		expr_rec tmp_expr = process_temp(tmp);
		assign(tmp_expr, op1);
		*target = tmp_expr;
	}

	else {
		assign(*target, op1);
	}

	if (print_flag) {
		write_expr(*target);
	}
}

// Verifica la lista de expresiones.
void expr_list(void) {
	// <expr list> ::= <expression> { , <expression> }
	expression(NULL);

	while (next_token() == COMMA) {
		match(COMMA);
		expression(NULL);
	}
}

// Obtiene la operacion de una expression.
op_rec add_op(void) {
	token tok = next_token();
	op_rec op;
	// <addop> ::= PLUSOP | MINUSOP
	if (tok == PLUSOP || tok == MINUSOP) {
		match(tok);
		op = process_op(token_buffer.token);
		return op;
	} else {
		syntax_error();
	}
	return op;
}

// Procesamiento de las expressions.
expr_rec primary(expr_rec target) {
	token tok = next_token();
	expr_rec src;
	switch (tok) {
		case LPAREN:
			// <primary> ::= ( <expression> )
			match(LPAREN);
			expression(&target);
			src = target;
			match(RPAREN);
			break;

		case ID:
			// <primary> ::= ID
			if (!lookup(token_buffer.token)) {
				semantic_error();
			}
			match(ID);
			src = process_id(token_buffer.token);
			break;

		case INTLITERAL:
			// <primary> ::= INTLITERAL
			match(INTLITERAL);
			src = process_literal(token_buffer.token);
			break;

		default:
			syntax_error();
			break;
	}

	return src;
}

// Verificar que el token gramatico es igual al del programa.
void match(token t) {
	if(temp_token == NULL) {
		if (scanner() == t) {
			current_token = t;
		} else {
			syntax_error();
		}
	} else {
		if (*temp_token == t) {
			current_token = *temp_token;
			temp_token = NULL;
		} else {
			syntax_error();
		}
	}
}

// Obtiene el siguiente token.
token next_token(void) {
	token tok;
	if (temp_token == NULL) {
		tok = scanner();
		temp_token = malloc(sizeof(tok));
		*temp_token = tok;
	} else {
		tok = *temp_token;
	}
	return tok;
}

/*		================		SEMANTICS		================	*/
typedef struct {
	// Para los operadores.
	enum op { PLUS, MINUS } operator;
} op_rec;

// Tipos de expresiones.
enum expr { IDEXPR, LITERALEXPR, TEMPEXPR };

// Para <primary> y <expression>
typedef struct {
	enum expr kind;
	union {
		char *name; // Para IDEXPR, TEMPEXPR.
		int val;	// Para LITERALEXPR.
	};
} expr_rec;

char symbol_table[2048][MAXIDLEN];
FILE *temp_data_stg;

// Esta en la tabla de simbolos?
extern int lookup(char *s);

// Pone el char* en la tabla de simbolos.
extern void enter(char *s);

token current_token;
token *temp_token;

// Revisa si una variable ha sido definida previamente.
int lookup(char *s) {
	char *symbol;
	for (int i = 0; i < 2048; i++) {
		symbol = symbol_table[i];
		if (symbol != NULL && strcmp(symbol, s) == 0) {
			return 1;
		}
	}
	return 0;
}

// Agrega una variable a la tabla de simbolos.
void enter(char *s) {
	static int index = 0;
	strcpy(symbol_table[index], s);
	index++;
}

char *get_temp(void) {
	// Maximo temporal alocado por ahora.
	static int max_temp = 0;
	static char tempname[MAXIDLEN];
	if (max_temp == 10) {
		max_temp = 0;
	}
	sprintf(tempname, "$t%d", max_temp);
	max_temp++;
	return tempname;
}

char *get_label(void) {
	// Maxima etiqueta alocada por ahora.
	static int max_label = 0;
	static char label[MAXIDLEN];

	sprintf(label, ".label%d", max_label);
	max_label++;

	return label;
}

void start(void) {
	// Inicializaciones semanticas.
	output = fopen(output_name, "w");
	if (output != NULL) {
		temp_data_stg = tmpfile();
		fprintf(temp_data_stg, ".data\n");

		fprintf(output, ".text\n");
		fprintf(output, ".globl _start\n");
		fprintf(output, "_start:\n");
	} else {
		printf(">> Error... I don't know what it is, and I cannot do anything, I give up... :( \n");
		exit(-1);
	}
}

void finish(void) {
	// Generar el codigo para terminar el programa.
	fprintf(output, "\tmov r7, #1\n\tswi 0\n");
	fprintf(output, "\n");

	long file_size;
	char *buffer;
	size_t result;

	// Determinar el tamano del archivo.
	fseek(temp_data_stg, 0, SEEK_END);
	file_size = ftell(temp_data_stg);
	rewind(temp_data_stg);

	// Reservar memoria para contener el archivo entero.
	buffer = (char *)malloc(sizeof(char) * file_size);
	

	// Copiar el archivo al buffer.
	result = fread(buffer, 1, file_size, temp_data_stg);
	if (result != file_size) {
		fputs(">> An error has occurred during a file operation... I give up... :L \n", stderr);
		exit(3);
	}

	// Ahora todo el archivo esta en el buffer.
	fprintf(output, "%s\n", buffer);
}

// Escribe el codigo ensamblador para hacer una asignacion de un valor.
void assign(expr_rec target, expr_rec source_expr) {
	// Generar el codigo de la asignacion.

}

/*		================		ERRORS			================	*/

// Error lexico
void lexical_error(void) {
	printf(">> A lexical error has been found during compile time. \n>> I give up... :/ \n");
	fclose(input);
	fclose(output);
	exit(-1);
}

// Error sintactico
void syntax_error(void) {
	printf(">> A syntax error has been found during compile time. \n>> I give up... :( \n");
	fclose(input);
	fclose(output);
	exit(-1);
}

// Error semantico
void semantic_error(void) {
	printf(">> A semantic error has been found during compile time. \n>> I give up... :L \n");
	fclose(input);
	fclose(output);
	exit(-1);
}