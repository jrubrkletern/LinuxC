#include <stdio.h>
#include <stdlib.h>

int WFLAG = 0, CFLAG = 0, LFLAG = 0;

FILE *fp;

int ln, ch, chCount, word, totLines, totWords, totChars;

int lastEmpty = 0, multiFile = 0, newFileIndex = 0, nameIndex = 0;

char *cp, *filePrint;

int inputMode = 1;

main(int argc, char *argv[]) {


        int i;
        for(i = 1; i < argc; i++) {

        cp = argv[i];
        if(*cp == '-') {
                while(*cp != 0) {
                        if(*++cp == 'w') {
                                WFLAG = 1;
                        } else if (*cp == 'c') {
                                CFLAG = 1;
                        } else if (*cp == 'l') {
                                LFLAG = 1;
                        }
                }
        } else {

                inputMode = 0;

                }


        }

        while(inputScan(argc, argv) != 0) {
                ln = 0, word = 0, chCount = 0, multiFile = 0;

                }


        if(newFileIndex > 1) {
                printf("Total: ");
                if(LFLAG == 1) {
                        printf("%d ", totLines);
                }

                if(WFLAG == 1) {
                        printf("%d ", totWords);
                }

                if(CFLAG == 1) {
                        printf("%d ", totChars);
                }

                else if(LFLAG != 1 && WFLAG != 1 && CFLAG != 1) {
                        printf("%d %d %d\n", totLines, totWords, totChars);

                }
        }


return 0;
}

int inputScan(int argc, char *argv[]) {
        int i = 0;
        if(inputMode == 0) {
                for(i = newFileIndex+1; i < argc; i++) {
                        cp = argv[i];
                        if(*cp != '-' && multiFile == 0) {
                                multiFile = 1;
                                newFileIndex = i;
                                nameIndex = i;


                        }  else if(*cp != '-' && multiFile  == 1) {

                                multiFile = 2;

                                i = argc-1;
                        }
                }
        fp = fopen(argv[newFileIndex], "r");
        filePrint = argv[newFileIndex];

        }
        getCount();

        if(inputMode == 0) {
                fclose(fp);
        }


        if(multiFile == 2) {
                return 1;
        } else return 0;
}

int getCount() {
        char myChar;
        if(inputMode != 1) {
                ch = fgetc(fp);
        } else ch = getchar();

        while(ch != EOF) {
                if(ch == '\n') {
                        ln++;
                        lastEmpty = 1;
                }
                if(ch == ' ' || ch == '\t') {
                        lastEmpty = 1;
                }

                if(inputMode != 1) {
                        ch = fgetc(fp);
                } else ch = getchar();

                if(ch != ' ' && ch != '\t' && lastEmpty == 1 && ch != '\n') {
                        lastEmpty = 0;
                        word++;
                }
                chCount++;


        }

        totLines += ln;
        totWords += word;
        totChars += chCount;

        if(LFLAG == 1) {
                printf("%d ", ln);
        }
        if(WFLAG == 1) {
                printf("%d ", word);

        }
        if(CFLAG == 1) {
                printf("%d ", chCount);
        }
        if(LFLAG != 1 && WFLAG != 1 && CFLAG != 1) {
                printf("%d %d %d", ln, word, chCount);
        }
        if(inputMode == 0) {
                printf(" %s", filePrint);
        }
        printf("\n");

return 0;
}
