#ifndef __UTILS_H__
#define __UTILS_H__

/**
 * @brief Enhanced string tokenizer using strsep(). Returns array of tokens for quick access.
 * 
 * @param msg Buffer to be destructively parsed
 * @param sep String of separator characters
 * @param index Output token index
 * @param max_tokens Size of token index
 * @return int Number of tokens extracted
 */
int tokenize(char *msg, const char *sep, char **index, int max_tokens);

#endif

