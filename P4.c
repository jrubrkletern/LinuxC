#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>

        struct passwd *pwd; //passwd struct, for storing info about the user running the process

        struct stat stt; //stat struct for storing info about the file we want to find permissisons for

        gid_t gid;

        uid_t uid;

        char *home;

        char *file;

        int main(int argc, char *argv[]) {

                gid = getgid();

                uid = getuid();

                pwd = getpwuid(getuid()); //pull our worling directory based off the uid pulled above.

                home = pwd->pw_dir; //storing this for later use to get to our if it is relative.  it will be relative to this and we can build the path starting with home

                int i = 0;

                for(i = 1; i < argc; i++) { //for each file in the command line build the directory of the file, then retrieve file info via stat, user info via passwd then comapre uids and gids with stat struct.

                        if(argv[i][0] != '/') {//if our file is relative
                                file = malloc(strlen(argv[i])+strlen(home) + 1); //allocate this to the size the relative path declared
                                strcat(file, home);
                                strcat(file, "/");
                                strcat(file, argv[i]);
                        } else { //in the case that the path is relative
                                file = malloc(strlen(argv[i]));
                                strcpy(file, argv[i]);

                        }

                        if(stat(file, &stt) == -1) { //grab file information and store it in
                                printf("Cannot access file, %s.", argv[i]); //in the case that our file doesnt exist.
                                free(file);

                        } else {

                                printf("%s ", argv[i]);
                                //now we can print out permissions by comparing uids and gids.

                                if(uid == stt.st_uid) {

                                        printf("User is owner ");
                                        if(stt.st_mode & 0400) {

                                                printf("r");

                                        } else printf("-");

                                        if(stt.st_mode & 0200) {

                                                printf("w");

                                        } else printf("-");


                                        if(stt.st_mode & 0100) {

                                                printf("x");

                                        } else printf("-");

                                } else if(gid == stt.st_gid) {

                                        printf("User is in the same group as owner ");
                                        if(stt.st_mode & 0040) {
                                                printf("r");
                                        } else printf("-");

                                        if(stt.st_mode & 0020) {
                                                printf("w");
                                        } else printf("-");

                                        if(stt.st_mode & 0010) {
                                                printf("x");
                                        } else printf("-");

                                } else {

                                        printf("User is neither owner nor in their group ");
                                        if(stt.st_mode & 0004) {
                                                printf("r");
                                        } else printf("-");
                                        if(stt.st_mode & 0002) {
                                                printf("w");
                                        } else printf("-");
                                        if(stt.st_mode & 0001) {
                                                printf("x");
                                        } else printf("-");
                                }




                        free(file);
                        }
                        printf("\n");



                }


}
