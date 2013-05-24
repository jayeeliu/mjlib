#include <stdio.h>
#include <unistd.h>

void Usage(char* cmd) {
  fprintf(stderr,
    "Usage: %s [-a value] [-b] [-c] [-d] <filename1> <filename2>\n", cmd);
}

int main(int argc, char* argv[]) {
  int ch;
  opterr = 0;
  while ((ch = getopt(argc, argv, "a:bcd")) != -1) {
    switch (ch) {
    case 'a':
      printf("a value: %s\n", optarg);
      break;
    case 'b':
      printf("b set\n");
      break;
    case 'c':
      printf("c set\n");
      break;
    case 'd':
      printf("d set\n");
      break;
    default:
      Usage(argv[0]);
      return 1;
    }
  }

  if (optind >= argc) {
    Usage(argv[0]);
    return 1;
  }
    
  printf("filename1:%s\n", argv[optind++]);
  printf("filename2:%s\n", argv[optind++]);
  return 0;
}
