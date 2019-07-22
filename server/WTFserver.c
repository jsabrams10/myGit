/*JOHN ABRAMS (jsa109)
 *CHRIS ZACHARIAH (cvz2)
 *4/29/2019
 *Asst3 - Where's The File? (WTF)
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <openssl/sha.h>

// main function that deals with the threads its commands
void* doSomething(void* socket);

//mutex used in the the main fuction
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // use this mutuex in the doSomething function

int main(int argc, char* argv[]){

    // check the number of arguments
    if (argc != 2) {
        printf("Server: ERROR: too few/many arguments\n");
        return 1;
    }
    
    // open the socket
    int socketNum;
    socketNum = socket(AF_INET, SOCK_STREAM, 0);
    if (socketNum < 0) {
        printf("Server: ERROR opening the socket\n");
        return 1;
    } else {
        printf("Server: Socket successfully opened\n");
    }
    
    // build the mutex
    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("Mutex failed to build.\n");
        return 1;
    }
    
    // check the port number to make sure that it is within the range of [8000-65535] (inclusive)
    int PORT = atoi(argv[1]);
    if ((PORT >= 8000) && (PORT <= 65535)) {
        printf("Port number is VALID.\n");
    } else {
        printf("Port number is INVALID. Please use numbers between 8000 and 65535 inclusive for safety purposes\n");
        return 1;
    }
    
    // Initialize the socket struct (serverAdd)
    struct sockaddr_in serverAdd;
    bzero((char *)&serverAdd, sizeof(serverAdd));
    
    serverAdd.sin_family = AF_INET;                 // normal communication protocol
    serverAdd.sin_addr.s_addr = INADDR_ANY;         // use available IP address
    serverAdd.sin_port = htons(PORT);               // use the specific port number
	
    //Need to bind the address
    int binding = bind(socketNum,(struct sockaddr *)&serverAdd, sizeof(serverAdd));
    if (binding < 0) {
        printf("Server: ERROR while binding\n");
        return 1;
    } else {
        printf("Server: Address bound\n");
    }
    
    //make the server listen for clients
    int listening = listen(socketNum,80);           // can accept up to 80 clients
    if (listening < 0) {
        printf("Server: ERROR on listen.\n");
        return 1;
    } else {
        printf("Server: Server is now listening.\n");
    }
    struct sockaddr_in clientAdd;
    int clientAddSize = sizeof(clientAdd);
    int thread;
   
    
    //make an array of 80 threads that can be used to handle the clients
    int threadNumb = 0;
    pthread_t clients[80];
    int newClient;
    
    while (1) {
        
        //check to see if the client socket can be connected
        newClient = accept(socketNum,(struct sockaddr *)&clientAdd,(socklen_t *)&clientAddSize);
        if (newClient < 0) {
            printf("Server: Error accepting the client\n");
            return 1;
        } else {
            printf("Server: Client accepted and will be dealt with shortly.\n");
            
            //put the client in the next available spot in the array of threads
            thread = pthread_create(&clients[threadNumb], NULL , doSomething ,(void*)&newClient);
            if (thread < 0) {
                printf("Server: Error making thread.\n");
                return 1;
            } else {
                printf("Server: Client handled.\n");
            }
            // once the thread has done the job, join it and then move to the next index to put in more clients
            pthread_join(clients[threadNumb], NULL);
            ++threadNumb;
            
            // if the next client is #80, then it would go out of bounds
            // so assuming that client thread #0 is done with its job, then loop around to the front of the array
            if (threadNumb == 80) {
                threadNumb = 0;
            }
            printf("Server: Server is now listening.\n");
            
        }
        
    } // ends the main while loop
     pthread_mutex_destroy(&mutex);
    return 1;
} // end of the main() function

/*
 Main function that handles the clients
 */
void* doSomething( void* socket) {
    
    // locking everything
    pthread_mutex_lock (&mutex);
    
    int sock = *(int*)socket;
    int n;
    char commandBuff[2];
    
    // read what command the client wants to run
    bzero(commandBuff,2);
    n = read(sock,commandBuff,1);
    if (n < 0) {
        printf("Server: ERROR reading from socket\n");
        exit(1);
    }
    
    char command = commandBuff[0];
    printf("Server: Command '%c' received from client.\n",command);
    
    //variables used to talk with clients and do other processes
    char buffer[100000];
    int good;
    
    int i;
    int numDigits;
    char* storeDigits;
    
    int sizeOfProjectName;
    char* projectName;
    int count;
    
    char* message;
    
    DIR* pDir;
    struct dirent* pEnt;
    int found;
    char c;
    
    int ManifestFD;
    int b;
    
    switch(command){
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------
        case 'A': //add <projectname> <filename>
             // *** THIS CASE WILL NOT SHOW UP *** //
            break;
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
        case 'D': //destroy <projectname>
            
            break;
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------
        case 'E': //create <projectname>
            
            // get the project name from the client => format => #ofChars:name
            bzero(buffer,100000);
            good = read(sock,buffer,99999);
            if (good < 0) {
                printf("Server: Error reading from the clinet.\n");
                break;
            }
            printf("Server: Received the project name from the client.\n");
            
            // count the number of digits
            i = 0;
            numDigits = 0;
            while(buffer[i] != ':') {
                ++numDigits;
                ++i;
            }
            
            //make an array to store the number
            storeDigits = (char*)malloc((numDigits+1)*sizeof(char));
            
            //store the digits into an array
            i = 0;
            while(i < numDigits) {
                storeDigits[i] = buffer[i];
                ++i;
            }
            storeDigits[numDigits] = '\0';
            
            //now that you have the size of the project name, make a string to store it and then zero out the buffer
            sizeOfProjectName = atoi(storeDigits);
            projectName = (char*)malloc((sizeOfProjectName+1)*sizeof(char));
            
            //store the project name in the array
            i = numDigits+1;
            count = 0;
            while(count < sizeOfProjectName) {
                projectName[count] = buffer[i];
                ++count;
                ++i;
            }
            projectName[sizeOfProjectName] = '\0';
        
            // 1 = found , 0 = not found
            found = 0;
            
            // search through the current directory for the project directory
            pDir = opendir(".");
            if(pDir != NULL){
                while((pEnt = readdir(pDir)) != NULL) {
                    // project directory is found
                    if( (pEnt -> d_type == DT_DIR) && (strcmp(pEnt->d_name,projectName) == 0) ) {
                        found = 1;
                    }
                } // ends the while loop
                closedir(pDir);
            } else {
                printf("Server: Error when looking through current directory.\n");
                break;
            }
            
            // server will send a message to the client whether it found the project or not
            bzero(buffer,100000);
            
            if (found == 0) {
                message = "Project not found. Creating new directory and shortly sending over .Manifest";
                write(sock,message,strlen(message));
                
                //create the project directory
                snprintf(buffer,100000, "./%s",projectName);
                mkdir(buffer,0777);
                
                //make a .Manifest and it in the new project directory and also send it over to the client
                bzero(buffer,100000);
                snprintf(buffer,100000, "./%s/.Manifest",projectName);
                
                //write into the .Manifest => first line => "0 \n"
                ManifestFD = open(buffer, O_RDWR | O_APPEND | O_CREAT, 0644);
                if (ManifestFD < 0) {
                    printf("Server: Error in creating a .Manifest for %s\n",projectName);
                } else {
                    c = '0';                    // version number for the project
                    write(ManifestFD,&c,1);
                    c = '\n';                   // newLine
                    write(ManifestFD,&c,1);
                    
                    close(ManifestFD);
                    
                    //now send the contents of the .Manifest to the client
                    bzero(buffer,100000);
                    snprintf(buffer,100000, "0\n");
                    write(sock,buffer,strlen(buffer));
                    printf("Server: Sent the .Manifest contents to the client.\n");
                }
            } else {
                message = "Project found. Cannot create a new project with the same name.";
                write(sock,message,strlen(message));
            }
            
            free(storeDigits);
            free(projectName);
            
            break;
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        case 'F': //configure <IP/hostname> <port>
            // *** THIS CASE WILL NOT SHOW UP *** //
            break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        case 'G': //upgrade <projectname>
            
            break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        case 'H': //history <projectname>
            
            break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        case 'K': //checkout <projectname>
            
            break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        case 'L': //rollback <projectname> <version>
            
            break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        case 'M': //commit <projectname>
            
            // get the project name from the client => format => #ofChars:name
            bzero(buffer,100000);
            good = read(sock,buffer,99999);
            if (good < 0) {
                printf("Server: Error reading from the clinet.\n");
                break;
            }
            printf("Server: Received the project name from the client.\n");
            
            // count the number of digits
            i = 0;
            numDigits = 0;
            while(buffer[i] != ':') {
                ++numDigits;
                ++i;
            }
            
            //make an array to store the number
            storeDigits = (char*)malloc((numDigits+1)*sizeof(char));
            
            //store the digits into an array
            i = 0;
            while(i < numDigits) {
                storeDigits[i] = buffer[i];
                ++i;
            }
            storeDigits[numDigits] = '\0';
            
            //now that you have the size of the project name, make a string to store it and then zero out the buffer
            sizeOfProjectName = atoi(storeDigits);
            projectName = (char*)malloc((sizeOfProjectName+1)*sizeof(char));
            
            //store the project name in the array
            i = numDigits+1;
            count = 0;
            while(count < sizeOfProjectName) {
                projectName[count] = buffer[i];
                ++count;
                ++i;
            }
            projectName[sizeOfProjectName] = '\0';
            
            // 1 = found , 0 = not found
            found = 0;
            
            // search through the current directory for the project directory
            pDir = opendir(".");
            if(pDir != NULL){
                while((pEnt = readdir(pDir)) != NULL) {
                    // project directory is found
                    if( (pEnt -> d_type == DT_DIR) && (strcmp(pEnt->d_name,projectName) == 0) ) {
                        found = 1;
                    }
                } // ends the while loop
                closedir(pDir);
            } else {
                printf("Server: Error when looking through current directory.\n");
                break;
            }
            
            if (found == 0) {
                message = "Project not found on the server.";                   // server does not have the project and its files
                write(sock,message,strlen(message));
                
            } else {
                message = "Project found. Will shortly send over .Manifest.";   // server has the project directory and will send over
                                                                                // the .Manifest
                write(sock,message,strlen(message));
                
                //make the pathfile to the .Manifest of the project
                bzero(buffer,100000);
                snprintf(buffer,100000, "./%s/.Manifest",projectName);
                
                // open the .Manifest and put the contents into the buffer
                ManifestFD = open(buffer, O_RDWR | O_APPEND | O_CREAT, 0644);
                if (ManifestFD < 0) {
                    printf("Server: Error in opening the .Manifest for %s\n",projectName);
                } else {
                    //copy the contents of the .Manifest over to the buffer to send to the client
                    good = 1;
                    b = 0;
                    while((b <= 99999) && (good == 1)) {         // read the chars in the .Manifest and put it into the buffer
                        good = read(ManifestFD,&c,1);
                        buffer[b] = c;
                        ++b;
                    }
                    buffer[b] = '\0';
                    close(ManifestFD);
                    write(sock,buffer,strlen(buffer));          // send the contents of the .Manifest to the client
                    printf("Server: Sent the .Manifest contents to the client.\n");
                }
            }
            
            // there is more to do with the client
            
            
            
            free(storeDigits);
            free(projectName);
            
            break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        case 'P': //push <projectname>
            
            break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        case 'R': //remove <projectname> <filename>
            
            break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        case 'U': //update <projectname>
            
            break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        case 'V': //currentversion <projectname>
            
            // get the project name from the client => format => #ofChars:name
            bzero(buffer,100000);
            good = read(sock,buffer,99999);
            if (good < 0) {
                printf("Server: Error reading from the clinet.\n");
                break;
            }
            printf("Server: Received the project name from the client.\n");
            
            // count the number of digits
            i = 0;
            numDigits = 0;
            while(buffer[i] != ':') {
                ++numDigits;
                ++i;
            }
            
            //make an array to store the number
            storeDigits = (char*)malloc((numDigits+1)*sizeof(char));
            
            //store the digits into an array
            i = 0;
            while(i < numDigits) {
                storeDigits[i] = buffer[i];
                ++i;
            }
            storeDigits[numDigits] = '\0';
            
            //now that you have the size of the project name, make a string to store it and then zero out the buffer
            sizeOfProjectName = atoi(storeDigits);
            projectName = (char*)malloc((sizeOfProjectName+1)*sizeof(char));
            
            //store the project name in the array
            i = numDigits+1;
            count = 0;
            while(count < sizeOfProjectName) {
                projectName[count] = buffer[i];
                ++count;
                ++i;
            }
            projectName[sizeOfProjectName] = '\0';
            
            // 1 = found , 0 = not found
            found = 0;
            
            // search through the current directory for the project directory
            pDir = opendir(".");
            if(pDir != NULL){
                while((pEnt = readdir(pDir)) != NULL) {
                    // project directory is found
                    if( (pEnt -> d_type == DT_DIR) && (strcmp(pEnt->d_name,projectName) == 0) ) {
                        found = 1;
                    }
                } // ends the while loop
                closedir(pDir);
            } else {
                printf("Server: Error when looking through current directory.\n");
                break;
            }
            
            if (found == 0) {
                message = "Project not found on the server.";                   // server does not have the project and its files
                write(sock,message,strlen(message));
               
            } else {
                message = "Project found. Will shortly send over .Manifest.";   // server has the project directory and will send over
                                                                                // the .Manifest
                write(sock,message,strlen(message));
                
                //make the pathfile to the .Manifest of the project
                bzero(buffer,100000);
                snprintf(buffer,100000, "./%s/.Manifest",projectName);
                
                // open the .Manifest and put the contents into the buffer
                ManifestFD = open(buffer, O_RDWR | O_APPEND | O_CREAT, 0644);
                if (ManifestFD < 0) {
                    printf("Server: Error in opening the .Manifest for %s\n",projectName);
                } else {
                    
                    // skip over the first line
                    c = '\0';
                    while (c != '\n') {
                        read(ManifestFD,&c,1);
                    }
                    
                    //just need to send over the current file data
                    good = 1;
                    b = 0;
                    while((b <= 99999) && (good == 1)) {         // read the chars in the .Manifest and put it into the buffer
                        good = read(ManifestFD,&c,1);
                        buffer[b] = c;
                        ++b;
                    }
                    buffer[b] = '\0';
                    close(ManifestFD);
                    write(sock,buffer,strlen(buffer));          // send the contents of the .Manifest to the client
                    printf("Server: Sent the .Manifest contents to the client.\n");
                }
            }
            
            free(storeDigits);
            free(projectName);
            
            break;
    } // end of the switch statements
    
    // unlocking everything
    pthread_mutex_unlock (&mutex);
    
    return 0;
} // ends the doSomething() method

/*
 
 // *** old code for the main while loop in the server to handle clients *** //
 
 int thread;
 pthread_t thread_id;
 
 newClient = accept(socketNum,(struct sockaddr *)&clientAdd,(socklen_t *)&clientAddSize);
 if (newClient < 0) {
 printf("Error accepting the client\n");
 return 1;
 } else {
 printf("Client accepted\n");
 }
 while(newClient >= 0) {
 printf("Client accepted\n");
 
 thread = pthread_create(&thread_id, NULL , doSomething , (void*)&newClient);
 if (thread < 0) {
 printf("Error making thread.\n");
 return 1;
 } else {
 printf("Client being handled.\n");
 }
 
 } // ends the while loop
 
*/

/*
 
 // *** old code for the main while loop in the server to handle clients *** //
 
 struct sockaddr_in clientAdd;
 int clientAddSize = sizeof(clientAdd);
 
 int newClient;
 int forking;
 
 // need this to never stop...
 while(1) {
 newClient = accept(socketNum,(struct sockaddr *)&clientAdd,(socklen_t *)&clientAddSize);
 if (newClient < 0) {
 printf("Error accepting the client\n");
 return 1;
 } else {
 printf("Client accepted\n");
 }
 
 //make a child process
 forking = fork();
 
 if (forking < 0) {
 printf("Error in fork.");
 }
 if (forking == 0) {
 close(socketNum);
 doSomething(newClient);
 exit(0);
 } else {
 close(newClient);
 }
 } // ends the while loop
 
 */

/*
 
 thread = pthread_create(&thread_id, NULL , doSomething , (void*)&newClient);
 if (thread < 0) {
 printf("Error making thread.\n");
 return 1;
 } else {
 printf("Client handled.\n");
 }
 
 */


/*
 
 // *** old code for the main while loop in the server to handle clients *** //
 
 // want to add the client into a LL of other clients, they will be dealt with one by one by the server
 struct node* newClientNode = (struct node*)malloc(sizeof(struct node));
 ptr = newClientNode;
 //ptr->client_ID = newClient;
 ptr->next = NULL;
 if (root == NULL) {
 root = newClientNode;
 ptr = NULL;
 } else {
 rootPtr = root;
 rootPrev = root;
 while(rootPtr != NULL) {
 rootPrev = rootPtr;
 rootPtr = rootPtr->next;
 }
 rootPrev->next = newClientNode;
 }
 
 // now handle the client the root points to
 while (root != NULL) {
 thread = pthread_create(&root->client_ID, NULL , doSomething , (void*)root->client_ID);
 if (thread < 0) {
 printf("Server: Error making thread.\n");
 return 1;
 } else {
 printf("Server: Client handled.\n");
 }
 printf("Server: Server is now listening.\n");
 rootPtr = root;
 pthread_join(root->client_ID, NULL);
 root = root->next;
 free(rootPtr);
 
 
 
 } // ends the inner while loop
 
 
 */

//pthread_t thread_id;
//struct node* root = NULL;
//struct node* rootPtr = NULL;
//struct node* rootPrev = NULL;
//struct node* ptr = NULL;

/*
// most likely not going to use this node
struct node {
    pthread_t client_ID;
    struct node* next;
};
 */
