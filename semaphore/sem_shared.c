/*Child Process writes to the shared memory and parent process reads it.. Reader writer problem which occurs is avoided using semaphores */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <errno.h>


#define SHM_SIZE 1024  /* make it a 1K shared memory segment */

#define MAX_RETRIES 10

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

/* Semaphore Initialization*/
int initsem(key_t key, int nsems)  /* key from ftok() */
{
    int i;
    union semun arg;
    struct semid_ds buf;
    struct sembuf sb;			//user defined semaphore operation strucutres
    int semid;

    semid = semget(key, nsems, IPC_CREAT | IPC_EXCL | 0666);

    //printf("Semget() returned %d \n",semid);

    if (semid >= 0) 
    { /* we got it first */
        sb.sem_op = 1; sb.sem_flg = 0;
        arg.val = 1;

        //printf("press return\n"); getchar();

        for(sb.sem_num = 0; sb.sem_num < nsems; sb.sem_num++) 
        { 
            /* do a semop() to "free" the semaphores. */
            /* this sets the sem_otime field, as needed below. */
            if (semop(semid, &sb, 1) == -1) 
            {
                int e = errno;
                semctl(semid, 0, IPC_RMID); /* clean up */
                errno = e;
                return -1; /* error, check errno */
            }
        }

    } 
    else if (errno == EEXIST) 
    { /* someone else got it first */
        int ready = 0;

        semid = semget(key, nsems, 0); /* get the id */
        if (semid < 0) return semid; /* error, check errno */

        /* wait for other process to initialize the semaphore: */
        arg.buf = &buf;
        for(i = 0; i < MAX_RETRIES && !ready; i++) 
        {
            semctl(semid, nsems-1, IPC_STAT, arg);
            if (arg.buf->sem_otime != 0) 
            {
                ready = 1;
            } 
            else 
            {
                sleep(1);
            }
        }
        if (!ready) {
            errno = ETIME;
            return -1;
        }
    } else {
        return semid; /* error, check errno */
    }

    return semid;
}

int main(int argc, char *argv[])
{
    key_t key;
    int shmid;
    int *data;
    int mode,i,pid;
    int semid;
    struct sembuf sb;

    if (argc > 2) {
        fprintf(stderr, "usage: shmdemo [data_to_write]\n");
        exit(1);
    }

    /* make the key: */
    if ((key = ftok("sem_shared.c", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }

    /* connect to (and possibly create) the segment: */
    if ((shmid = shmget(key, SHM_SIZE, 0644 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }
/*	Thus we see that when the writer is faster than the reader, then
	the reader may miss some of the values written into the shared
	memory. Similarly, when the reader is faster than the writer, then
	the reader may read the same values more than once. Perfect 
	inter-process communication requires synchronization between the
	reader and the writer. You can use semaphores to do this.
*/

    if(!fork()){ /* Parent Process */
		/* Semaphore Intialization */
	    sb.sem_num = 0;
	    sb.sem_op = 1;  /* free to use shared memory resource */
	    sb.sem_flg = SEM_UNDO;

	    if ((key = ftok("sem_shared.c", 'J')) == -1) {
		perror("ftok");
		exit(1);
	    }

	    if ((semid = initsem(key, 1)) == -1) {
		perror("initsem");
		exit(1);
	    }
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
		    if (semop(semid, &sb, 1) == -1) {
		    	perror("semop");
		    	exit(1);
	    	    }
	    printf("Parent Process-- Reading from segment: \"%d, %d \"\n", data[0],data[1]);
	    sleep(1);
   	    sb.sem_op = 1; /* free resource */
   		if (semop(semid, &sb, 1) == -1) {
	       		perror("semop");
        		exit(1);
   		}
	    } 	

	    wait(NULL);				//Wait till child completes the execution.
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
		/* Semaphore Intialization */
	    sb.sem_num = 0;
	    sb.sem_op = -1;  /*Allocating the shared memory resource to child for writing */
	    sb.sem_flg = SEM_UNDO;

	   if ((key = ftok("sem_shared.c", 'J')) == -1) {
		perror("ftok");
		exit(1);
	    }

	    if ((semid = initsem(key, 1)) == -1) {
		perror("initsem");
		exit(1);
	    }
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
		    if (semop(semid, &sb, 1) == -1) {				///Semaphore lock on Shared memory
		    	perror("semop");				
		    	exit(1);
	    	    }
	    data[0]=data[0]*2;
	    data[1]=data[1]*3;
	    printf("Child Process: Writing to segment: \"%d, %d \"\n", data[0],data[1]);
	    sleep(1);
 	    sb.sem_op = 1; /* free resource */
    		if (semop(semid, &sb, 1) == -1) {				//Semaphore unlock on Shared memory
        		perror("semop");
        		exit(1);
   		}
	    }

	    /* detach from the segment: */
	    if (shmdt(data) == -1) {
		perror("shmdt");
		exit(1);
	    }     	    

    }
    return 0;
}
