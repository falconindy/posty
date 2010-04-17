/* Copyright (c) 2010 Dave Reisner
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CONTINUE 1
#define BREAK 0

#define STACK_SIZE 512


static double opstack[512] = {0};
static double *stackptr;

/* default runtime options */
static int verbose = 0;
static int precision = 3;

/* protos */
static char *strtrim(char*);
static int parse_expression(char*);
static int parse_operand(char*, double*);
static int parse_operator(char);
static void resetstack();

char *strtrim(char *str) {
  char *pch = str;

  if (str == NULL || *str == '\0')
    return str;

  while (isspace(*pch)) pch++;

  if (pch != str)
    memmove(str, pch, (strlen(pch) + 1));

  if (*str == '\0')
    return str;

  pch = (str + strlen(str) - 1);

  while (isspace(*pch))
    pch--;

  *++pch = '\0';

  return str;
}

void resetstack() {
  if (stackptr == &opstack[0])
    return;

  if (verbose) {
    printf(":: Stack Dump :: ");
    while (stackptr != &opstack[0])
      printf("%.*f ", precision, *--stackptr);
    putchar('\n');
  } else
    stackptr = &opstack[0];

}

/** parse operations */
int parse_operand(char *token, double *operand) {
  char *endPtr;

  *operand = strtod(token, &endPtr);
  if (*operand == HUGE_VAL) {
    fprintf(stderr, "!! Input overflow.\n");
    return 1;
  }
  if (token + strlen(token) != endPtr) {
    fprintf(stderr, "!! Bad input: %s\n", token);
    return 1;
  }

  return 0;
}

int parse_operator(char operator) {
  double op1, op2;

  op2 = *--stackptr;
  op1 = *--stackptr;

  if (verbose)
    printf(":: %.*f ", precision, op1);

  switch (operator) {
    case '+': op1 += op2; break;
    case '-': op1 -= op2; break;
    case '*': op1 *= op2; break;
    case '^': op1 = pow(op1, op2); break;
    case '/': if (! op2) {
                fprintf(stderr, "!! Divide by zero\n");
                return 1;
              }
              op1 /= op2; 
              break;
    case '%': if (! op2) {
                fprintf(stderr, "!! Divide by zero\n");
                return 1;
              }
              op1 = (int)op1 % (int)op2; 
              break;
  }

  if (verbose)
    printf("%c %.*f = %.*f\n", operator, precision, op2, precision, op1);

  if (op1 == HUGE_VAL) {
    fprintf(stderr, "!! Result overflow\n");
    return 1;
  }

  if (stackptr != &opstack[STACK_SIZE])
    *stackptr++ = op1;
  else {
    fprintf(stderr, "!! Stack overflow. Expression too large.\n");
    resetstack();
  }

  return 0;
}

int parse_precision(char *p) {
  char *endPtr;
  int pre = (int)strtol(p, &endPtr, 10);
  if (endPtr != p + strlen(p)) {
    fprintf(stderr, "!! Bad precision specified\n");
    return 1;
  } else {
    precision = pre;
    if (precision < 0) /* clamp negative numbers to 0 */
      precision ^= precision;
    printf(":: Precision set to %d decimal places.\n", precision);
  }

  return 0;

}

int parse_expression(char *expr) {

  if (strlen(strtrim(expr)) == 0)
    return BREAK;

  char *token;
  static const char *operators = "+/*-%^";
  double operand;

  while ((token = strsep(&expr, " \n"))) {
    if (strlen(token) == 0) continue;

    if (strchr(operators, *token) && strlen(token) == 1) { /* Caught an operator */
      if (stackptr - opstack < 2) {
        fprintf(stderr, "!! Malformed expression -- insufficient operands.\n");
        return CONTINUE;
      }
      if (parse_operator(*token) > 0) {
        return CONTINUE;
      }
    } else if (*token == ':') {
      parse_precision(++token);
    } else { /* Hope this is an operand */
      if (parse_operand(token, &operand) > 0) /* Parse failed, error thrown, next expr */
        return CONTINUE;

      if (stackptr != &stackptr[STACK_SIZE])
        *stackptr++ = operand;
      else {
        fprintf(stderr, "!! Stack overflow. Expression too large.\n");
        resetstack();
      }
    }
  }

  if (stackptr - opstack > 1)
    fprintf(stderr, "!! Malformed expression -- excess operands.\n");
  else if (stackptr - opstack == 1) {
    printf(" = %.*f\n", precision, *--stackptr);
  }

  return CONTINUE;
}


int main(int argc, char *argv[]) {
  if (argc > 1 && strcmp(argv[1], "-v") == 0) {
    fprintf(stderr, "::Stack dumps enabled::\n");
    verbose = 1;
  }

  char *buf;
  buf = calloc(sizeof(char), BUFSIZ);

  stackptr = &opstack[0];

  do {
    resetstack();
    printf("> ");
    *buf = '\0';
    fgets(buf, BUFSIZ, stdin);
  } while (parse_expression(buf));

  free(buf);

  return 0;
}
