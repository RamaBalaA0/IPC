/*
** server.c -- a stream socket server demo, need to give port number as command line argument..
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

//#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold

#define MAXDATASIZE 100
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, new_fd,numbytes;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN],buf[MAXDATASIZE];			//IPV6 address size.
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP, so no need to keep IP address as an argument.

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
/* getaddrinfo() returns a list of address structures.
              Try each address until we successfully bind.
              If socket (or bind) fails, we (close the socket
              and) try the next address. */

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,	//socket()  creates  an endpoint for communication and returns a descriptor.

                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
/*The bind() function shall assign a local socket address  address  to  a
       socket identified by descriptor sockfd that has no local socket address
       assigned. Sockets created with  the  socket()  function  are  initially
       unnamed; they are identified only by their address family.*/


        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
/*listen()  marks  the  socket referred to by sockfd as a passive socket,
       that is, as a socket that will be used to  accept  incoming  connection
       requests 
*/
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
/*
The accept() function shall extract the first connection on  the  queue
       of  pending  connections, create a new socket with the same socket type
       protocol and address family as the specified socket, and allocate a new
       file descriptor for that socket.*/


        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
/*This  function  converts  the  network  address structure src in the af
       address family into a character string.  The resulting string is copied
       to the buffer pointed to by dst, which must be a non-null pointer.  The
       caller specifies the number of bytes available in this  buffer  in  the
       argument size.
*/
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork())
	 { // this is the child process
            close(sockfd); // child doesn't need the listener
            //if (send(new_fd, "Hello, world!", 13, 0) == -1)
            //    perror("send");
	    while(1)
	    {
	    if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
       		perror("recv");
        	exit(1);
    		}
    	    buf[numbytes] = '\0';
            printf("Server: Received '%s'\n",buf);
	    if(!strcmp(buf,"exit"))
		break;
	    }
	    printf("server: connection from %s ended", s);
	    printf("\n \nserver: waiting for connections...\n");	
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}

