#include <stdio.h>


int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stdout, "Use example: %s PORT\n", argv[0]);
		return 0;
	}

	return 0;
}
