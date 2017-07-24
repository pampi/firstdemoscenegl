#include <stdio.h>
#include <stdlib.h>

#define READ_SIZE 4096

FILE *fIn, *fOut;
unsigned char file_buffer[READ_SIZE];
size_t bytes_read;
int i;

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		fprintf(stderr, "Sorry, to less arguments!\n");
		return EXIT_FAILURE;
	}

	fIn = fopen(argv[1], "rb");
	fOut = fopen(argv[3], "w");

	if (!fIn || !fOut)
	{
		fclose(fIn);
		fclose(fOut);
		fprintf(stderr, "Failed to open or create one of files!\n");

		return EXIT_FAILURE;
	}

	fprintf(fOut, "unsigned char %s[] = {", argv[2]);

	//print file content as hex values array C like code
	do
	{
		bytes_read = fread(file_buffer, 1, READ_SIZE, fIn);

		for (i = 0; i < bytes_read; i++)
		{
			fprintf(fOut,
					  (i == bytes_read - 1 && bytes_read != READ_SIZE)
							? "0x%.2X" : "0x%.2X,",
					  file_buffer[i]);
		}
	} while(bytes_read == READ_SIZE);

	fprintf(fOut, "};\n");

	fclose(fIn);
	fclose(fOut);

	return EXIT_SUCCESS;
}
