#include "tokenizer.h"

#define MAX_STRING_LENGTH 255
#define DELIMITER " "

/*
 * Function to return the array of tokens in a string delimited by space
 * @param   {char*, char**} String to be split, token's array 
 * @return  {int} number of tokens
 *
 * Example Snippet: 
 *  char* line ="act bat cat dog";
 *  char* tokens[10];
 *  int number_of_tokens = parseString(line, tokens);
 *  for(int i=0; i<number_of_tokens; i++){
 *    printf("%s: %lu\n", tokens[i], strlen(tokens[i]));
 *  }
 *
 */
int tokenize(char* line, char* tokens[]){
  // Split the string into an array of strings 
  // String = array of chars -> char*, char[]
  // Array of strings = array of array of chars = char**, char[][]
  // using strtok to tokenize 
  // strtok(string, delimiter)
  char* buffer;
  int token_count = 0;
  char* temp_token;
  buffer = (char*) malloc(strlen(line) * sizeof(char));
  strcpy(buffer, line);
  temp_token = strtok(buffer, DELIMITER);
  tokens[token_count] = temp_token;
  while(temp_token != NULL) {
    token_count++;
    temp_token = strtok(NULL, DELIMITER);
    tokens[token_count] = temp_token;
  }
  return (token_count);
}
