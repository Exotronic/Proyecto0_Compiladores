#include "scanner.h"

// Palabras reservadas
const char *reserved_words[] = {"begin", "end", "read", "write"};

// Posicion inicial del archivo
line_t line_info = {1, 0};

// Esta funcion revisa si es una palabra reservada o un ID.
token check_reserved() {
	for (int i = 0; i < 4; ++i) {
		if (!strcmp(reserved_words[i], token_buffer.token)) {
			if (i == 0) {
				return BEGIN;
			} else if (i == 1) {
				return END;
			} else if (i == 2) {
				return READ;
			} else if (i == 3) {
				return WRITE;
			}
		}
	}

	return ID;
}

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
				line_info.num++;
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
				do { in_char = getc(input); } while (in_char != '\n');
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
		token_buffer.token = (char *)calloc(token_buffer.size, sizeof(char));
		token_buffer.size = 1;
	}
	token_buffer.token[token_buffer.size - 1] = c;
}

// Limpiar el buffer
void clear_buffer(void) {
	free(token_buffer.token);
	token_buffer.token = NULL;
}