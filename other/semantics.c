#include "parser.h"
#include "semantics.h"

// Revisa si una variable ha sido definida previamente.
int lookup(char *s) {
	char *symbol;
	for (int i = 0; i < 1024; i++) {
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

void start(void) {
	// Inicializaciones semanticas.
	output = fopen(output_name, "w");
	if (output != NULL) {
		temp_data_stg = tmpfile();
		fprintf(temp_data_stg, ".data\n");

		fprintf(output, ".text\n");
		fprintf(output, ".globl _start\n");
		fprintf(output, "_start:\n");

		int length = sizeof(symbol_table)/sizeof(symbol_table[0]);
		for (int i = 0; i < length; i++) {
			symbol_table[i][0] = '\0';
		}
	} else {
		printf(">> Error... I don't know what it is, and I cannot do anything, I give up... :( \n");
		exit(-1);
	}
}

void finish(void) {
	// Generar el codigo para terminar el programa.
	fprintf(output, "\tmov r7, #1\n\tswi 0\n");
	fprintf(output, "\n");

	//long file_size;
	//char *buffer;
	//size_t result;

	printf("??\n");
	int length = 1024;
	printf("%d?\n",  length);
	//int length = 2048;

	printf("antes del for");
	for (int i = 0; i < length; i++) {
		printf("???\n");
		if (symbol_table[i][0] == '\0') {
			break;
		}
		printf("????\n");
		char var_name[] = "\nadr_";
		strcat(var_name, symbol_table[i]);
		fprintf(output, var_name);

		fprintf(output, ": .word ");
		fprintf(output, symbol_table[i]);
	}

	fprintf(output, "\n.data\n");
	for (int j = 0; j < length; j++) {
		if (symbol_table[j][0] == '\0') {
			break;
		}

		fprintf(output, symbol_table[j]);
		fprintf(output, ": .word 0\n");
	} 

	/*
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
	fprintf(output, buffer);
	fprintf(output, "\n");
*/
	fprintf(output, "string_: .asciz \"%d\\n\"\n");
	fprintf(output, "fmtInput_: .string \"%d\"");
	fprintf(output, "\n");

}

// Escribe el codigo ensamblador para hacer una asignacion de un valor.
void assign(expr_rec target, expr_rec source_expr) {
	/* Generate code for assigment */
	if (source_expr.kind == LITERALEXPR && target.kind == IDEXPR) {
		if (!lookup(target.name)) {
			enter(target.name);
		} else {
			fprintf(output, "\n\tldr r0, =dir_");
			fprintf(output, extract(target));
			fprintf(output, "\n");

			fprintf(output, "\tldr r1, =");
			fprintf(output, extract(source_expr));
			fprintf(output, "\n");

			fprintf(output, "\tmov r2, r1\n");
			fprintf(output, "\tstr r2, [r0]");
		}
	}

	if (source_expr.kind == LITERALEXPR && target.kind == TEMPEXPR) {
		fprintf(output, "\tmov r0, adr_%s\n\tmov r1, #%s", target.name, extract(source_expr));
	}

	if (source_expr.kind == IDEXPR && target.kind == TEMPEXPR) {
		fprintf(output, "\tldr r2, adr_%s\n\tstr %s, r3", target.name, extract(source_expr)); // doubt
	}

	if (source_expr.kind == IDEXPR && target.kind == IDEXPR) {
		char *tmp_reg = get_temp();
		if (!lookup(target.name)) {
			enter(target.name);
		}
        fprintf(output, "\tldr r4, adr_%s\n\tmov r5, #%s", tmp_reg, extract(source_expr));
        fprintf(output, "\tldr r6, [r5]\n\tstr r6, [r4]\n");
		///fprintf(output, "lw %s, %s\n", tmp_reg, extract(source_expr));
		//fprintf(output, "sw %s, %s\n", tmp_reg, target.name);
	}

	if (source_expr.kind == TEMPEXPR && target.kind == IDEXPR) {
		if (!lookup(target.name)) {
			enter(target.name);
		}
		fprintf(output, "\tldr r8, adr_%s\n\tmov r9, #%s", extract(source_expr), target.name);
        fprintf(output, "\tldr r10, [r9]\n\tstr r10, [r8]\n");
	}

	if (source_expr.kind == TEMPEXPR && target.kind == TEMPEXPR) {
		fprintf(output, "\tmov %s, %s\n", target.name, extract(source_expr));
	}
}


op_rec process_op() { 
	// Produce el operador descriptor.
	op_rec o;

	if (current_token == PLUSOP) {
		o.operator = PLUS;
	} else {
		o.operator = MINUS;
	}

	return o;
}

// Escribe las operaciones en ensamblador.
expr_rec gen_infix(expr_rec e1, op_rec op, expr_rec e2) {
	expr_rec e_rec;

	// Arreglar constantes - constant folding.
	if (e2.kind == LITERALEXPR && e1.kind == LITERALEXPR) {
		e_rec.kind = LITERALEXPR;

		if (op.operator == PLUS) {
			e_rec.val = e2.val + e1.val;
		} else {
			e_rec.val = e1.val - e2.val;
		}
	} else {
		// Una expr_rec con tipo Temp.
		e_rec.kind = TEMPEXPR;

		// Genera el codigo por una operacion infix.
		// Se obtiene el resultado y se crea un record semantico para este.

		strcpy(e_rec.name, get_temp());
		if (e1.kind == LITERALEXPR) {
			// ldr r1, =<e1>
			fprintf(output, "\n\tldr r1, =");
			fprintf(output, extract(e1));
			fprintf(output, "\n");
		} else if (e1.kind == IDEXPR) {
			// ldr r2, =dir_<e1>
			fprintf(output, "\n\tldr r2, =dir_");
			fprintf(output, extract(e1));
			fprintf(output, "\n");

			// ldr r1, [r2]
			fprintf(output, "\tldr r1, [r2]\n");
		} else {
			// ldr r2, =<e1>
			fprintf(output, "\n\tldr r2, =");
			fprintf(output, extract(e1));
			fprintf(output, "\n");

			// ldr r1, [r2]
			fprintf(output, "\tldr r1, [r2]\n");
		}

		if (e2.kind == LITERALEXPR) {
			// ldr r2, =<e2>
			fprintf(output, "\n\tldr r2, =");
			fprintf(output, extract(e2));
			fprintf(output, "\n");
		} else if (e2.kind == IDEXPR) {
			// ldr r3, =dir_<e2>
			fprintf(output, "\n\tldr r3, =dir_");
			fprintf(output, extract(e2));
			fprintf(output, "\n");

			// ldr r2, [r3]
			fprintf(output, "\tldr r2, [r3]\n");
		} else {
			// ldr r3, =<e2>
			fprintf(output, "\n\tldr r3, =");
			fprintf(output, extract(e2));
			fprintf(output, "\n");

			// ldr r2, [r3]
			fprintf(output, "\tldr r2, [r3]\n");
		}

		fprintf(output, "\t");
		fprintf(output, extract_op(op)); // add r0, r1, r2 // sub r0, r1, r2
		fprintf(output, " r0, r1, r2\n");
		fprintf(output, "\tldr r1, dir_"); // ldr r1, adr_<temp_name>
		fprintf(output, e_rec.name);
		fprintf(output, "\n");
		fprintf(output, "\tmov r2, r0 \n");
		fprintf(output, "\tstr r2, [r1]\n"); // str r0, [r1]
	}

	return e_rec;
}

// Escribe la operacion para leer de stdin.
void read_id(expr_rec in_var) {

}

expr_rec process_id(char *token) {
	expr_rec tok;

	// Declara un ID y construye el record semantico correspondiente.

	tok.kind = IDEXPR;
	tok.name = malloc(sizeof(token));
	strcpy(tok.name, token);
	
	return tok;
}

expr_rec process_temp(char *token) {
	expr_rec tok;

	// Declara un ID y construye el record semantico correspondiente.

	tok.kind = TEMPEXPR;
	tok.name = malloc(sizeof(token));
	strcpy(tok.name, token);

	return tok;
}

expr_rec process_literal(char *token) {
	expr_rec tok;

	// Convierte un literal en una representacion numerica y construye el record semantico correspondiente.

	tok.kind = LITERALEXPR;
	sscanf(token, "%d", &tok.val);

	return tok;
}

// Escribe una expresion en el archivo ensamblador.
void write_expr(expr_rec out_expr) {

}

// Extrae la informacion de una expresion.
char *extract(expr_rec expr) {
	char *data = NULL;

	if (expr.kind == TEMPEXPR || expr.kind == IDEXPR) {
		data = expr.name;
	} else {
		data = malloc(sizeof(int));
		sprintf(data, "%d", expr.val);
	}

	return data;
}

// Extrae la operacion de una expresion.
char *extract_op(op_rec op) {
	char *operation;

	if (op.operator == PLUS) {
		operation = "add";
	} else {
		operation = "sub";
	}

	return operation;
}