/* Child Process writes to the shared memory and parent process reads from it */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

#define SHM_SIZE 1024  /* make it a 1K shared memory segment */

int main(int argc, char *argv[])
{
    key_t key;
    int shmid;
    int *data;
    int mode,i;

    if (argc > 2) {
        fprintf(stderr, "usage: shmdemo [data_to_write]\n");
        exit(1);
    }

    /* make the key: */
    if ((key = ftok("shmf.c", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }

    /* connect to (and possibly create) the segment: */
    if ((shmid = shmget(key, SHM_SIZE, 0644 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }
    if(!fork()){
    	    /* attach to the segment to get a pointer to it: */
		/*  shmat() returns a char pointer which is typecast here
		    to int and the address is stored in the int pointer a.
		    Thus the memory locations of the parent
		    are the same as the memory locations of
		    the child, since the memory is shared.
		*/
	    data = (int *) shmat(shmid, (void *)0, 0);
	    if (data == (int *)(-1)) {
		perror("shmat");
		exit(1);
	    }
	    for(i=0;i<5;i++)
	    {
	    printf("Parent Process-- Reading from segment: \"%d, %d \"\n", data[0],data[1]);
	    //sleep(1);
	    } 	
	    wait(NULL);
	    /* detach from the segment: */
	    if (shmdt(data) == -1) {
		perror("shmdt");
		exit(1);
	    }
		/* Child has exited, so parent process should delete
		   the cretaed shared memory. Unlike attach and detach,
		   which is to be done for each process separately,
		   deleting the shared memory has to be done by only
		   one process after making sure that noone else
		   will be using it 
		 */

	    shmctl(shmid, IPC_RMID, 0); 
     }
     else
     {		/* Child Process */
		/*  shmat() returns a char pointer which is typecast here
		    to int and the address is stored in the int pointer b. */
	    data = (int *) shmat(shmid, (void *)0, 0);
	    if (data == (int *)(-1)) {
		perror("shmat");
		exit(1);
	    }
 		data[0]=1;data[1]=1;
	    for(i=0;i<5;i++)
	    {
	    data[0]=data[0]*2;
	    data[1]=data[1]*3;
	    printf("Child Process: Writing to segment: \"%d, %d \"\n", data[0],data[1]);
	    //sleep(1);
	    }

	    /* detach from the segment: */
	    if (shmdt(data) == -1) {
		perror("shmdt");
		exit(1);
	    }     	    

    }
    return 0;
}
