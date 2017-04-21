#include <stdio.h>

int main(int argc, char *argv[])
{
  FILE *fin = NULL;

  if(argc < 3)
  {
    printf("Usage: path_to_data output_name\n");

    return 1;
  }

  fin = fopen(argv[1], "rb");

  if(!fin)
  {
    printf("Error: Failed to open input file\n");

    return 1;
  }

  printf("char *%s = \"", argv[2]);

  while(1)
  {
    size_t nb = 0;
    unsigned char buffer[128] = {0};
    size_t i = 0;

    nb = fread(buffer, sizeof(unsigned char), 128, fin);
    if(nb <= 0) break;

    for(i = 0; i < nb; i++)
    {
      printf("%02x", buffer[i]);
    }
  }

  printf("\";\n");

  fclose(fin);

  return 0;
}

