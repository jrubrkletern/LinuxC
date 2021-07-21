#include <stdio.h>
#include <stdlib.h>

int WFLAG = 0, CFLAG = 0, LFLAG = 0;

FILE *fp;

int ln, ch, chCount, word, totLines, totWords, totChars;

int lastEmpty = 0, multiFile = 0, newFileIndex = 0, nameIndex = 0;

char *cp, *filePrint;

int inputMode = 0;

pid_t pid;

int fd[2];

int dataStorage[3];

int childpid;
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
                inputMode = 1;
                multiFile++;
                }
        }

        if(inputMode == 1) {
                for(i = 1; i < argc; i++) {

                        if(argv[i][0] == '-') {
                                continue;
                        }
                        pipe(fd);

                        if((pid = fork()) == 0) {

                                getCount(argv[i]);

                                write(fd[1], dataStorage, sizeof(dataStorage));

                                exit(0);
                        }
                        close(fd[1]);
                        wait(NULL);

                        if(pid != 0) {
                                readPipe();
                                printResults(argv[i]);
                                }

                }
        } else {
                pipe(fd);

                if((pid = fork()) == 0) {

                        getCount("STDIN");

                        write(fd[1], dataStorage, sizeof(dataStorage));

                        exit(0);

                }
                close(fd[1]);
                wait(NULL);

                if(pid != 0) {
                        readPipe();
                        printResults(argv[0]);
                }
        }

        if(multiFile > 1) {
                printf("Total: ");
                dataStorage[2] = totLines;
                dataStorage[1] = totWords;
                dataStorage[0] = totChars;
                inputMode = 0;
                pid = 0;
                printResults(argv[0]);

         }

        printf("\n");
return 0;
}

int readPipe() {
        read(fd[0], dataStorage, sizeof(dataStorage));
        totLines += dataStorage[2];
        totWords += dataStorage[1];
        totChars += dataStorage[0];
        close(fd[0]);
}

int printResults(char myString[]) {
        if(LFLAG == 1) {
                 printf("%d ", dataStorage[2]);
        }
        if(WFLAG == 1) {
                printf("%d ", dataStorage[1]);
        }
        if(CFLAG == 1) {
                printf("%d ", dataStorage[0]);
        }
        if(LFLAG != 1 && WFLAG != 1 && CFLAG != 1) {
                printf("%d %d %d", dataStorage[2], dataStorage[1], dataStorage[0]);
        }
        if(inputMode == 1) {
                printf(" %s", myString);
        }
        if(pid != 0) {
                printf(" PID: %d ", pid);
        }
        printf("\n");



}

int getCount(char myString[]) {
        ln = 0;
        chCount = 0;
        word = 0;
        if(inputMode == 1) {
                fp = fopen(myString, "r");
        }
        char myChar;
        if(inputMode != 0) {
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

                if(inputMode != 0) {
                        ch = fgetc(fp);
                } else ch = getchar();

                if(ch != ' ' && ch != '\t' && lastEmpty == 1 && ch != '\n') {
                        lastEmpty = 0;
                        word++;
                }
                chCount++;


        }
        dataStorage[2] = ln;
        dataStorage[1] = word;
        dataStorage[0] = chCount;
        if(inputMode == 1) {
                fclose(fp);
        }
return 0;
}
