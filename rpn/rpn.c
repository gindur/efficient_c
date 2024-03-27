#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define N		(10)

static void error(unsigned int line, int c, bool* err){
	char buf[3];

	if (c == '\n')
		strcpy(buf, "\\n");
	else {
		buf[0] = c;
		buf[1] = 0;
	}
	printf("line %u: error at %s\n", line, buf);
	
	*err = true;
}

int main(void)
{	
	int			stack[N];
	int 		i;
	int 		c;
	int 		x; //Value of a number
	bool		num; //reading a number
	bool 		err; //found error on the line
	unsigned	line; //line number

	x = 0;
	i = 0;
	line = 1;
	num = false;
	err = false;


	while ((c = getchar()) != EOF){
		//printf("stack is: %d\n", i);
		if (err){
			if (c == '\n'){
				line += 1;
				err = 0;
				i = 0;
			}
			continue;

		} else if (isdigit(c)){
			x = x * 10 + c - '0';
			num = true;
			continue;
		} else if (num) {
			if (i == N){
				error(line, '0' + x%10, &err);
			} else {
				stack[i] = x;
				i++;
				num = false;
				x = 0;
			}
		} 
		
		if (!isdigit(c)){
			if (c == 10){
				if (i > 1 || i == 0){
					//printf("stack is: %d", i);
					error(line, '\n', &err);
					err = 0;
					i = 0;
					line += 1;
				} else {
					printf("line %d: %d\n", line, stack[i-1]);
					line++;
					i = 0;
					err = 0;
					num = false;
				}
				i = 0;
			}else if (c == 32){
				continue;
			} else if (c == '+'){
				if (i>= 2){
					stack[i-2] = stack[i-2] + stack[i-1];
					i--;
				} else {
					error(line, '+', &err);
				}

			} else if (c == '-'){
				if (i>= 2){
					stack[i-2] = stack[i-2] - stack[i-1];
					i--;
				} else {
					error(line, '-', &err);
				}
				
			} else if (c == '*'){
				if (i>= 2){
					stack[i-2] = stack[i-2] * stack[i-1];
					i--;
				} else {
					error(line, '*', &err);
				}
				
			} else if (c == '/'){
				if (i>= 1 && stack[i-1] != 0){
					stack[i-2] = stack[i-2] / stack[i-1];
					i--;
				} else {
					error(line, '/', &err);
				}
				
			} else {
				error(line, c, &err);
			}
		} 
	}
	return 0;
}
