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
	sprintf(tempname, "t$%d", max_temp);
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
		fprintf(output, ".global main\n\n");
		fprintf(output, "main:\n");

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
	fprintf(output, "\n\tmov r7, #1\n\tswi 0\n");
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
	fprintf(output, buffer);
	fprintf(output, "\n");

	fprintf(output, "string: .asciz \"%%d\\n\"\n");
	fprintf(output, "input: .string \"%%d\"");
	fprintf(output, "\n");

}

// Escribe el codigo ensamblador para hacer una asignacion de un valor.
void assign(expr_rec target, expr_rec source_expr) {
		printf("Target: %s\n", extract(target));
    /* Generate code for assigment */
    if (source_expr.kind == LITERALEXPR && target.kind == IDEXPR) {
        if (!lookup(target.name)) {
            enter(target.name);
            fprintf(temp_data_stg, "dir_%s: .word 0\n", target.name);

            fprintf(output, "\n\tldr r0, =dir_");
            fprintf(output, extract(target));
            fprintf(output, "\n");

            fprintf(output, "\tldr r1, =");
            fprintf(output, extract(source_expr));
            fprintf(output, "\n");

            fprintf(output, "\tmov r2, r1\n");
            fprintf(output, "\tstr r2, [r0]\n");
        } else {
            fprintf(output, "\n\tldr r0, =dir_");
            fprintf(output, extract(target));
            fprintf(output, "\n");

            fprintf(output, "\tldr r1, =");
            fprintf(output, extract(source_expr));
            fprintf(output, "\n");

            fprintf(output, "\tmov r2, r1\n");
            fprintf(output, "\tstr r2, [r0]\n");
        }
    }

	if (source_expr.kind == IDEXPR && target.kind == LITERALEXPR) { 
        if (!lookup(source_expr.name)) {
            enter(source_expr.name);
            fprintf(temp_data_stg, "dir_%s: .word 0\n", source_expr.name);

            fprintf(output, "\n\tldr r0, =dir_");
            fprintf(output, extract(source_expr));
            fprintf(output, "\n");

            fprintf(output, "\tldr r1, =");
            fprintf(output, extract(target));
            fprintf(output, "\n");

            fprintf(output, "\tmov r2, r1\n");
            fprintf(output, "\tstr r2, [r0]\n");
        } else {
            fprintf(output, "\n\tldr r0, =dir_");
            fprintf(output, extract(source_expr));
            fprintf(output, "\n");

            fprintf(output, "\tldr r1, =");
            fprintf(output, extract(target));
            fprintf(output, "\n");

            fprintf(output, "\tmov r2, r1\n");
            fprintf(output, "\tstr r2, [r0]\n");
        }
    } 

	if (source_expr.kind == LITERALEXPR && target.kind == TEMPEXPR) { 
		fprintf(output, "\n\tldr r0, =");
		fprintf(output, extract(target));
		fprintf(output, "\n");

		fprintf(output, "\tldr r1, =");
		fprintf(output, extract(source_expr));
		fprintf(output, "\n");

		fprintf(output, "\tmov r2, r1\n");
		fprintf(output, "\tstr r2, [r0]\n");
	}

	if (source_expr.kind == IDEXPR && target.kind == TEMPEXPR) {
		fprintf(output, "\tldr r0, =");
		fprintf(output, extract(target));
		fprintf(output, "\n");
		printf("Se metio esta picha\n");

		fprintf(output, "\tldr r1, =dir_");
		fprintf(output, extract(source_expr)); 
		fprintf(output, "\n");
	}

	if (source_expr.kind == IDEXPR && target.kind == IDEXPR) {
		char *tmp_reg = get_temp();
		enter(tmp_reg);
		fprintf(temp_data_stg, "%s: .word 0\n", tmp_reg);

		if (!lookup(target.name)) {
			enter(target.name);
			fprintf(temp_data_stg, "dir_%s: .word 0\n", target.name);
		}
        fprintf(output, "\tldr r4, =%s\n", tmp_reg);
		fprintf(output, "\tmov r5, #%s", extract(source_expr));
        fprintf(output, "\tldr r6, [r5]\n\tstr r6, [r4]\n");
	}

	if (source_expr.kind == TEMPEXPR && target.kind == IDEXPR) {
		if (!lookup(target.name)) {
			enter(target.name);
			fprintf(temp_data_stg, "dir_%s: .word 0\n", target.name);
		}
		fprintf(output, "\n\tldr r8, =");
		fprintf(output, extract(source_expr));
		fprintf(output, "\n");

		fprintf(output, "\tldr r9, =dir_");
		fprintf(output, target.name);
		fprintf(output, "\n");

		fprintf(output, "\tldr r10, [r8]\n");
		fprintf(output, "\tstr r10, [r9]\n");
	}

	if (source_expr.kind == TEMPEXPR && target.kind == TEMPEXPR) {
		fprintf(output, "\tmov %s, %s\n", target.name, extract(source_expr));
	}
	printf("\n");
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

		e_rec.name = get_temp();
		enter(e_rec.name);
		fprintf(temp_data_stg, "%s: .word 0\n", e_rec.name);
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
		fprintf(output, "\tldr r1, ="); // ldr r1, adr_<temp_name>
		fprintf(output, e_rec.name);
		fprintf(output, "\n");
		fprintf(output, "\tmov r2, r0 \n");
		fprintf(output, "\tstr r2, [r1]\n"); // str r0, [r1]
	}

	return e_rec;
}

// Escribe la operacion para leer de stdin.
void read_id(expr_rec in_var) {
	fprintf(output, "\n\tldr r0, =input\n");
	//fprintf(output, extract(in_var));
	fprintf(output, "\tbl scanf\n");
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
	fprintf(output, "\n\tldr r0, =string\n");
	if (out_expr.kind == INTLITERAL) {
		// ldr r1, =<out_expr>
		fprintf(output, "\tldr r1, =");
		fprintf(output, extract(out_expr));
		fprintf(output, "\n");
	} else if (out_expr.kind == IDEXPR) {
		// ldr r2, =dir_<out_expr>
		fprintf(output, "\tldr r1, =dir_");
		fprintf(output, extract(out_expr));
		fprintf(output, "\n");

		// ldr r1, [r2]
		fprintf(output, "\tldr r1, [r1]\n");
	} else {
		// ldr r2, =<out_expr>
		fprintf(output, "\tldr r1, ="); ///
		fprintf(output, extract(out_expr));
		fprintf(output, "\n");

		// ldr r1, [r2]
		fprintf(output, "\tldr r1, [r1]\n");
	}
	fprintf(output, "\tbl printf\n");
}

// Extrae la informacion de una expresion.
char *extract(expr_rec expr) {
	char *data;

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