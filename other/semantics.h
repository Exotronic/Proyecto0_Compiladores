#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stdio.h>
#include "errores.h"
#include "scanner.h"

#define MAXIDLEN 33

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

typedef struct {
	// Para los operadores.
	enum op { PLUS, MINUS } operator;
} op_rec;

char symbol_table[1024][MAXIDLEN];
FILE *temp_data_stg;

// Pone el char* en la tabla de simbolos.
extern void enter(char *s);

// Esta en la tabla de simbolos?
extern int lookup(char *s);

char *get_temp(void);
void start(void);
void finish(void);
void assign(expr_rec target, expr_rec source);

expr_rec gen_infix(expr_rec e1, op_rec op, expr_rec e2);
void read_id(expr_rec in_var);
expr_rec process_id(char *token);
expr_rec process_temp(char *token);
expr_rec process_literal(char *token);
op_rec process_op();
void write_expr(expr_rec out_expr);

char *extract(expr_rec expr);
char *extract_op(op_rec expr);

#endif