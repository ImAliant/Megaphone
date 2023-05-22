#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "interface_client.h"

int ask(const char *question, char *answer_buf, size_t buf_size) {
  printf("%s> ", question);

  char *r = fgets(answer_buf, buf_size, stdin);
  if (r == NULL) {
    // EOF
    printf("\n");
    exit(0);
  }

  printf("\n");

  for (size_t i = 0; answer_buf[i]; i++) {
    if (answer_buf[i] == '\n') {
      answer_buf[i] = '\0';
    }
  }

  return 0;
}

char getCharacter(const char *s) {
  size_t len = sizeof(s);
  char buf[len + 1];
  memset(buf, 0, len + 1);

  size_t j = 0;
  for (size_t i = 0; s[i]; i++) {
    if (isgraph(s[i])) {
      buf[j] = s[i];
      j++;
    }
  }

  // entree vide
  if (!buf[0]) return -1;
  // plus qu'un caractere
  if (buf[1]) return -2;

  return tolower(buf[0]);
}
