//Gives the count of number of .jpg files present in the current directory

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc,char *argv[])
{
    int pfds[2];
    //printf("hello%s",argv[1]);
    pipe(pfds);

    if (!fork()) 
    {
        close(1);       /* close normal stdout so that it will not print anything to screen */
        dup(pfds[1]);   /* make stdout same as pfds[1] grabs stdout file descriptor,whatever prints to output will be stored in pfds1 file*/
        close(pfds[0]); /* we don't need this * no need for input or read to this command */
	execlp("sh", "sh", "-c", "ls -l *.jpg", NULL);  // As wildcard characters will be understood only by shell, running the command under 									//the shell
	exit(0);

    } 
    else 
    {
	wait(NULL);
        close(0);       /* close normal stdin */
        dup(pfds[0]);   /* make stdin same as pfds[0] */
        close(pfds[1]); /* we don't need this */
        execlp("wc", "wc", "-l",NULL);
    }

    return 0;
}
