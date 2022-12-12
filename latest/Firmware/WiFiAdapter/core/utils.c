#include "utils.h"
#include <string.h>

int tokenize(char *msg, const char *sep, char **index, int max_tokens)
{
  char *token;
  int indexpos = 0;
  while ( (token=strsep(&msg, sep)) )
  {
    if ( indexpos < max_tokens )
      index[indexpos++] = token;
    else
      break;
  }

  return indexpos;
}
