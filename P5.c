#include <sys/dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

        int main(int argc, char *argv[]) {
        if(argc > 2) {
                printf("Please only supply one directory to recursively print at a time. \n");
                exit(0);
        }
                else if(argc < 1) {
                        recursivePrint(".");

                }
                else {
                        recursivePrint(argv[1]);

                }

                return 0;
        }
                 int recursivePrint(char *myChar) {
                        chdir(myChar);
                        DIR *dp;
                        struct direct *dir;

                        if((dp = opendir(".")) == NULL) {

                                fprintf(stderr, "Cannot open directory.\n");
                                exit(1);

                        }

                        while((dir = readdir(dp)) != NULL) {

                                if (dir->d_ino == 0) {

                                        continue;

                                }

                                if(strcmp(dir->d_name, "..") == 0 || strcmp(dir->d_name, ".") == 0) {

                                continue;
                                }
                                if(DT_DIR & dir->d_type) {
                                        printf("%s\n", dir->d_name);
                                        recursivePrint(dir->d_name);
                                        chdir("..");
                                }
                        }
                closedir(dp);
                return 0;
                }
