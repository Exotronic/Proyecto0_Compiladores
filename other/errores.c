#include "errores.h"

// Error lexico
void lexical_error(void) {
	printf(">> A lexical error has been found during compile time. \n>> I will initiate self-destruct... \n");
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
	printf(">> A semantic error has been found during compile time. \n>> C r i t i c a l   E r r o r\n");
	fclose(input);
	fclose(output);
	exit(-1);
}