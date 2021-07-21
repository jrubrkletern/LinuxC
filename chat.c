#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <time.h>
#include <poll.h>


#define BACKLOG 10	 // how many pending connections queue will hold

#define MAXDATASIZE 100 // max number of bytes we can get at once



	struct connectionInfo {
		int socketList[256];
	 	char port[256][5];
	 	char ipList[256][INET_ADDRSTRLEN];

	} connectionInfo;

	struct connectionInfo socketInfo;
	struct pollfd myPoll = {STDIN_FILENO, POLLIN|POLLPRI};

	char initialIP[INET_ADDRSTRLEN];
	char removalIP[INET_ADDRSTRLEN];
	char message[256];
	int serverCons;


	// get sockaddr, IPv4 or IPv6:
	void *get_in_addr(struct sockaddr *sa) {
		if (sa->sa_family == AF_INET) {
			return &(((struct sockaddr_in*)sa)->sin_addr);
		}
		return &(((struct sockaddr_in6*)sa)->sin6_addr);
	}


	int printClientList(char port[], int offset) {//The inbound connection handling ends method for printing clients using the provided offset.
		for(int i = 5; i <= 255; i++) {
			if(socketInfo.socketList[i] != -1) {
				printf("%d\t", i+offset-4);
				printf("\t%s\t", socketInfo.ipList[i]);
				printf("%s\t", port);
				printf("\n");
				fflush(stdout);
			}
		}
	}

	int killConnection(int sockfd, int timeout) {//Method to kill connections
		int len;
		int lastSocket;
		for(int i = 1; i <= 255; i++) { //Check to find the socket with the highest index on the outbound connection end, to determine if user wanted to kill something on the inbound end or not.
			if(socketInfo.socketList[i] != -1) {
				lastSocket = i;
			}
		}

		if(sockfd <= lastSocket) { //if targeted index is smaller than our last one, then we kill it here.
			strcpy(message, "Connection to ");
			strcat(message, initialIP);
			strcat(message, " terminated by peer.");
			len = strlen(message);
			send(socketInfo.socketList[sockfd], message, len, 0); //give the peer on the other end of this connection a heads up that we're killing this connection!
			memset(message, 0, sizeof message);
			shutdown(socketInfo.socketList[sockfd], 2); //This connection is no more
			close(socketInfo.socketList[sockfd]);
			if(timeout == 1) {//if we do the connection killing, do this
				printf("Connection to ID %d has been killed.\n", sockfd);
			} else if (timeout == 2) {//if we found out a connection of ours was killed and we're cleaning up after the fact, do this.
				printf("Peer with ID %d has killed this connection.\n", sockfd);
			}
			serverCons--;//one less connection to deal with!
			socketInfo.socketList[sockfd] = -1;
			memset(socketInfo.ipList[sockfd], 0, sizeof socketInfo.ipList[sockfd]);
			memset(socketInfo.port[sockfd], 0, sizeof socketInfo.port[sockfd]);
		} else { //if not? kill on inbound handling end.
			int serverSocket = sockfd - lastSocket;
			strcpy(message, "Kill ");//hardcoded message for the inbound handling end, containing also the index of the connection to kill.
			char snum[3] = {'a', 'a', 'a'};
			snprintf(snum, sizeof(snum), "%d", serverSocket);
			strcat(message, snum);
			len = strlen(message);
			send(socketInfo.socketList[0], message, len, 0);
			memset(message, 0, sizeof message);
			memset(snum, 0, sizeof snum);
			recv(socketInfo.socketList[0], message, sizeof message, 0); //wait for a response back from the server side to know if its done terminating
			memset(message, 0, sizeof message);

		}
		return 1;
	}
	int shutDown() { //turn off everything
		int len;
		int lastSocket;
		for(int i = 1; i <= 255; i++) {
			if(socketInfo.socketList[i] != -1) { //kill every connection we have, same method as in killConnection()
			strcpy(message, "Connection to ");
			strcat(message, initialIP);
			strcat(message, " terminated by peer.");
			len = strlen(message);
			send(socketInfo.socketList[i], message, len, 0);
			memset(message, 0, sizeof message);
			shutdown(socketInfo.socketList[i], 2);
			close(socketInfo.socketList[i]);
			serverCons--;
			socketInfo.socketList[i] = -1;
			memset(socketInfo.ipList[i], 0, sizeof socketInfo.ipList[i]);
			memset(socketInfo.port[i], 0, sizeof socketInfo.port[i]);
			}
		}

			int serverSocket = serverCons; //Now just as in killConnection() we send a message to the inbound handling end, but this time we tell it to kill EVERY connection, then shutdown.
			strcpy(message, "Sill ");
			char snum[3] = {'a', 'a', 'a'};
			snprintf(snum, sizeof(snum), "%d", serverSocket);
			strcat(message, snum);
			len = strlen(message);
			send(socketInfo.socketList[0], message, len, 0);
			memset(message, 0, sizeof message);
			memset(snum, 0, sizeof snum);

			shutdown(socketInfo.socketList[0], 2); //no need for our local process link anymore.
			close(socketInfo.socketList[0]);
			serverCons--;
			socketInfo.socketList[0] = -1;
			memset(socketInfo.ipList[0], 0, sizeof socketInfo.ipList[0]);
			memset(socketInfo.port[0], 0, sizeof socketInfo.port[0]);

		return 1;
	}

	int pingCons() { //check to see if someone gave us a heads up that they killed our connection
		int ping;
		for(int i = 1; i <= 255; i++) {
			if((ping = recv(socketInfo.socketList[i], message, sizeof message, MSG_DONTWAIT)) > 0) {
				//printf("Recieved: %d from %s.\n", ping, socketInfo.ipList[i]);
				killConnection(i, 2); //if so, then remove info from our end as well.
			}

		}
	}

	int sendMessage(int sockfd) { //send a message to a peer
		printf("What would you like to send?\n");
		int condition = 0;
		while(condition == 0) {
			if(poll(&myPoll, 1, 7500)) {//poll to be able to check for new disconnects while taking user input.
				scanf(" %[^\n]s\n", message);
				printf("\n");
				condition = 1;
			} else {
				pingCons();
			}
		}
		int len;
		len = strlen(message);
		len = send(sockfd, message, len, 0);
		if(len == -1) {
			printf("ERROR: Invalid socket detected, removing..\n");//in the off chance that this is somehow invalid, we are aware of that now so lets remove the info from our end.
			for(int i = 0; i < 255; i++) {
				if(socketInfo.socketList[i] == sockfd) {
					sockfd = i;
					i = 256;
				}
			}
			killConnection(sockfd, 0);
		}
		memset(message, 0, sizeof message);
		//printf("send return value: %d\n", len);
	}


	int listConnections() { //list connections for the outbound end
		pingCons();
		printf("Connection ID\tIP\t\tPort\n");
		int offset = 0;
		for(int i = 1; i <= 255; i++) {
			if(socketInfo.socketList[i] != -1) {
				offset = i;//getting our highest index number in which we have a connection, this will be our offset for inbound handling end.
				printf("%d\t", i);
				printf("\t%s\t", socketInfo.ipList[i]);
				printf("%s \t", socketInfo.port[i]);
				printf("\n");
		}

	}
		char snum[3] = {'a', 'a', 'a'};
		strcpy(message, "Offset ");//we send inbound end a special message commanding it to print its connection list, with proper offsets.
		snprintf(snum, sizeof(snum), "%d", offset);
		strcat(message, snum);
		int len = strlen(message);
		send(socketInfo.socketList[0], message, len, 0);
		memset(message, 0, sizeof message);
		memset(snum, 0, sizeof snum);
		recv(socketInfo.socketList[0], message, sizeof message, 0);
		memset(message, 0, sizeof message);
	}

	int newConnection() { //new connection
		bool openSpot = false;
		int openIndex;
		for(int i = 1; i <= 255; i++) { //do we have a spot open to accept another connection?
			if(socketInfo.socketList[i] == -1) {
				openSpot = true;
				openIndex = i; //we can keep this for later, if the connection ends up working then we'll store info in this index, because its empty.
				i = 256;

			}
		}
		if(openSpot == false) {
			printf("Error, maxed out connections, cannot add more!\n");
		}
		char port[5];
		printf("Enter server's port.\n");
		int condition = 0;
		while(condition == 0) {
			if(poll(&myPoll, 1, 7500)) {
				scanf("%s", port);
				printf("\n");
				condition = 1;
			} else {
				pingCons();
			}
		}
		condition = 0;
		char address[INET_ADDRSTRLEN];
		printf("Enter server address.\n");
		while(condition == 0) {
			if(poll(&myPoll, 1, 7500)) {
				scanf("%s", address);
				printf("\n");
				condition = 1;
			} else {
				pingCons();
			}
		}
		if(strcmp(address, socketInfo.ipList[0]) == 0 && strcmp(port, socketInfo.port[0]) == 0) { //make sure they arent trying to connect our processes!
			printf("Error connecting, cannot connect to self!\n");
			return 0;
		}
		for(int i = 1; i < 255; i++) { //make sure we dont already have this connection.
			if(strcmp(address, socketInfo.ipList[i]) == 0 && strcmp(port, socketInfo.port[i]) == 0) {
				printf("Error connecting, duplicate connection already exists!\n");
				return 0;
			}
		}
		int sockfd, numbytes;
		struct addrinfo hints, *servinfo, *p;
		int rv;
		char s[INET6_ADDRSTRLEN];
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		if ((rv = getaddrinfo(address, port, &hints, &servinfo)) != 0) {//obtaining info for a new connection using the address and port provided.
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}


		// loop through all the results and connect to the first we can
		for(p = servinfo; p != NULL; p = p->ai_next) {
			if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
				perror("client: socket");
				continue;
			}
			if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
				perror("client: connect");
				close(sockfd);
				continue;
			}
			break;
		}
		if (p == NULL) {
			fprintf(stderr, "client: failed to connect\n");
			return 2;
		}
		inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),	s, sizeof s);
		printf("Connecting to %s\n\n", s);

		for(int i = 0; i <= INET_ADDRSTRLEN-1; i++) {
			socketInfo.ipList[openIndex][i] = address[i];
		}
		socketInfo.socketList[openIndex] = sockfd;
		strcpy(socketInfo.port[openIndex], port);
		serverCons++;
		return 1;
	}
	int main(int argc, char *argv[]) {
		if (argc != 2) {
				fprintf(stderr,"Usage: client port\n");
				exit(1);
		}
		char port[5];
		strcpy(port, argv[1]);
		char hostname[256];
	 	gethostname(hostname, sizeof hostname);
		struct addrinfo myHints, *res, *v;
		int status;
		memset(&myHints, 0, sizeof myHints);
		myHints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
		myHints.ai_socktype = SOCK_STREAM;
		if ((status = getaddrinfo(hostname, NULL, &myHints, &res)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
			return 2;
		}
		for(v = res;v != NULL; v = v->ai_next) {
			void *addr;
			// get the pointer to the address itself,
			// different fields in IPv4 and IPv6:
			if (v->ai_family == AF_INET) { // IPv4
				struct sockaddr_in *ipv4 = (struct sockaddr_in *)v->ai_addr;
				addr = &(ipv4->sin_addr);
				// convert the IP to a string and print it:
				inet_ntop(v->ai_family, addr, initialIP, sizeof initialIP);
			}
		}
		freeaddrinfo(res); // free the linked list
		pid_t pid;
		 if((pid = fork()) == 0) {
			 for(int i=0; i <= 255; i++) {//setting servers entire socketList to -1, or "empty".
				 socketInfo.socketList[i] = -1;
			 }
			fd_set master;    // master file descriptor list
			fd_set read_fds;  // temp file descriptor list for select()
			int fdmax;        // maximum file descriptor number
			int listener;     // listening socket descriptor
			int newfd;        // newly accept()ed socket descriptor
			struct sockaddr_storage remoteaddr; // client address
			socklen_t addrlen;	//length of address
			char buf[256];    // buffer for client data
			int nbytes;				//number of bytes, used in returning recv() values.
			int firstCon = 1; //Used for pairing local client/server, user cannot make these two interact.
			char remoteIP[INET6_ADDRSTRLEN]; //When someone connects to us, we will store their IP here temporarily.
			int yes=1;        // for setsockopt() SO_REUSEADDR, below
			int i, j, rv;
			int pairfd;				//pairing local process
			struct addrinfo hints, *ai, *p;
			FD_ZERO(&master);    // clear the master and temp sets
			FD_ZERO(&read_fds);
		 	memset(&hints, 0, sizeof hints);	// get us a socket and bind it
		 	hints.ai_family = AF_UNSPEC;
		 	hints.ai_socktype = SOCK_STREAM;

		 	if ((rv = getaddrinfo(initialIP, port, &hints, &ai)) != 0) {
		 		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		 		exit(1);
		 	}

		 	for(p = ai; p != NULL; p = p->ai_next) {
		     	listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		 		if (listener < 0) {
		 			continue;
		 		}

				setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); // lose the pesky "address already in use" error message
		 		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
		 			close(listener);
		 			continue;
		 		}

		 		break;
		 	}


		 	if (p == NULL) {// if we got here, it means we didn't get bound
		 		fprintf(stderr, "Server: failed to bind\n");
				kill(getppid(), 9);
		 		exit(2);
		 	}

		 	freeaddrinfo(ai); // all done with this

		  if (listen(listener, 10) == -1) { // listen
		  	perror("listen");
		    exit(3);
		  }

		     FD_SET(listener, &master); // add the listener to the master set
		     fdmax = listener; // keep track of the biggest file descriptor; So far, it's this one

		     for(;;) { // main server loop
		         read_fds = master; // copy master into read_fds
		         if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) { //select function, for handling from multiple peers
		             perror("select");
		             exit(4);
		         }


		         for(i = 0; i <= fdmax; i++) { // run through the existing connections looking for data to read
		             if (FD_ISSET(i, &read_fds)) { // we got one!!
		                 if (i == listener) { //check to see if the fd we are on at the moment is the listener
		                     addrlen = sizeof remoteaddr; // handle new connections
												 bool openSpot = false;
												 int openIndex;
												 for(int q = 4; q <= 255; q++) { //do we have a spot open to accept another connection?
													 if(socketInfo.socketList[q] == -1) {
														 openSpot = true;
														 openIndex = q; //we can keep this for later, if the connection ends up working then we'll store info in this index, because its empty.
														 q = 256;
													 }
												 }
												 if(openSpot == true) {
												 newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
											 } else {
													printf("No more open slots to handle connections!\n");
													shutdown(newfd, 2);
													close(newfd);
													newfd = -1;

												}
												 if (newfd == -1) {
												     perror("accept");
												 } else {
												     FD_SET(newfd, &master); // add to master set

												     if (newfd > fdmax) {    // keep track of the max
												         fdmax = newfd;
												     }
														 if(firstCon == 0) {		//Prints inbound messages not from local process.
								             		printf("New inbound connection from %s on "
																	"socket %d\n", inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN), newfd);
															} else {
																firstCon = 0;
																pairfd = newfd; //we have our local process noted!
															}

															strcpy(remoteIP, inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN)); //getting this into a readable format to store.

															for(int j = 0; j <= INET_ADDRSTRLEN-1; j++) {
																socketInfo.ipList[openIndex][j] = remoteIP[j];
															}
															socketInfo.socketList[openIndex] = newfd;
		               				}
		               } else {
		                     // handle data from a client
		                     if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
		                         // got error or connection closed by client

		                         if (nbytes == 0) {
		                             // connection closed

																 for(int k = 5; k <= fdmax; k++) {//nbytes which stores recv() return value is 0, so we're hunting through our storage list
																	 if(i == socketInfo.socketList[k]) {//to find the right index where said peer's information is stored, and then we erase it.
																		 socketInfo.socketList[k] = -1;
																		 printf("Inbound IP %s hung up.\n", socketInfo.ipList[k]);
																		 memset(socketInfo.ipList[k], 0, sizeof socketInfo.ipList[k]);
																		 memset(socketInfo.port[k], 0, sizeof socketInfo.port[k]);
																		 k = fdmax; //If we're in this conditional that means we found the socket of interest and already did our work, so we can leave the loop now!

																	 }

																 }
		                         } else {
		                             perror("recv");
		                         }

														 close(i); // bye!
		                         FD_CLR(i, &master); // remove from master set
		                     } else {

													 		//We are communicating with our local process, if we get sent a message by our local process then we do not print the message, we perform special operations depending on the message.
															if(i == pairfd) {

																if(buf[0] == 'K') {//Our local process has sent us information about a connection to kill.
																	char killSocket[3];
																	for(int y = 0; y < 3; y++) {//here we decode and find out which socket is to be killed.
																		if(buf[y+5] != 'a')
																			killSocket[y] = buf[y+5];
																	}
																	int kill = atoi(killSocket) + 4; //First index is 4 which is our local process hidden away from the user, so the offset for this is 4.
																		strcpy(buf, "blahblahblah"); //We send a message to the peer at the other end of this connection, to let them know we are killing the connection.
																		int len;
																		len = strlen(buf);
																		send(socketInfo.socketList[kill], buf, nbytes, 0);
																		memset(message, 0, sizeof message);
																if(socketInfo.socketList[kill] != -1) {//We make sure that nothing somehow happened and that this index does indeed contain a connection with info.
																		socketInfo.socketList[kill] = -1;
																		printf("Inbound connection to %s terminated.\n", socketInfo.ipList[kill]);
																		memset(socketInfo.ipList[kill], 0, sizeof socketInfo.ipList[kill]);
																		memset(socketInfo.port[kill], 0, sizeof socketInfo.port[kill]);
																		close(kill); // bye!
																		FD_CLR(kill, &master); // remove from master set
																		memset(buf, 0, sizeof buf);
																		strcpy(buf, "aaaaaa");
																		int len;
																		len = strlen(buf);
																		send(socketInfo.socketList[4], buf, len, 0);

																	} else {
																		printf("Invalid Connection ID!\n");//in the case that this connection never existed.
																	}
															} else if(buf[0] == 'S') {//This scenario indicates that our local process is telling us to shut everything down, kill all connections and exit.
																for(int g = 0; g <= 255; g++) {
																if(socketInfo.socketList[g] != -1) {
																	strcpy(buf, "blahblahblah");
																	int len;
																	len = strlen(buf);
																	send(socketInfo.socketList[g], buf, len, 0);
																	memset(message, 0, sizeof message);
																	socketInfo.socketList[g] = -1;
																	//printf("Connection to %s terminated.\n", socketInfo.ipList[g]);
																	memset(socketInfo.ipList[g], 0, sizeof socketInfo.ipList[g]);
																	memset(socketInfo.port[g], 0, sizeof socketInfo.port[g]);
																	close(g); // bye!
																	FD_CLR(g, &master); // remove from master set
																}
															}
															return 0;
														} else {//the third and final possible scenario from our local process, we need to print info we have on our end, the client has provided us with the proper offset to use as well to seemlessly display info.
																char offset[3];
																for(int y = 0; y < 3; y++) {
																	if(buf[y+7] != 'a')
																		offset[y] = buf[y+7];
																}
																	int print = atoi(offset);
																	printClientList(port, print);
																	memset(buf, 0, sizeof buf);
																	strcpy(buf, "aaaaaa");
																	int len;
																	len = strlen(buf);
																	send(socketInfo.socketList[4], buf, len, 0);
																}
															} else {
															printf("Message recieved from: %s\n", socketInfo.ipList[i]); //Print a message from an inbound connection with proper info included!
															printf("On port: %s\n", port);
															printf("Message: %s\n", buf);
															fflush(stdout);
															memset(buf, 0, sizeof buf);
														}
														 // we got some data from a client
														 memset(message, 0, sizeof message);
		                     }
		                 } // END handle data from client
		             } // END got new incoming connection
		         } // END looping through file descriptors
		     } // END for(;;)--and you thought it would never end!
			 } //fork inbound side
		for(int i=1; i <= 255; i++) {//client side, setting all indices to empty, because when adding/removing we specifically check the list of sockets.
			socketInfo.socketList[i] = -1;
		}
		serverCons = 0; //keep track of how many connections we have in total.
		int sockfd, numbytes;

		//first we're going to intialize a 'hidden connection' for communication, the user CANNOT actively interact with this connection in any way.
		struct addrinfo hints, *servinfo, *p;
		int rv;
		char s[INET6_ADDRSTRLEN];
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		if ((rv = getaddrinfo(initialIP, port, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			exit(6);
		}


		// loop through all the results and connect to the first we can
		for(p = servinfo; p != NULL; p = p->ai_next) {
			if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
				perror("client: socket");
				continue;
			}

			if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
				perror("client: connect");
				close(sockfd);

			}

			break;
		}

		if (p == NULL) {
			fprintf(stderr, "client: failed to connect\n");
			exit(5);
		}

		inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),	s, sizeof s);

		for(int i = 0; i <= INET_ADDRSTRLEN-1; i++) {
							socketInfo.ipList[0][i] = initialIP[i];
						}
						socketInfo.socketList[0] = sockfd;
						strcpy(socketInfo.port[0], port);

		int choice = 0; //keep track of what the user wants to do.
		bool notShutDown = true; //when we shutdown, this will be set to false and we will be done!
		int sBytes; //we hold scanf return values in here, helps deal with faulty input.
		while(notShutDown == true) {
			pingCons();
			printf("What would you like to do?\n");
			printf("1. Help\n");
			printf("2. My IP\n");
			printf("3. My Port\n");
			printf("4. Connect to a server\n");
			printf("5. list\n");
			printf("6. terminate\n");
			printf("7. Send message\n");
			printf("8. Exit");
			printf("\n");
			while(choice > 8 || choice < 1) {//choose only between 1 and 8 inclusive.
				if(poll(&myPoll, 1, 7500)) {//using poll() to scan but also to check to make sure connections we made are still active to update the list.
					sBytes = scanf("%d", &choice);
					printf("\n");
					if(choice > 8 || choice < 1) {
	          printf("Invalid choice!\n");
	          if(sBytes == 0) {
	            int x;
	            while ( (x = getchar()) != EOF && x != '\n' );
	          }
	        }
		    }
		    else
		    {
					pingCons(); //Here we 'ping' cons after every 7500 ms (7.5 seconds)
		    }

		}
			switch(choice) {
		   		case 1: //help
					printf("This program allows you to chat with other computers in your network,\n");
					printf("by using their IP and a proper port to connect to.\n");
					printf("Option 2 displays the IP of the computer, \nthis is the IP others will connect to this program from.\n");
					printf("Option 3 displays the port specified at initial execution, \nthis is the port others would use to connect to this program.\n");
					printf("Option 4 allows you to connect to another peer by specifying first the port and then the IP.\n");
					printf("Option 5 lists all the connections you have to other users, \nas well as what connections other users have to you.\n");
					printf("Option 6 will terminate any connection specified, assuming the Connection ID inputted is valid. \nConnection IDs are obtained from option 5.\n");
					printf("Option 7 will send a message of your choice to the specified Connection ID, \nassuming the Connection ID is valid. Connection IDs are obtained from option 5.\n");
					printf("Option 8 ends all connections associated with this process and shuts down.\n");
		      break;

		   		case 2: //print your IP
					printf("Your IP is: %s\n\n", initialIP);
		      break;

					case 3: //print your port
					printf("Your port is: %s\n\n", port);
					break;

					case 4: //connect to a server
					newConnection();
					break;

					case 5: //list outbound and inbound connections
					listConnections();
					break;

		 			case 6: //terminate a connection
					choice = 0;
					printf("Select connection to terminate.\n");
					while(choice < 1 || choice > 510) {
						if(poll(&myPoll, 1, 7500)) {
							sBytes = scanf("%d", &choice);
							printf("\n");
							if(choice > 510 || choice < 1) {
								printf("Invalid choice!\n");
								if(sBytes == 0) {
									int x;
									while ( (x = getchar()) != EOF && x != '\n' );
									break;
								}
							}
				    } else {
							pingCons();
				    }
					}
						if(choice != 0) {
							killConnection(choice, 1);
						} else {
							printf("Invalid Connection Id!\n\n");
						}
					break;

		 			case 7: //sending a message
					choice = 0;
					printf("Select connection id to send a message to.\n");
					while(choice > 510 || choice < 1) {
						if(poll(&myPoll, 1, 7500)) {
							sBytes = scanf("%d", &choice);
							printf("\n");
							if(choice > 510 || choice < 1) {
								printf("Invalid choice!\n");
								if(sBytes == 0) {
									int x;
									while ( (x = getchar()) != EOF && x != '\n' );
									break;
								}
							}
						} else {
							pingCons();
						}
				}
				if (choice > 255) {
						choice = 0;
				}
						if(choice != 0 && socketInfo.socketList[choice] != -1) {//We check to see if this index has information in it before sending, and if its not 0!
				 			sendMessage(socketInfo.socketList[choice]);
					} else {
						printf("You may only send messages to peers YOU have connected to.\n\n");
				}
					break;
					case 8:
					choice = 0;
					printf("Are you sure you want to shutdown? 1 = yes, 2 = no\n");
					while(choice < 1 || choice > 2) {
						if(poll(&myPoll, 1, 7500)) {
							sBytes = scanf("%d", &choice);
							printf("\n");
							if(choice < 1 || choice > 2) {
								printf("Invalid input!\n");
								if(sBytes == 0) {
									int x;
									while ( (x = getchar()) != EOF && x != '\n' );
									break;
								}
							}
						} else {
							pingCons();
						}

				}
		      if(choice == 1) {
						shutDown();//This method will close everything down, and tell the side thats handling inbound connections to cut everything and close as well.
					notShutDown = false;
					printf("Shutting down..\n");
					sleep(1);
					printf("Shutdown complete.\n");
					return 0;
					}
					 else if(choice == 2) {
						printf("Aborting shutdown..\n");
				}
					break;
		}
			choice = 0;
	}

		return 0;
	}
