#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

int string_Split(char str[])
{
char ** res  = NULL,*op;
char *  p    = strtok (str, " ");
int n_spaces = 0, i,num1,num2,result;


/* split string and appends tokens to 'res' */

while (p) {
 	 res = realloc (res, sizeof (char*) * ++n_spaces);

	if (res == NULL)
	    exit (-1); /* memory allocation failed */

	res[n_spaces-1] = p;

	p = strtok (NULL, " ");
	}

	num1=atoi(res[0]);		//Gets first number
	num2=atoi(res[2]);		//Gets second number
	if(!strcmp(res[1],"+"))		//Gets operator
	{
    		result=num1 + num2;
		return result;
	}
	else if(!strcmp(res[1],"-"))
        	return num1-num2;
        else if(!strcmp(res[1],"*"))
                return num1*num2;
        else
            	return num1/num2;


/* free the memory allocated */
free (res);
 //return result;
}
struct my_msgbuf {
    long mtype;
    char mtext[200];
};

int main(void)
{
    struct my_msgbuf buf;
    char buffer[10];
    int msqid,output,i;
    key_t key;
    FILE *f;
    

    if ((key = ftok("kirk.c", 'B')) == -1) {  /* same key as kirk.c */
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, 0644)) == -1) { /* connect to the queue */
        perror("msgget");
        exit(1);
    }
    
    printf("spock: ready to receive messages, captain.\n");

for(; ; ) {
    /* Spock never quits! */
        if (msgrcv(msqid, &buf, sizeof buf.mtext, 0, 0) == -1) 
        {
            perror("msgrcv");
            exit(1);
        }
        printf("spock: \"%s\"\n", buf.mtext);
	memset(buffer, '\0', 10);
	strcpy(buffer,buf.mtext);
	output=string_Split(buf.mtext);
	if(!(f=fopen("file","a")))			//Opening a file to write the output.. All the data from the begining will be stored.
	{
	perror("File Descriptor");
	exit(1);
	}
	if(fprintf(f,"client id: %d Input : %s Output : %d\n",buf.mtype,buffer,output) == -1)
	{
	perror("file problem");
	exit(1);
	}
	fclose(f);
}
	
    return 0;
}
