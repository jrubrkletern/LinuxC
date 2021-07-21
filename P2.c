#include <stdio.h>
#include <stdlib.h>



main() {

char ch;
char check;
int linenumber = 2;
ch = getchar();
check = getchar();
if(check == EOF) {
ch = check;
} else {
printf("1\t");
printf("%c", ch);
ch = check;
 }
while(ch != EOF) {

if(ch == '\n') {
printf("%c", ch);
printf("%d", linenumber);
printf("\t");


linenumber++;
} else printf("%c", ch);

ch = getchar();

}





}
