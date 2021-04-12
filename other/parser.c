#include "parser.h"

// Verificar que el token gramatico es igual al del programa.
void match(token t) {
	if(temp_token == NULL) {
		current_token = scanner();
	} else {
		current_token = *temp_token;
		temp_token = NULL;
	}

	if (t != current_token) {
		printf("Token no es matched %d y %d\n", t, current_token);
		syntax_error();
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

void system_goal(void) {
	/* <system goal> ::= <program> SCANEOF */
	program();
	//printf("Hola?");
	//match(SCANEOF);
}

void program(void) {
	/* <program> ::= BEGIN <statement list> END */
	start();

	match(BEGIN);
	statement_list();
	match(END);

	finish();
}

void statement_list(void) {
	/*
	 *	<statement list> ::= <statement> { <statement> }
	 */
	statement();
	while (1) {
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
			expression(&id, 0); ////
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
			//printf("Next token type: %d\n", next_token());
			//printf("Token en write: %s\n", token_buffer.token);
			expr_list(next_token());
			match(RPAREN);
			match(SEMICOLON);
			break;
		default:
			printf("Aqui 1");
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
void expression(expr_rec *target, int flag) {
	/*
	 *	<expression> ::= <primary> { <add op> <primary> }
	 */

	token t;
	int print_flag = flag;
	if (print_flag) {
		//printf("Target Kind: %d\n", target->kind);
		//target->kind = IDEXPR;
		//printf("Target Kind: %d\n", target->kind);
		write_expr(*target);
	} else {
		expr_rec op1 = primary(*target);
		for (t = next_token(); t == PLUSOP || t == MINUSOP; t = next_token()) {
			op_rec op = add_op();
			expr_rec op2 = primary(*target);
			op1 = gen_infix(op1, op, op2);
		}
		//printf("\n");
		assign(*target, op1);
	}

	/*
	// Si el target es NULL es porque se tiene que imprimir.
    if (target == NULL) {
        char *tmp = get_temp();
		printf("Temp: %s\n", tmp);
        enter(tmp);
        fprintf(temp_data_stg, "%s: .word 0\n", tmp);
        expr_rec tmp_expr = process_temp(tmp);

        target = &tmp_expr;

        print_flag = 1;
    }

	expr_rec op1 = primary(*target);
	for (t = next_token(); t == PLUSOP || t == MINUSOP; t = next_token()) {
		op_rec op = add_op();
		expr_rec op2 = primary(*target);
		op1 = gen_infix(op1, op, op2);
	}

	///////////
	printf("Op1 Kind: %d\n", op1.kind);
	printf("Target Kind: %d\n", target->kind);
	printf("Print Flag: %d\n", print_flag);
	//printf("\n");
	assign(*target, op1);
	/*if (op1.kind == IDEXPR) {
        char *tmp = get_temp();
        enter(tmp);
        fprintf(temp_data_stg, "%s: .word 0\n", tmp);
        expr_rec tmp_expr = process_temp(tmp);
        assign(tmp_expr, op1);
        *target = tmp_expr;
    } else {
		assign(*target, op1);
	}*/

	/*if (print_flag) {
		//printf("Target Kind: %d\n", target->kind);
		//target->kind = IDEXPR;
		//printf("Target Kind: %d\n", target->kind);
		write_expr(*target);
	}*/
}

// Verifica la lista de expresiones.
void expr_list(token tok) {
	// <expr list> ::= <expression> { , <expression> }
	expr_rec target;
	if (tok != RPAREN) {
		//match(COMMA);
		if (tok == ID) {
			match(ID);
			target = process_id(token_buffer.token);
			expression(&target, 1);
		} else if (tok == INTLITERAL) {
			match(INTLITERAL);
			target = process_literal(token_buffer.token);
			expression(&target, 1);
		}
		//tok = next_token();
	}
	printf("Salio\n");
	//expression(NULL);
	//while (next_token() == COMMA)

	/*while (next_token() != RPAREN) {
		match(COMMA);
		//expression(NULL);
		expression(&target, 1);
	}*/
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
		printf("Aqui 2");
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
			expression(&target, 0); ////
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
			printf("Matcheo un token %d\n", tok);
			syntax_error();
			break;
	}

	return src;
}