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

/* stack declaration */
typedef struct __stack_t {
  double data;
  struct __stack_t *next;
} stack_t;

static stack_t *opstack = NULL;
static int stack_size = 0;

/* default runtime options */
static int verbose = 0;
static int precision = 3;

/* protos */
static char *strtrim(char*);
static int parse_expression(char*);
static int parse_operand(char*, double*);
static int parse_operator(char);
static void clearstack();
static stack_t *push(stack_t*, double);
static stack_t *pop(stack_t*);
static stack_t *poptop(stack_t*, double*);
static double top(stack_t*);

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

/** stack operations */
void clearstack() {
  if (opstack == NULL)
    return;

  if (verbose)
    printf(":: Stack Dump :: ");
  while (opstack) {
    if (verbose)
      printf("%.*f ", precision, top(opstack));
    opstack = pop(opstack);
  }
  if (verbose)
    putchar('\n');
}

stack_t *push(stack_t *stack, double data) {
  stack_t *newnode = malloc(sizeof(stack_t));
  newnode->data = data;
  stack_size++;

  if (stack == NULL)
    newnode->next = NULL;
  else
    newnode->next = stack;

  return newnode;
}

stack_t *pop(stack_t *stack) {
  stack_t *tmpnode = stack->next;

  free(stack);
  stack_size--;

  return tmpnode;
}

stack_t *poptop(stack_t *stack, double *op) {
  *op = top(stack);
  return pop(stack);
}

double top(stack_t *stack) {
  return stack->data;
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

  opstack = poptop(opstack, &op2);
  opstack = poptop(opstack, &op1);

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

  opstack = push(opstack, op1); /* push result back onto stack */

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
      if (stack_size < 2) {
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

      opstack = push(opstack, operand);
    }
  }

  if (stack_size > 1)
    fprintf(stderr, "!! Malformed expression -- excess operands.\n");
  else if (stack_size == 1) {
    printf(" = %.*f\n", precision, top(opstack));
    opstack = pop(opstack);
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

  do {
    clearstack();
    printf("> ");
    *buf = '\0';
    fgets(buf, BUFSIZ, stdin);
  } while (parse_expression(buf));

  free(buf);

  return 0;
}
