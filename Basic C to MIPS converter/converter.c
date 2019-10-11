#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>



// integers might be positive, negative, or 0. 
// addition, subtraction, multiplication, division, and mod 
// on the same line, its always mixture of +/-. * and / and %, and assignment (x=1), so
   // no need to worry about the order of precedence. 
// Uses $s0....$s7 for saved, $t0.....$t9 for temps, however, the final result of the last assignment likely will not end up in register $s0. 
// recycling backs to $t0 if all up to $t9 are used.
// $s0 is always the very first variable that is either an assignment (g = 100), or the very first variable in 
// the right hand side of an expression, ie., f = g+h-42 where g is $s0. At the end, if the left side is an already existing variable, use that s, otherwise a new one.
// multiple lines of inputs are accomadated.


// ASSUMPTIONS: 
// ALL input files valid; 
// length of argv[1] is at most 128 characters( 128*4 = 512 bytes, so let say 1000), but not always populated; 
// Constants only appear as the second operand(NO CASE AS X = 42/Y); 
// NO ADJACENT NUMBERS ON THE TWO SIDES OF OPERATOR (I.E., 1+2, 1-2, 3/3, 2*2);


/* MULTIPLICATION:
- when multiplying with variable, do it in the mul, mflo way.
- when multiplying with positive constant/negative constant, use sll. For neg, replace the last one
// with sub $s, $zero, $t1 INSTEAD OF move $s1, $t1(positive) 
- when multiplying with 0, treat this as a special case - li $s3, 0; li $t3, 0
- when multiplying with 1 ir -1: 
# b = a * 1;
move $t0,$s0
move $s1,$t0

# b = a * -1;
move $t0,$s0
sub $s1,$zero,$t0


DIVISION:
- when dividing with a variable, do it in div, mflo way
- if divisor is constant, power of 2, test the first operand using bltz. 
   - If first operand negative, branch to div, mflo,  
   - if first operand positive, use srl, and then under the condition of 2nd operand neg, 
     // use sub $s1, $zero, $s1 after srl.
- if divisor is NOT power of 2, do it div and mflo way no matter 2nd operand pos or neg

bash$ cat example5.src
n = 100;
b = n / 32;
bash$ ./a.out example5.src
# n = 100;
li $s0,100
# b = n / 32;
bltz $s0,L0
srl $s1,$s0,5
j L1
L0:
li $t0,32
div $s0,$t0
mflo $s1
L1:
 
- if dividing 1 or -1 (special cases)
# b = a / 1;
move $s1,$s0
# b = a / -1;
sub $s1,$zero,$s0

*/
// *** Don't use pointer++ in if statements or anything unless 
// one intentionally wants to move to the next pointer. 
// Use (*[ptr+1] instead)!!!


// This is the function to find the index of saved variables
int convert_char_to_int(char * num){
     int j = 0;
     int result = 0;
     int x;
     while (num[j] != '-'){
     x = (num[j]) - '0';
     result *= 10;
     result += x;
     j++;
   }
   return result;
}

int convert_dec_to_bin(int x, int * bin){
    int i = 0; 
    int count = 0;
    while (x > 0) { 
  
        // storing remainder in binary array 
        bin[i] = x % 2; 
        x = x / 2; 
        i++; 
        count ++;
    } 
    count -= 1;
    return count;
}

int checkPowerofTwo(int x) {
   //checks whether a number is zero or not
   int pow =0;
   if (x == 0)
      return -1;

   //true till x is not equal to 1
   while( x != 1)
   {
      //checks whether a number is divisible by 2
      if(x % 2 != 0) { // if odd, no way to be power of 2
         return -1; }
         x /= 2;
         pow ++;
   }
   return pow;
}



int find_i(char * var_array, char var){
	
	int index=0;
	int j;
	for (j = 0; j<10; j++){
		if (var_array[j] == '0'){
			break;
		}
		if(var == var_array[j]){
			return index;
		}
		index++;
	}
	return j;
}

// Function to test if a specific variable already exists.
int is_the_var_there(char* var_array, char var){
	
	int index=0;
	int j;
	for (j = 0; j<10; j++){
		if (var_array[j] == '0'){
			break;
		}
		if(var == var_array[j]){
			return 1;
		}
		index++;
	}
	return 0;
}



char type(char* str){
   int j = 0;
   int is_mult_div = 0;
   int is_add_sub = 0;
   char a_s = '+';
   char m_d = '*';
   char ass = '=';
   while (str[j] != ';'){
      if((str[j] == '+' || str[j] == '-') && (str[j+1] == ' ')){
         is_add_sub = 1;
      }
      else if(str[j] == '*' || str[j] == '/' || str[j] == '%'){
         is_mult_div = 1;
      }
      j++;
   }
   if (is_mult_div == 1){
      return m_d;
   }
   else if (is_add_sub == 1){
      return a_s;
   }
   else if (is_mult_div == 0 && is_add_sub == 0){
      return ass;
   }
   return '-';
}


int main( int argc, char *argv[]) {

   if (argc != 2){
      fprintf(stderr,"Missing an command line argument!");
      return EXIT_FAILURE;
   }

   FILE *input;
   input = fopen(argv[1], "r");

   char str[128]; // input string
   char var[100]; // array to store the saved variable
   int k, j; 


   // initialize the saved var array so we know how many vars are there.
   for (k=0; k < 100; k++){
   	var[k] = '0';
   }
   for (k=0; k < 128; k++){
      str[k] = ' ';
   }

   // assuming that the max number is only 2-digit.
   // Everytime we encounter a number, store it in this array,
   // then erase, then repeat.
   int sign = 0;
   char num[10]= {'-', '-', '-', '-', '-', '-', '-', '-', '-', '-'};
   int binary[100];

   for (k = 0; k<100; k++){
      binary[k] = -1;
   }


   // The array we use to decide when to print the operations.
   // i.e., after we read two variables, we change the '0' to other
   // specified chars, and if both are changed, we proceed to print.
   char pair[2] = {'0', '0'};

   // Pointers to iterate through each of the arrays
   char* ptr = str; 
   char* var_ptr = var;
   char* pair_ptr = pair;
   char* num_ptr = num;

   // This int variable is used to keep track of temporary variables $t0~$t9
   int i = -1;
   int label = -1;

   // This command is to determine whether we use "+" or "-"
   int command = -1;

   // This index is used to find the index of the saved variable
   int index;
   char left_var = '-';


   // We use fget to read, test each char by is.lower, is.digit, etc, then store them into
   // their corresponding arrays.
   while (fgets(str, 128, input) != NULL){
      char type_of_str = type(str);
      j =0;
      printf("# ");
       while (str[j] != ';'){
          printf("%c", str[j]);
          j ++;
      }
      printf(";\n");

      if (type_of_str == '='){ // equal
            for(ptr = str; *ptr; ptr++){

               if (*(ptr) == ';'){

                  index = find_i(var, *pair);
                  printf("li $s%d,", index);
                  if (sign == -1){printf("-");}
                  j = 0;
                  while (num[j] != '-'){
                     printf("%c", num[j]);
                     j++;
                  }
                  printf("\n");
                  break;
               }

         if (*ptr == '-' && isdigit(*(ptr+1))){
            sign = -1;
         }
         // if the current character is alpha and it is the leftmost variable ($s0)
         // (*(ptr+1) != ';') is used to specify that it is not the case of last variable
         if((isalpha(*ptr)) && (*(ptr+2) == '=')){
            int is_there = is_the_var_there(var, *ptr);
            if (is_there == 0){ // if the variable is not there, store this new var to array
               *var_ptr = *ptr;
               var_ptr++;}
            *pair_ptr = *ptr;
             
         }

         if (isdigit(*ptr) && !(isdigit(*(ptr-1)))){
            int l = 0;
            while (isdigit(*(ptr+l))) {
               *(num_ptr+l)  = *(ptr+l); // first digit
               l++;
            }
            *(pair_ptr+1) = '#';
         }
      }
       memset(pair, '0', 2); // reset the pair to both '0''s
       memset(num, '-', 10); // reset the num to all "-"
       sign = 0;
       memset(str, ' ', 128);
   }



      else if (type_of_str == '+'){ //addition/subtraction
           
         for(ptr = str; *ptr; ptr++){
            if (*(ptr+2) == '='){
               left_var = *ptr;
            }

         // When we hit the end of the string ';', we print out the final result with $s0
         if(*(ptr) == ';'){
            if (i != -1) { i += 1;}
            if (i == 10) { i = 0;}
            pair_ptr = pair;
            index = find_i(var, left_var);
            int is_there = is_the_var_there(var, left_var);
            if (is_there == 0){ // if the variable is not there, store this new var to array
               *var_ptr = left_var;
               var_ptr++;}


            if (command == 0){
            printf("%s $s%d,", "add", index);}
            else if (command == 1){
            printf("%s $s%d,", "sub", index);}

            // the first slot
            if((*pair) == '1'){ // '1' is the hint for temp variables
               // i-1 would be the index of temp var since it is always from
               // the last operation
               printf("$t%d,",i-1); 
            }
            else if(islower(*pair)){  // saved variable
               index = find_i(var, *pair);
               printf("$s%d,", index); 
            }
            else if ((*pair)=='#'){ // if this is a number, we use the number stored in 
                              // num array
               j = 0;
               while (num[j] != '-'){
               printf("%c", num[j]);
                  j++;
               }
               printf(",");
            }

            // the second slot
            if (islower(*(pair_ptr+1))){
            index = find_i(var, *(pair_ptr+1));
               printf("$s%d\n", index);
            }
            if(((*pair_ptr+1)) == '1'){
               printf("$t%d,",i-1);
            }
            else if ((*(pair_ptr+1))=='#'){
               j = 0;
               while (num[j] != '-'){
               printf("%c", num[j]);
                j++;
               }
               printf("\n");
            }
            break;
         }

         if (*ptr == '+'){ // 0 stands for '+'
            command = 0;
           
         }
         if (*ptr == '-'){ // 1 stands for '-'
            command = 1;

         }
      
      //

      if(( (isalpha(*ptr)) && (*(ptr+2) == '='))){
            int is_there = is_the_var_there(var, *ptr);
            if (is_there == 0){ // if the variable is not there, store this new var to array
               *var_ptr = *ptr;
               var_ptr++;}
            left_var = *ptr;
         }



         //


         // if the current character is alpha and from $s0 - $9 (the right side of equation)
         if(( (isalpha(*ptr)) && (*(ptr+2) != '='))){
            int is_there = is_the_var_there(var, *ptr);
            if (is_there == 0){ // if the variable is not there, store this new var to array
               *var_ptr = *ptr;
               var_ptr++;}

            // Here, we fill in the var to either the first or second slot of the pair
            // 2 cases: it's either both empty or the first one is filled
            if(*pair_ptr == '0'){
            *pair_ptr = *ptr;}
            else{
            *(pair_ptr+1) = *ptr;
            }
         }

         // if the current char is a numerical, 2-digit value, 
         // and not the second number of the 2-digit value
         if ((isdigit(*ptr)) && (!(isdigit(*(ptr-1))))){
            int l = 0;
            while (isdigit(*(ptr+l))) {
               *(num_ptr+l)  = *(ptr+l); // first digit
               l++;
            }

            // again, assign the specified hint to pair
            if(*pair_ptr == '0'){
            *pair_ptr = '#';}
            else{
            *(pair_ptr+1) = '#';
            }

         }

         // if both slots of the pair is filled, we print out the operation IF AND ONLY IF IT'S
         // NOT THE LAST OPERATION, tested by if the next ptr points to ';'
         // !(isdigit(*(ptr+1) is used when we have the corner case of a variable and a 2-digit 
         // number, the program will start printing after the first digit of the number so in
         // order to avoid that, we need to specify to print after the second digit.
         if(*pair_ptr !='0' && *(pair_ptr+1) != '0' && *(ptr+1)!=';' && !(isdigit(*(ptr+1)))){
             i +=1;
            if (i == 10) { i = 0;}

            if (command == 0){ 
               printf("%s $t%d,", "add", i);
            }
            else if(command == 1){
               printf("%s $t%d,", "sub", i);
            }

            // first slot
            if((*pair) == '1'){ // if temp var
               printf("$t%d,",i-1);
            }
            else if(islower(*pair)){ // if alpha
               index = find_i(var, *pair);
               printf("$s%d,", index);
            }
            else if ((*pair)=='#'){ // if numerical
               j = 0;
               while (num[j] != '-'){
               printf("%c", num[j]);
                  j++;
               }
               printf(",");
            }

            // second slot
            if(((*pair_ptr+1)) == '1'){ 
               printf("$t%d,",i-1);
            }
            if (islower(*(pair_ptr+1))){
            index = find_i(var, *(pair_ptr+1));
               printf("$s%d\n", index);
            }
            else if ((*(pair_ptr+1))=='#'){
               j = 0;
               while (num[j] != '-'){
               printf("%c", num[j]);
                j++;
               }
               printf("\n");
            }
            memset(pair, '0', 2); // reset the pair to both '0''s, repeat, would never encounter
                             // the case of the last operation.
            memset(num, '-', 10);
            *pair = '1'; // IMPORTANT! Must set the first of the pair to '1' b/c it
                      // is a temporal variable now!!!
            command = -1;
           
         }      
      }
       memset(pair, '0', 2); // reset the pair to both '0''s
       memset(num, '-', 10); // reset the num to all "-"
       sign = 0;
       memset(str, ' ', 128);
       command = -1;
       left_var = '-';
   }







      else if (type_of_str == '*'){ // multiplication/

         for(ptr = str; *ptr; ptr++){
            if (*(ptr+2) == '='){
               left_var = *ptr;
            }

         // When we hit the end of the string ';', we print out the final result with $s0
         if(*(ptr) == ';'){

            pair_ptr = pair;
            int left_index = find_i(var, left_var);
            int is_there = is_the_var_there(var, left_var);
            if (is_there == 0){ // if the variable is not there, store this new var to array
               *var_ptr = left_var;
               var_ptr++;}



            if (command == 2){ 
               
         // if reg & #
               if ((islower(*pair_ptr) || *pair_ptr == '1') && (*(pair_ptr+1) == '#')){
               int integer = convert_char_to_int(num);
              

              // if x/0
              if (integer == 0){
               printf("li $s%d,0\n", left_index);
              }

              else if (integer == 1 && sign == 0){
                  if (islower(*pair_ptr)){
                     index = find_i(var, *pair_ptr);
                     i++;
                      if (i == 10) { i = 0;}
                     printf("move $t%d,$s%d\n", i, index);
                     printf("move $s%d,$t%d\n", left_index, i);
                  }
                  else if(*pair_ptr == '1'){
                     i++;  if (i == 10) { i = 0;}
                     printf("move $t%d,$t%d\n", i, i-1);
                     printf("move $s%d,$t%d\n", left_index, i);

                  }
              }

              else if (integer == 1 && sign == -1){
                  if (islower(*pair_ptr)){
                     index = find_i(var, *pair_ptr);
                     i++;  if (i == 10) { i = 0;}
                     printf("move $t%d,$s%d\n", i, index);
                     printf("sub $s%d,$zero,$t%d\n", left_index, i);
                  }
                  else if(*pair_ptr == '1'){
                     i++;  if (i == 10) { i = 0;}
                     printf("move $t%d,$t%d\n", i, i-1);
                     printf("sub $s%d,$zero,$t%d\n", left_index, i);

                  }
              }
               

               else{
                  int size_of_bin = convert_dec_to_bin(integer, binary);

                  if (islower(*pair_ptr)){
                     index = find_i(var, *pair_ptr);
                     j = 0;
                     int first = 0;
                     while (binary[j+1] != -1){
                        if (binary[j] == 1 && first == 0){
                           i ++;  if (i == 10) { i = 0;}
                           printf("sll $t%d,$s%d,%d\n", i, index, size_of_bin - j);
                           i++;  if (i == 10) { i = 0;}
                           printf("move $t%d,$t%d\n", i, i-1);
                           first = 1;
                        }
                        else if (binary[j] == 1 && first == 1){
                           printf("sll $t%d,$s%d,%d\n", i-1, index, size_of_bin -j);
                           printf("add $t%d,$t%d,$t%d\n", i, i, i-1);
                        }
                        j++;
                     }
                     printf("add $t%d,$t%d,$s%d\n", i, i, index);
                     if (sign != -1) {
                     printf("move $s%d,$t%d\n",left_index, i);}
                     else if (sign == -1){
                        printf("sub $s%d,$zero,$t%d\n", left_index, i);
                     }

                  }
                  else if (*pair_ptr == '1'){
                     j = 0;
                     int first = 0;
                     while (binary[j+1] != -1){
                        if (binary[j] == 1 && first == 0){
                           i ++;  if (i == 10) { i = 0;}
                           printf("sll $t%d,$t%d,%d\n", i, i-1, size_of_bin - j);
                           i++;  if (i == 10) { i = 0;}
                           printf("move $t%d,$t%d\n", i, i-1);
                           first = 1;
                        }
                        else if (binary[j] == 1 && first == 1){
                           printf("sll $t%d,$t%d,%d\n", i-1, i-2, size_of_bin -j);
                           printf("add $t%d,$t%d,$t%d\n", i, i, i-1);
                        }
                        j++;
                     }
                     printf("add $t%d,$t%d,$t%d\n", i, i, i-2);
                       if (sign != -1) {
                     printf("move $s%d,$t%d\n",left_index, i);}
                     else if (sign == -1){
                        printf("sub $s%d,$zero,$t%d\n", left_index, i);
                     }
                  }


            }
         }

            else if ((islower(*pair_ptr) || *pair_ptr == '1') && (islower(*(pair_ptr+1)))) {
               if (islower(*pair_ptr)) {
                  int index1 = find_i(var, *pair_ptr);
                  int index2 = find_i(var, *(pair_ptr+1));
                  printf("mult $s%d,$s%d\n", index1, index2);
                  printf("mflo $s%d\n", left_index);
               }
               else if (*pair_ptr== '1') {
                  int index = find_i(var, *(pair_ptr+1));
                  printf("mult $t%d,$s%d\n", i, index);
                  printf("mflo $s%d\n", left_index);
               }
            }
     
          }



           else if(command == 3){

               // if reg & #
               if ((islower(*pair_ptr) || *pair_ptr == '1') && (*(pair_ptr+1) == '#')){
               int integer = convert_char_to_int(num);
               int if_pow_of_2 = checkPowerofTwo(integer);
              

              // if x/0
              if (integer == 0){
               fprintf(stderr, "Division by zero error\n");
               return EXIT_FAILURE;
              }
              else if (integer == 1 && sign == 0){
                  if (islower(*pair_ptr)){
                     index = find_i(var, *pair_ptr);
                     printf("move $s%d,$s%d\n", left_index, index);
                  }
                  else if(*pair_ptr == '1'){
                     printf("move $s%d,$t%d\n", left_index, i);

                  }
              }
              else if (integer == 1 && sign == -1){
                  if (islower(*pair_ptr)){
                     index = find_i(var, *pair_ptr);
                     printf("sub $s%d,$zero,$s%d\n", left_index, index);
                  }
                  else if(*pair_ptr == '1'){
                     printf("sub $s%d,$zero,$t%d\n", left_index, i);

                  }
              }
               
              else if (if_pow_of_2 != -1){
                  label ++;

                  if (*pair_ptr == '1') {
                     printf("bltz $t%d,L%d\n", i, label);
                     printf("srl $s%d,$t%d,%d\n", left_index, i, if_pow_of_2);
                     if (sign == -1) { printf("sub $s%d,$zero,$s%d\n", left_index, left_index);}
                     label ++;
                     printf("j L%d\n",label);
                     printf("L%d:\n", label-1);
                     i ++;  if (i == 10) { i = 0;}
                     printf("li $t%d,", i);
                     if (sign == -1){printf("-");}
                        j = 0;
                         while (num[j] != '-'){
                          printf("%c", num[j]);
                         j++;
                          }
                     printf("\n");
                     printf("div $t%d,$t%d\n", i-1, i);
                     printf("mflo $s%d\n", left_index);
                     printf("L%d:\n", label);
                  }

                  else if (islower(*pair_ptr)) {
                     index = find_i(var, *pair_ptr);
                     printf("bltz $s%d,L%d\n", index, label);
                     printf("srl $s%d,$s%d,%d\n", left_index, index, if_pow_of_2);
                     if (sign == -1) { printf("sub $s%d,$zero,$s%d\n", left_index, left_index);}
                     label ++;
                     printf("j L%d\n",label);
                     printf("L%d:\n", label-1);
                     i ++;  if (i == 10) { i = 0;}
                     printf("li $t%d,", i);
                     if (sign == -1){printf("-");}
                        j = 0;
                         while (num[j] != '-'){
                          printf("%c", num[j]);
                         j++;
                          }
                     printf("\n");
                     printf("div $s%d,$t%d\n", index, i);
                     printf("mflo $s%d\n", left_index);
                     printf("L%d:\n", label);

                  }
               }

               else if (if_pow_of_2 == -1){
                  i ++;  if (i == 10) { i = 0;}
                  printf("li $t%d,",i);
                  if (sign == -1) {printf("-");}
                  j = 0;
                  while (num[j] != '-'){
                     printf("%c", num[j]);
                     j++;
                  }
                  printf("\n");
                  if (islower(*pair_ptr)){
                     index = find_i(var, *pair_ptr);
                     printf("div $s%d,$t%d\n", index, i);
                     printf("mflo $s%d\n", left_index);}
                  else if (*pair_ptr == '1'){
                     printf("div $t%d,$t%d\n", i-1, i);
                     printf("mflo $s%d\n", left_index);}
               }
            }

            else if ((islower(*pair_ptr) || *pair_ptr == '1') && (islower(*(pair_ptr+1)))) {
               if (islower(*pair_ptr)) {
                  int index1 = find_i(var, *pair_ptr);
                  int index2 = find_i(var, *(pair_ptr+1));
                  printf("div $s%d,$s%d\n", index1, index2);
                  printf("mflo $s%d\n", left_index);
               }
               else if (*pair_ptr== '1') {
                  int index = find_i(var, *(pair_ptr+1));
                  printf("div $t%d,$s%d\n", i, index);
                  printf("mflo $s%d\n", left_index);
               }
            }
         }

         //////////////////////////////////////////////////////////
         else if (command == 4){
             // if reg & #
               if ((islower(*pair_ptr) || *pair_ptr == '1') && (*(pair_ptr+1) == '#')){
               
                  if (islower(*pair_ptr)){
                     index = find_i(var, *pair_ptr);
                     printf("div $s%d,", index);
                     if (sign == -1) {printf("-");}
                     j = 0;
                     while (num[j] != '-'){
                     printf("%c", num[j]);
                     j++;
                  }
                     printf("\n");
                     printf("mfhi $s%d\n", left_index);}
                
                  else if (*pair_ptr == '1'){
                     printf("div $t%d,", i);
                     if (sign == -1) {printf("-");}
                     j = 0;
                     while (num[j] != '-'){
                        printf("%c", num[j]);
                        j++;
                     }
                     printf("\n");
                     printf("mfhi $s%d\n", left_index);}
               }

            else if ((islower(*pair_ptr) || *pair_ptr == '1') && (islower(*(pair_ptr+1)))) {
               if (islower(*pair_ptr)) {
                  int index1 = find_i(var, *pair_ptr);
                  int index2 = find_i(var, *(pair_ptr+1));
                  printf("div $s%d,$s%d\n", index1, index2);
                  printf("mfhi $s%d\n", left_index);
               }
               else if (*pair_ptr== '1') {
                  int index = find_i(var, *(pair_ptr+1));
                  printf("div $t%d,$s%d\n", i, index);
                  printf("mfhi $s%d\n", left_index);
               }
            }
         }

           
            break;
         }

         if(( (isalpha(*ptr)) && (*(ptr+2) == '='))){
            int is_there = is_the_var_there(var, *ptr);
            if (is_there == 0){ // if the variable is not there, store this new var to array
               *var_ptr = *ptr;
               var_ptr++;}
            left_var = *ptr;
         }

         if (*ptr == '*'){ // 0 stands for '+'
            command = 2;
           
         }
         if (*ptr == '/'){ // 1 stands for '-'
            command = 3;
         }
         if (*ptr == '%'){ // 1 stands for '-'
            command = 4;
         }
      
         if (*ptr == '-' && isdigit(*(ptr+1))){
            sign = -1;
         }
         // if the current character is alpha and from $s0 - $9 (the right side of equation)
         if(( (isalpha(*ptr)) && (*(ptr+2) != '='))){
            int is_there = is_the_var_there(var, *ptr);
            if (is_there == 0){ // if the variable is not there, store this new var to array
               *var_ptr = *ptr;
               var_ptr++;}

            // Here, we fill in the var to either the first or second slot of the pair
            // 2 cases: it's either both empty or the first one is filled
            if(*pair_ptr == '0'){
            *pair_ptr = *ptr;}
            else{
            *(pair_ptr+1) = *ptr;
            }
         }

         // if the current char is a numerical, 2-digit value, 
         // and not the second number of the 2-digit value
         if ((isdigit(*ptr)) && (!(isdigit(*(ptr-1))))){
            int l = 0;
            while (isdigit(*(ptr+l))) {
               *(num_ptr+l)  = *(ptr+l); // first digit
               l++;
            }

            // again, assign the specified hint to pair
            if(*pair_ptr == '0'){
            *pair_ptr = '#';}
            else{
            *(pair_ptr+1) = '#';
            }

         }

         // if both slots of the pair is filled, we print out the operation IF AND ONLY IF IT'S
         // NOT THE LAST OPERATION, tested by if the next ptr points to ';'
         // !(isdigit(*(ptr+1) is used when we have the corner case of a variable and a 2-digit 
         // number, the program will start printing after the first digit of the number so in
         // order to avoid that, we need to specify to print after the second digit.
         if(*pair_ptr !='0' && *(pair_ptr+1) != '0' && *(ptr+1)!=';' && !(isdigit(*(ptr+1)))){
             i +=1;
             if (i == 10) { i = 0; }


            if (command == 2){ 
               if ((islower(*pair_ptr) || *pair_ptr == '1') && (*(pair_ptr+1) == '#')){
               int integer = convert_char_to_int(num);
              

              // if x/0
              if (integer == 0){
               printf("li $t%d,0\n", i);
              }

              else if (integer == 1 && sign == 0){
                  if (islower(*pair_ptr)){
                     index = find_i(var, *pair_ptr);
                     printf("move $t%d,$s%d\n", i, index);
                     i++; if (i == 10) { i = 0; }
                     printf("move $t%d,$t%d\n", i, i-1);
                  }
                  else if(*pair_ptr == '1'){
                     printf("move $t%d,$t%d\n", i, i-1);
                     i++; if (i == 10) { i = 0; }
                     printf("move $t%d,$t%d\n", i, i-1);

                  }
              }

              else if (integer == 1 && sign == -1){
                  if (islower(*pair_ptr)){
                     index = find_i(var, *pair_ptr);
                     printf("move $t%d,$s%d\n", i, index);
                     i++; if (i == 10) { i = 0; }
                     printf("sub $t%d,$zero,$t%d\n", i, i-1);
                  }
                  else if(*pair_ptr == '1'){
                     printf("move $t%d,$t%d\n", i, i-1);
                     i++; if (i == 10) { i = 0; }
                     printf("sub $t%d,$zero,$t%d\n", i, i-1);

                  }
              }
               

               else{
                  int size_of_bin = convert_dec_to_bin(integer, binary);

                  if (islower(*pair_ptr)){
                     index = find_i(var, *pair_ptr);
                     j = 0;
                     int first = 0;
                     while (binary[j+1] != -1){
                        if (binary[j] == 1 && first == 0){
                           printf("sll $t%d,$s%d,%d\n", i, index, size_of_bin - j);
                           i++; if (i == 10) { i = 0; }
                           printf("move $t%d,$t%d\n", i, i-1);
                           first = 1;
                        }
                        else if (binary[j] == 1 && first == 1){
                           printf("sll $t%d,$s%d,%d\n", i-1, index, size_of_bin -j);
                           printf("add $t%d,$t%d,$t%d\n", i, i, i-1);
                        }
                        j++;
                     }
                     printf("add $t%d,$t%d,$s%d\n", i, i, index);
                     i ++; if (i == 10) { i = 0; }
                     if (sign != -1) {
                     printf("move $t%d,$t%d\n", i, i-1);}
                     else if (sign == -1){
                        printf("sub $t%d,$zero,$t%d\n", i, i-1);
                     }

                  }
                  else if (*pair_ptr == '1'){
                     j = 0;
                     int first = 0;
                     while (binary[j+1] != -1){
                        if (binary[j] == 1 && first == 0){
                           printf("sll $t%d,$t%d,%d\n", i, i-1, size_of_bin - j);
                           i++; if (i == 10) { i = 0; }
                           printf("move $t%d,$t%d\n", i, i-1);
                           first = 1;
                        }
                        else if (binary[j] == 1 && first == 1){
                           printf("sll $t%d,$t%d,%d\n", i-1, i-2, size_of_bin -j);
                           printf("add $t%d,$t%d,$t%d\n", i, i, i-1);
                        }
                        j++;
                     }
                     printf("add $t%d,$t%d,$t%d\n", i, i, i-2);
                     i++; if (i == 10) { i = 0; }
                       if (sign != -1) {
                     printf("move $t%d,$t%d\n",i, i-1);}
                     else if (sign == -1){
                        printf("sub $t%d,$zero,$t%d\n", i, i-1);
                     }
                  }
            }
         }



             else if ((islower(*pair_ptr) || *pair_ptr == '1') && (islower(*(pair_ptr+1)))) {
               if (islower(*pair_ptr)) {
                  int index1 = find_i(var, *pair_ptr);
                  int index2 = find_i(var, *(pair_ptr+1));
                  printf("mult $s%d,$s%d\n", index1, index2);
                  printf("mflo $t%d\n", i);
               }
               else if (*pair_ptr== '1') {
                  int index = find_i(var, *(pair_ptr+1));
                  printf("mult $t%d,$s%d\n", i-1, index);
                  printf("mflo $t%d\n", i);
               }
            }

      }



             ///////////////////////////////////////////////////////////////////////////

           else if(command == 3){

               if ((islower(*pair_ptr) || *pair_ptr == '1') && (*(pair_ptr+1) == '#')){
                  int integer = convert_char_to_int(num);
                  int if_pow_of_2 = checkPowerofTwo(integer);

                if (integer == 0){
                  fprintf(stderr, "Division by zero error\n");
                  return EXIT_FAILURE;
              }


               else if (integer == 1 && sign == 0){
                  if (islower(*pair_ptr)){
                     index = find_i(var, *pair_ptr);
                     printf("move $t%d,$s%d\n", i, index);
                  }
                  else if(*pair_ptr == '1'){
                     printf("move $t%d,$t%d\n", i, i-1);

                  }
              }

              else if (integer == 1 && sign == -1){
                  if (islower(*pair_ptr)){
                     index = find_i(var, *pair_ptr);
                     printf("sub $t%d,$zero,$s%d\n", i, index);
                  }
                  else if(*pair_ptr == '1'){
                     printf("sub $t%d,$zero,$t%d\n", i, i-1);

                  }
              }
               
              
              else if (if_pow_of_2 != -1){
                  label ++;

                  if (*pair_ptr == '1') {
                     printf("bltz $t%d,L%d\n", i-1, label);
                     printf("srl $t%d,$s%d,%d\n", i, index, if_pow_of_2);
                     if (sign == -1) { printf("sub $t%d,$zero,$t%d\n", i, i);}
                     label ++;
                     printf("j L%d\n",label);
                     printf("L%d:\n", label-1);
                     i ++; if (i == 10) { i = 0; }
                     printf("li $t%d,", i);
                     if (sign == -1){printf("-");}
                        j = 0;
                         while (num[j] != '-'){
                          printf("%c", num[j]);
                         j++;
                          }
                     printf("\n");
                     printf("div $t%d,$t%d\n", i-2, i);
                     i ++; if (i == 10) { i = 0; }
                     printf("mflo $t%d\n", i);
                     printf("L%d:\n", label);
                  }

                  else if (islower(*pair_ptr)) {
                     index = find_i(var, *pair_ptr);
                     printf("bltz $s%d,L%d\n", index, label);
                     printf("srl $t%d,$s%d,%d\n", i, index, if_pow_of_2);
                     if (sign == -1) { printf("sub $t%d,$zero,$t%d\n", i, i);}
                     label ++;
                     printf("j L%d\n",label);
                     printf("L%d:\n", label-1);
                     i ++; if (i == 10) { i = 0; }
                     printf("li $t%d,", i);
                     if (sign == -1){printf("-");}
                        j = 0;
                         while (num[j] != '-'){
                          printf("%c", num[j]);
                         j++;
                          }
                     printf("\n");
                     printf("div $s%d,$t%d\n", index, i);
                     i ++; if (i == 10) { i = 0; }
                     printf("mflo $t%d\n", i);
                     printf("L%d:\n", label);

                  }
               }

               else if (if_pow_of_2 == -1){
                  printf("li $t%d,",i);
                  if (sign == -1)
                     { printf("-");}
                  j = 0;
                  while (num[j] != '-'){
                     printf("%c", num[j]);
                     j++;
                  }
                  printf("\n");
                  if (islower(*pair_ptr)){
                     index = find_i(var, *pair_ptr);
                     printf("div $s%d,$t%d\n", index, i);
                     i +=1; if (i == 10) { i = 0; }
                     printf("mflo $t%d\n", i);}
                  else if (*pair_ptr == '1'){
                     printf("div $t%d,$t%d\n", i-1, i);
                     i +=1; if (i == 10) { i = 0; }
                     printf("mflo $t%d\n", i);}
               }
            }

            else if ((islower(*pair_ptr) || *pair_ptr == '1') && (islower(*(pair_ptr+1)))) {
               if (islower(*pair_ptr)) {
                  int index1 = find_i(var, *pair_ptr);
                  int index2 = find_i(var, *(pair_ptr+1));
                  printf("div $s%d,$s%d\n", index1, index2);
                  printf("mflo $t%d\n", i);
               }
               else if (*pair_ptr== '1') {
                  int index = find_i(var, *(pair_ptr+1));
                  printf("div $t%d,$s%d\n", i-1, index);
                  printf("mflo $t%d\n", i);
               }
            }
         }
         
            else if(command == 4){ // %
            
            if ((islower(*pair_ptr) || *pair_ptr == '1') && (*(pair_ptr+1) == '#')){
               
                  if (islower(*pair_ptr)){
                     index = find_i(var, *pair_ptr);
                     printf("div $s%d,", index);
                     if (sign == -1) {printf("-");}
                     j = 0;
                     while (num[j] != '-'){
                     printf("%c", num[j]);
                     j++;
                  }
                     printf("\n");
                     printf("mfhi $t%d\n", i);}
                
                  else if (*pair_ptr == '1'){
                     printf("div $t%d,", i-1);
                     if (sign == -1) {printf("-");}
                     j = 0;
                     while (num[j] != '-'){
                        printf("%c", num[j]);
                        j++;
                     }
                     printf("\n");
                     printf("mfhi $t%d\n", i);}
               }

            else if ((islower(*pair_ptr) || *pair_ptr == '1') && (islower(*(pair_ptr+1)))) {
               if (islower(*pair_ptr)) {
                  int index1 = find_i(var, *pair_ptr);
                  int index2 = find_i(var, *(pair_ptr+1));
                  printf("div $s%d,$s%d\n", index1, index2);
                  printf("mfhi $t%d\n", i);
               }
               else if (*pair_ptr== '1') {
                  int index = find_i(var, *(pair_ptr+1));
                  printf("div $t%d,$s%d\n", i-1, index);
                  printf("mfhi $t%d\n", i);
               }
            }

       }
            memset(pair, '0', 2); // reset the pair to both '0''s, repeat, would never encounter
                             // the case of the last operation.
            memset(num, '-', 10);
            memset(binary, -1, sizeof(binary));
            *pair = '1'; // IMPORTANT! Must set the first of the pair to '1' b/c it
                      // is a temporal variable now!!!
           sign = 0;
           command = -1;

         }      
      }
       memset(pair, '0', 2); // reset the pair to both '0''s
       memset(num, '-', 10); // reset the num to all "-"
       memset(binary, -1, sizeof(binary));
       sign = 0;
       memset(str, ' ', 128);
       command = -1;
       left_var = '-';

      }
  	
	
}
fclose(input);
   return EXIT_SUCCESS;
}

