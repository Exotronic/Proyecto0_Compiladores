#include "parser.h"
#include "scanner.h"

char *remove_extension(const char *filename);

int main(int argc, char const *argv[]) {
	char *input_name = argv[1];
	size_t input_name_length = strlen(input_name);

	if (input_name[input_name_length - 3] == '.' && input_name[input_name_length - 2] == 'm') {
		printf(">> File is not of language MICRO... \n>> Task Failed Successfully!\n");
		return 1;
	}

	input = fopen(argv[1], "r");

	if (input != NULL) {
		output_name = remove_extension(argv[1]);
		output_name = strcat(output_name, ".s");

		system_goal();

		fclose(input);
		fclose(output);

		return 0;
	} else {
		printf(">> Catastophic Failure! \nMaybe try deleting ./root/?\n");
		return 1;
	}
}

char *remove_extension(const char *filename) {
	char *res;
	char *final_dot;

	if (filename == NULL) {
		return NULL;
	}

	if ((res = malloc(strlen(filename) + 1)) == NULL) {
		return NULL;
	}

	strcpy(res, filename);

	final_dot = strrchr(res, '.');

	if (final_dot != NULL) {
		*final_dot = '\0';
	}

	return res;
}