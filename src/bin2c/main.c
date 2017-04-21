#include <vector.h>

#include <stdio.h>

int main(int argc, char *argv[])
{
  FILE *fin = NULL;
  vector(char) *output = NULL;
  size_t count = 0;

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

  output = vector_new(char);

  while(1)
  {
    size_t nb = 0;
    unsigned char buffer[128] = {0};
    size_t i = 0;

    nb = fread(buffer, sizeof(unsigned char), 128, fin);
    if(nb <= 0) break;

    for(i = 0; i < nb; i++)
    {
      char sb[3] = {0};

      sprintf(sb, "%02x", buffer[i]);

      count+=2;
      vector_push_back(output, '\'');
      vector_push_back(output, sb[0]);
      vector_push_back(output, '\'');
      vector_push_back(output, ',');
      vector_push_back(output, '\'');
      vector_push_back(output, sb[1]);
      vector_push_back(output, '\'');
      vector_push_back(output, ',');
    }
  }

  count++;
  vector_push_back(output, '0');
  printf("char %s[%i] = {", argv[2], (int)count);

  printf("%s", vector_raw(output));
  printf("};\n");

  vector_delete(output);
  fclose(fin);

  return 0;
}

