#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
   char command[50];

   strcpy(command, "python otp-code.py " );
   strcat(command, argv[1]);
   int result = system(command);

   printf("Result: %d \n", result);

   return(0);
} 