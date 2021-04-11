#ifndef PARSER_H
#define PARSER_H

#include "semantics.h"
#include "errores.h"
#include "scanner.h"

token current_token;
token *temp_token;

// Rutinas que llaman al scanner.
void match(token t);
token next_token(void);

// Rutinas de compilacion.
void system_goal(void);
void program(void);
void statement_list(void);
void statement(void);
void id_list(void);
void expression(expr_rec *target);
void expr_list(void);
op_rec add_op(void);
expr_rec primary(expr_rec target);

#endif