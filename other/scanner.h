#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definicion de tipos.
#include <ctype.h>

// Importar el header de manejo de errores.
#include "errores.h"

extern const char *reserved_w[];

// Este enum es para determinar el tipo de token que el scanner encuentra.
typedef enum token_types {
	BEGIN, END, READ, WRITE, ID,
	INTLITERAL, LPAREN, RPAREN, SEMICOLON,
	COMMA, ASSIGNOP, PLUSOP, MINUSOP, SCANEOF
} token;

// Este struct es para el buffer que agarra las palabras del archivo de entrada una por una.
typedef struct {
	char *token;	// Contiene el string del token
	int size;		// Tamano del string
} buffer;

// Este struct es para determinar la linea del archivo de output y la posicion de la columna.
typedef struct {
	int num;		// Numero de linea
	int offset;		// Caracter de linea
} line_t;

// Declaracion de archivos.
FILE *input; // Archivo de entrada
FILE *output; // Archivo de salia
char *output_name; // Nombre del archivo de salida (termina en .s)

const char *reserved_w[4]; // Aqui se almacenan las palabras reservadas del programa {read, write, begin, end}
line_t line_info;
buffer token_buffer;

token scanner(void);
void buffer_char(char c);
void clear_buffer(void);
token chech_reserved();

#endif