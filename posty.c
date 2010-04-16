#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CONTINUE 1
#define BREAK 0

typedef struct __stack_t {
  double data;
  struct __stack_t *next;
} stack_t;

/* stack declaration */
stack_t *opstack = NULL;
int stack_size = 0;

/* default runtime options */
int verbose = 0;
int precision = 3;

/* protos */
int parse_expression(char*);
int parse_operator(char);
stack_t *push(stack_t*, double);
stack_t *pop(stack_t*);
stack_t *poptop(stack_t*, double*);
double top(stack_t*);

static char *strtrim(char *str) {
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
static void clearstack() {
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
/*********************/


/** parse operations */
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

  char *token, *endPtr;
  static const char *operators = "+/*-%^";
  double operand;

  while ((token = strsep(&expr, " \n"))) {
    if (strlen(token) == 0) continue;

    if (strchr(operators, *token) && strlen(token) == 1) { /* Caught an operator */
      if (stack_size < 2) {
        fprintf(stderr, "!! Malformed expression -- insufficient operands.\n");
        return CONTINUE;
      }
      if (parse_operator(*token) > 0) { /* This should never be executed */
        return CONTINUE;
      }
    } else if (*token == ':') {
      parse_precision(++token);
    } else { /* Caught an operand, validate it */
      errno = 0;
      operand = strtod(token, &endPtr);
      if (errno != 0) {
        operand = fabs(operand);
        if (operand == HUGE_VAL)
          fprintf(stderr, "!! Input overflow.\n");
        if (operand == 0)
          fprintf(stderr, "!! Input underflow.\n");

        return CONTINUE;
      }
      if (token + strlen(token) != endPtr) {
        fprintf(stderr, "!! Bad input: %s\n", token);
        return CONTINUE;
      }
      opstack = push(opstack, operand); /* passed validation, push onto stack */
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
/*********************/

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

