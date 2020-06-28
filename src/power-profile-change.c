/*
 ============================================================================
 Name        : power-profile-change.c
 Author      : brunoais <https://github.com/brunoais/tuxedo-control-center-profile-changer>
 Version     :
 Description : Change tuxedo-control-center profile without sudo

 ============================================================================

 TEST:
 gcc power-profile-change.c -o power-profile-change `pkg-config --cflags -lm
 sudo chown root power-profile-change
 sudo chmod u+s power-profile-change

 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif


#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdarg.h>


/* Flag set by ‘--verbose’. */
static int verbose_flag = 0;
static char* tccd_path = "/opt/tuxedo-control-center/resources/dist/tuxedo-control-center/data/service/tccd";

void dump_help(){

            printf(
                    "\n\
Usage: power-profile-change [-p] config-location\n\
\n\
Change tuxedo-control-center profile without the use of the interface.\n\
\n\
Arguments:\n\
  -p config-location\t\tLocation of the profile file for tcc to load\n\
  -h\t\t\t\tDisplay this help and exit\n\
\n\
Without arguments this program will read the file specified and feed it to\n\
tcc service in order to swap the profile in use.\n\
This program runs with setuid=root flag so there's no need to input a password\n\
to change profiles.\n\
Profile files must be root-only writable and root owned for safety and stability reasons\n\
E.g.:\n\
    sudo chown root:root /path/to/profile.json\n\
    sudo chmod u+s /path/to/profile.json\n\
\n\
The profile change is done by using tcc's own API.\n\
\n");
}

#define PIPE_READ   0    
#define PIPE_WRITE  1 
#define MAX_PROFILE_CONTENTS 8192


int test_file(FILE* file){

    struct stat st;
    mode_t // owner,
     group, other;
  

    if(fstat(fileno(file), &st) < 0) {
        perror("Couldn't stat profile file.\n");
        return 1;
    }

    if(!S_ISREG(st.st_mode)){
        fprintf(stderr, "The profile path specified does not point to a file");
        exit(20);
    }


    // owner = st.st_mode & S_IRWXU;
    group = st.st_mode & S_IRWXG;
    other = st.st_mode & S_IRWXO;


    if(st.st_uid != 0){
        fprintf(stderr, "The profile file is not owned by root. Either change its owner to root.\n\
        E.g. chown root:root $FILE\n");
        exit(20);
    }

    int g_can_write = group & S_IWGRP;

    if(st.st_gid != 0 && g_can_write){
        fprintf(stderr, "The profile file is not in root group and is writable. Either change its group to root or make it non-writable\n\
        E.g. chown root:root $FILE\n\
        E.g. chmod g-w $FILE");
        exit(20);
    }

    int o_can_write = other & S_IWOTH;

    if(o_can_write){
        fprintf(stderr, "The profile file is world writable. Make it non-world writable\n\
        E.g. chmod o-w $FILE");
        exit(20);
    }

    return 0;

}



int test_and_read_file(char* file_name, char* buff){


    FILE* fp = fopen(file_name, "r");
    if (fp == NULL) {
        perror("Can't open profile file \n");
        exit(14);
    }

    test_file(fp);

    int readResult = fread(buff, 1, MAX_PROFILE_CONTENTS, fp);
    if(readResult < 1){
        perror("Can't read profile file \n");
        exit(14);
    }
    if(ferror(fp)){
        perror("Failed to read profile file \n");
        exit(14);
    }
    if(readResult == MAX_PROFILE_CONTENTS){
        if(!feof(fp)){
            perror("Profile file longer than expected \n");
            exit(14);
        }
    }
    clearerr(fp);

    return 0;
}


static void new_profile_choice(char* profile_location) {

    int myPipes[2];
    char buff[MAX_PROFILE_CONTENTS + 1] = {0};


    test_and_read_file(profile_location, buff);


    if( pipe( myPipes ) < 0 ){
        perror("Can't pipe through \n");
        exit(13);
    }

    if(verbose_flag) printf("pid is %d; pipe fds are.... %d & %d\n", getpid(), myPipes[PIPE_READ], myPipes[PIPE_WRITE]);

    //close (myPipes[PIPE_READ]);
    write (myPipes[PIPE_WRITE], buff, strlen(buff)); 
    close (myPipes[PIPE_WRITE]);

    char* pipeArg;
    if(verbose_flag){
        asprintf (&pipeArg, "/proc/%d/fd/%d", getpid(), myPipes[PIPE_READ]);
        printf("\n%s\n", pipeArg);
    } 

    //sleep(50000);
    asprintf (&pipeArg, "/dev/fd/%d", myPipes[PIPE_READ]);


    char* tccdArgv[] = {
        "tccd",
        "--new_settings",
        pipeArg,
        //"/dev/fd/0",
        NULL
    };


    // FILE* fp = fopen(pipeArg, "r");
    // if (fp == NULL) {
    //     perror("Can't open fd pipe file \n");
    //     exit(14);
    // }

    // fread(buff, sizeof(char), strlen(buff) + 1, fp);

    // printf("buff: %s", buff);

    //write(0,  buff, strlen(buff) + 1);
    //close(0);


    // FILE* fp = fopen("/dev/fd/0", "r");
    // if (fp == NULL) {
    //     perror("Can't open fd 0 pipe file \n");
    //     exit(14);
    // }

    // fread(buff, sizeof(char), strlen(buff) + 1, fp);

    // printf("buff2: %s", buff);

    // puts("scanwait\n");
    // scanf("%s", buff);
    // printf("buff2: %s", buff);

    // sleep(5000);

    // tccd must be called as root
    setuid(0);

    execvp(tccd_path, tccdArgv);
    perror("execvp screwed up");
    exit(15);
   
}



int main(int argc, char* argv[]) {
    printf("Simple tuxedo-control-center profile changer without root permissions\n");

    char* profile_location = NULL;

    static struct option long_options[] =
        {
          /* These options set a flag. */
          {"verbose",   no_argument,    &verbose_flag, 1},
          /* These options don’t set a flag.
             We distinguish them by their indices. */
          {"help"   ,   no_argument, NULL, 'h'},
          {"profile",   required_argument, NULL, 'p'},
          {0, 0, 0, 0}
        };

    int c;
    do {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "p:hv",
                        long_options, &option_index);

        switch (c) {
            case -1:
                // done
                break;
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
            break;

            case 'p':
                printf ("profile at location `%s'\n", optarg);
                profile_location = optarg;
            break;

            case 'h':
                dump_help();
                exit(255);
            break;

            case 'v':
                verbose_flag = 1;
            break;

            case '?':
                /* getopt_long already printed an error message. */
                dump_help();
                exit(64);
            break;

            default:
                abort ();
        }
    } while(c != -1);

    /* Instead of reporting ‘--verbose’
        and ‘--brief’ as they are encountered,
        we report the final status resulting from them. */
    if (verbose_flag)
        puts ("verbose is on");

    /* Print any remaining command line arguments (not options). */
    if (optind == argc - 1) {
        profile_location = argv[optind++];
        if(verbose_flag) printf ("non-option ARGV-elements: %s\n", profile_location);
    }

    if(!profile_location){
        fputs("No profile location was given", stderr);
        exit(64);
    }

    new_profile_choice(profile_location);
    
    exit(EXIT_SUCCESS);
}
