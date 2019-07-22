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
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <openssl/sha.h>

/*HELPER FUNCTION GENERATING HASH USING SHA256
 *
 *INPUT:
 *    -STRING USED TO BUILD HASH (CONTENTS OF A FILE)
 *    -STRING USED TO STORE HASH
 */
void setHash(char *string, char buffer[65]){
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, string, strlen(string));
    SHA256_Final(hash, &sha256);
    
    int index = 0;
    
    for(index = 0; index < SHA256_DIGEST_LENGTH; index++){
        
        sprintf(buffer + (index * 2), "%02x", hash[index]);
    }
    
    buffer[64] = 0;
}

/*ADD FUNCTION - VALIDATES USER REQUEST AND UPDATES MANIFEST ACCORDINGLY
 *
 *INPUT:
 *    -STRING WITH NAME OF PROJECT
 *    -STRING WITH PATH TO FILE (MAY OR MAY NOT CONTAIN PROJECT NAME)
 */
void add(char* projectname, char* filename){
    
    //CHECK IF PROJECT EXISTS
    DIR* dir = opendir(projectname);
    
    if(dir){    closedir(dir);}
    
    else{
        
        printf("Client: ERROR: project directory not found\n");
        return;
    }
    
    //GENERATE FILEPATH
    char* filepath = malloc(strlen(projectname) + 1 + strlen(filename) + 5);
    memset(filepath, '\0', strlen(projectname) + 1 + strlen(filename) + 5);
    
    strcat(filepath, projectname);
    strcat(filepath, "/");
    strcat(filepath, filename);
    
    //CHECK IF FILE EXISTS WITH BOTH FILENAME AND FILEPATH
    int fd = open(filename, O_RDONLY);
    int usePath = 0;
    
    if(fd < 3){
        
        fd = open(filepath, O_RDONLY);
        usePath = 1;
        
        if(fd < 3){
            
            printf("Client: ERROR: file to add not found\n");
            return;
        }
    }
    
    //GENERATE MANIFEST FILEPATH
    char* manifestFilepath = malloc(strlen(projectname) + 10 + 5);
    memset(manifestFilepath, '\0', strlen(projectname) + 10 + 5);
    
    strcat(manifestFilepath, projectname);
    strcat(manifestFilepath, "/.Manifest");
    
    //CHECK IF MANIFEST EXISTS
    int manFD = open(manifestFilepath, O_RDWR);
    
    if(manFD < 3){
        
        printf("Client: ERROR: .Manifest not found\n");
        return;
    }
    
    //COUNT BYTES IN MANIFEST
    int tempManFD = open(manifestFilepath, O_RDONLY);
    int numManBytes = 0;
    
    char buffer[1000000];
    memset(buffer, '\0', 1000000);
    
    while(read(tempManFD, buffer, 1) > 0){    numManBytes++;}
    
    close(tempManFD);
    
    //READ MANIFEST INTO A STRING
    char* manifestStr = malloc(numManBytes + 5);
    memset(manifestStr, '\0', numManBytes + 5);
    
    memset(buffer, '\0', 1000000);
    
    while(read(manFD, buffer, 1) > 0){
        
        strcat(manifestStr, buffer);
    }
    
    //CHECK IF FILEPATH ALREADY EXISTS IN MANIFEST
    if(strstr(manifestStr, filepath) != NULL){
        
        printf("Client: ERROR: file to add already exists in .Manifest\n");
        return;
    }
    
    //COUNT BYTES IN FILE
    int tempFD;
    
    if(usePath){    tempFD = open(filepath, O_RDONLY);}
    else{           tempFD = open(filename, O_RDONLY);}
    
    int numBytes = 0;
    
    memset(buffer, '\0', 1000000);
    
    while(read(tempFD, buffer, 1) > 0){   numBytes++;}
    
    close(tempFD);
    
    //READ FILE INTO A STRING
    char* fileStr = malloc(numBytes + 5);
    memset(fileStr, '\0', numBytes + 5);
    
    memset(buffer, '\0', 1000000);
    
    while(read(fd, buffer, 1) > 0){
        
        strcat(fileStr, buffer);
    }
    
    //GENERATE HASH STRING FOR FILE
    char hash[65];
    setHash(fileStr, hash);
   
    //WRITE INTO MANIFEST FILE'S ADD ENTRY
    int newManFD = open(manifestFilepath, O_WRONLY | O_APPEND);
    
    write(newManFD, "0", 1);
    write(newManFD, "\t", 1);
    
    write(newManFD, filepath, strlen(filepath));
    write(newManFD, "\t", 1);
    
    write(newManFD, hash, 64);
    write(newManFD, "\n", 1);
    
    //CLOSE FILE DESCRIPTORS AND FREE MALLOCED VARIABLES
    close(fd);
    close(manFD);
    close(tempManFD);
    close(tempFD);
    close(newManFD);
    
    free(filepath);
    free(manifestFilepath);
    
    return;
}

/*REMOVE FUNCTION - VALIDATES USER REQUEST AND UPDATES MANIFEST ACCORDINGLY
 *
 *INPUT:
 *    -STRING WITH NAME OF PROJECT
 *    -STRING WITH PATH TO FILE (MAY OR MAY NOT CONTAIN PROJECT NAME)
 */
void _remove(char* projectname, char* filename){
    
    //CHECK IF PROJECT DIRECTORY EXISTS
    DIR* dir = opendir(projectname);
    
    if(dir){    closedir(dir);}
    
    else{
        
        printf("ERROR: project directory not found\n");
        return;
    }
    
    //GENERATE FILEPATH
    char* filepath = malloc(strlen(projectname) + 1 + strlen(filename) + 5);
    memset(filepath, '\0', strlen(projectname) + 1 + strlen(filename) + 5);
    
    strcat(filepath, projectname);
    strcat(filepath, "/");
    strcat(filepath, filename);
    
    //GENERATE MANIFEST FILEPATH
    char* manifestFilepath = malloc(strlen(projectname) + 10 + 5);
    memset(manifestFilepath, '\0', strlen(projectname) + 10 + 5);
    
    strcat(manifestFilepath, projectname);
    strcat(manifestFilepath, "/.Manifest");
    
    //CHECK IF MANIFEST EXISTS
    int manFD = open(manifestFilepath, O_RDWR);
    
    if(manFD < 3){
        
        printf("ERROR: .Manifest not found\n");
        return;
    }
    
    //COUNT BYTES IN MANIFEST
    int tempManFD = open(manifestFilepath, O_RDONLY);
    int numManBytes = 0;
    
    char buffer[1000000];
    memset(buffer, '\0', 1000000);
    
    while(read(tempManFD, buffer, 1) > 0){    numManBytes++;}
    
    close(tempManFD);
    
    //READ MANIFEST INTO A STRING
    char* manifestStr = malloc(numManBytes + 5);
    memset(manifestStr, '\0', numManBytes + 5);
    
    memset(buffer, '\0', 1000000);
    
    while(read(manFD, buffer, 1) > 0){
        
        strcat(manifestStr, buffer);
    }
    
    //CHECK IF EITHER FILENAME OR FILEPATH EXISTS IN MANIFEST
    int usePath = 0;
    char* pathInManifestStr = NULL;
    
    pathInManifestStr = strstr(manifestStr, filepath);
    
    if(pathInManifestStr == NULL){
        
        pathInManifestStr = strstr(manifestStr, filename);
        
        if(pathInManifestStr == NULL){
            
            printf("ERROR: file to remove doesn't exist in .Manifest\n");
            return;
        }
    }
    
    else{    usePath = 1;}
    
    //GENERATE NEW MANIFEST STRING WITH ENTRY REMOVED
    int pathnameBegex = pathInManifestStr - manifestStr;
    int pathLength = strlen(filename);
    
    if(usePath){    pathLength = strlen(filepath);}
    
    int lineBegex = pathnameBegex - 2;
    int lineEndex = pathnameBegex + pathLength + 1 + 64 + 1;
    int lineLength = lineEndex - lineBegex;
    
    char* newManifestStr = malloc(strlen(manifestStr - lineLength + 5));
    memset(newManifestStr, '\0', strlen(manifestStr - lineLength + 5));
    
    strncpy(newManifestStr, manifestStr, lineBegex);
    strcat(&(newManifestStr[lineBegex]), &(manifestStr[lineEndex + 1]));
    
    //DELETE OLD MANIFEST
    remove(manifestFilepath);
    
    //CREATE NEW MANIFEST
    int newManFD = open(manifestFilepath, O_WRONLY | O_CREAT, 0644);  
    write(newManFD, newManifestStr, strlen(newManifestStr));
    
    //CLOSE FILE DESCRIPTORS AND FREE MALLOCED VARIABLES
    close(manFD);
    close(newManFD);
    
    free(filepath);
    free(manifestFilepath);
    free(manifestStr);
    free(newManifestStr);
    
    return;
}

int main(int argc, char* argv[]){

	if(argc < 3 || argc > 5){
		printf("ERROR: too few/many commands/arguments\n");
		return 1;
	}

	char command = '\0';
    
    // make sure the first argument is the correct input and give it a specifc command to use in the switch statement later on
	if(strcmp(argv[1], "add") == 0){                    command = 'A';}         // done - JA

	else if(strcmp(argv[1], "checkout") == 0){          command = 'K';}

	else if(strcmp(argv[1], "create") == 0){            command = 'E';}         // done - CZ

	else if(strcmp(argv[1], "commit") == 0){            command = 'M';}         // working on this - CZ

	else if(strcmp(argv[1], "configure") == 0){         command = 'F'; }        // done - CZ

    else if(strcmp(argv[1], "currentversion") == 0){    command = 'V';}

	else if(strcmp(argv[1], "destroy") == 0){           command = 'D';}

	else if(strcmp(argv[1], "history") == 0){           command = 'H';}

	else if(strcmp(argv[1], "push") == 0){              command = 'P';}

	else if(strcmp(argv[1], "remove") == 0){            command = 'R';}         // working on this - JA

	else if(strcmp(argv[1], "rollback") == 0){          command = 'L';}

    else if(strcmp(argv[1], "update") == 0){            command = 'U';}

	else if(strcmp(argv[1], "upgrade") == 0){           command = 'G';}

	else{
		printf("ERROR: invalid command.\n");
		return 1;
	}

    // main variables used in each case to connect to the server
    int configured = -1;
    int socketNum = 0;
    struct sockaddr_in serverAddress;
    int PORT;
    int address;
    int connection;
    int n;
    
    // variables used by cases to get into .configure and get the address and port number
    int configured2 = -1;
    char* getAdd;
    char* getPortNum;
    char c;
    int count;
    int addressSize;
    int portNumSize;
    
    //variables used to send messages back and forth between client and server
    char buffer[100000];
    int numberOfChars;
    int good;
    char copy;
    int i;
    
    //variables used to read/write/append the Mainfest
    int ManifestFD;
    int numTabs;
    
    //variables used to work with the .Update
    int updateFD;
    int isEmpty;
    char temp[100000];
    
    // variables used to work with .Commit
    char another[100000];
    // used char temp[]
    int CliManFD;
    //used int i
    //used char c
    //used int n
    //used int good
    int CommitFileFD;
    int versionNum;
    char currentFilePath[100000];
    char currentHash[65];
    char newHash[65];
    int getFileCharsFD;
    int numCharsInFile;
    char* allFileChars;
    int j;
    char* p;
    int serverVersionNum;
    char mode;
    int tempCommitFD;
    //int tempCommitFD2;
    
    
    
	switch(command){
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------
		case 'A': //add <projectname> <filename>

			if(argc != 4){
				printf("Client: ERROR: must run as: ");
				printf("Client: './WTF add <projectname> <filename>'\n");
				break;
			}

            configured = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
			if(configured < 0 ){
				printf("Client: ERROR: must first run: ");
				printf("Client: './WTF configure <IP/hostname> <port>'\n");
				break;
			}
            
            //SEE ADD FUNCTION
            add(argv[2], argv[3]);

			break;
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
		case 'D': //destroy <projectname>

			if(argc != 3){
				printf("Client: ERROR: must run as: ");
				printf("Client: './WTF destroy <projectname>'\n");
				break;
			}
            
            configured = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
			if(configured < 0){
				printf("Client: ERROR: must first run: ");
				printf("Client: './WTF configure <IP/hostname> <port>'\n");
				break;
			}

			break;
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------
		case 'E': //create <projectname>

			if(argc != 3){
				printf("Client: ERROR: must run as: ");
				printf("Client: './WTF create <projectname>'\n");
				break;
			}
            
            //make sure that the configure command was called so that the client can use the IP address and port number
            configured = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
			if(configured < 0){
				printf("Client: ERROR: must first run: ");
				printf("Client: './WTF configure <IP/hostname> <port>'\n");
				break;
			}
            
            // make an array to store the IP address
            read(configured,&c,1);
            count = 0;
            while(c != '\t'){
                ++count;
                read(configured,&c,1);
            }
            getAdd = (char*)malloc((count+1)*sizeof(char));
            addressSize = count;
            
            // skip the tab
            read(configured,&c,1);
            
            // make an array to store the Port number
            count = 0;
            while(c != '\n') {
                ++count;
                read(configured,&c,1);
            }
            getPortNum = (char*)malloc((count+1)*sizeof(char));
            portNumSize = count;
            
            close(configured);
            
            // now that the arrays are made, store the info inside them
            configured2 = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
            
            // store the address
            count = 0;
            while(count < addressSize) {
                read(configured2,&c,1);
                getAdd[count] = c;
                count++;
            }
            getAdd[addressSize] = '\0';
            
            // skip this tab
            read(configured2,&c,1);
            
            // store the port number
            count = 0;
            while(count < portNumSize) {
                read(configured2,&c,1);
                getPortNum[count] = c;
                count++;
            }
            getPortNum[portNumSize] = '\0';
            
            // now connect to the server
            socketNum = socket(AF_INET, SOCK_STREAM, 0);
            if (socketNum < 0) {
                printf("Client: Trouble creating socket.\n");
                return 1;
            } else {
                printf("Client: Socket created.\n");
            }
            
            //initialize the server address to 0
            memset(&serverAddress, '0', sizeof(serverAddress));
            serverAddress.sin_family = AF_INET;                 // normal communication
            PORT = atoi(getPortNum);                            // change the third argument from chars to an int
            serverAddress.sin_port = htons(PORT);               // put in the port number (last argument)
            
            //check the address
            address = inet_pton(AF_INET, getAdd , &serverAddress.sin_addr);  // Convert IPv4 and IPv6 addresses from text to binary form
            if( address <= 0) {
                printf("Client: Invalid address given\n");
                return 1;
            } else {
                printf("Client: Address valid.");
                //printf(" Trying to connect to server.")
                printf("\n");
            }
            
            //connect to the server
            connection = connect(socketNum , (struct sockaddr *)&serverAddress , sizeof(serverAddress));
            if (connection < 0) {
                printf("Client: Connection Failed\n");
                return 1;
            } else {
                printf("Client: Connection to server successful\n");
            }
            
            //tell the server which command this client wants to run
            n = write(socketNum, "E", 1);
            if (n < 0) {
                printf("Client: ERROR writing to socket\n");
                break;
            }
            printf("Client: Command sent. Ready to run.\n");
            
            // *** NOW DO THE COMMAND SPECIFIC INSTRUCTIONS *** //
            
            //make room in the buffer to send the name of the project
            bzero(buffer,100000);                               // zero out the bytes of memeory in the char array
            numberOfChars = (int)strlen(argv[2]);               // number of chars that the project name takes up
            snprintf(buffer,100000, "%d:%s", numberOfChars ,argv[2]);
            
            //send the server project name info (the number of chars and the name of the project => <numChar>:<project>
            good = write(socketNum,buffer,100000);
            if (good < 0) {
                printf("Client: Error writing to the server.\n");
                break;
            }
            printf("Client: Name of the project sent to the Server.\n");
            
            //wait for the server to send back a message about the project
            bzero(buffer,100000);                  // clear out the buffer so new messages can come in
            recv(socketNum,buffer,99999,0);
            printf("\tServer: %s\n",buffer);     // message from the server | can be either the project exists or it doesn't (will know based on the length of buffer)
            
            if ((int)strlen(buffer) == 76) {     // can expect the server to send a .Manifest for the project
                bzero(buffer,100000);
                recv(socketNum,buffer,99999,0);
                printf("Client: .Manifest contents for project '%s' received.\n",argv[2]);
                
                // now make the project directory and copy the contents of the .Manifest into a local copy
                mkdir(argv[2],0777);
                char tempBuffer[100000];
                snprintf(tempBuffer,100000, "./%s/.Manifest",argv[2]);
                
                ManifestFD = open(tempBuffer, O_RDWR | O_APPEND | O_CREAT, 0644);
                i = 0;
                copy = buffer[i];
                while (copy != '\0') {
                    write(ManifestFD,&copy,1);
                    ++i;
                    copy = buffer[i];
                }
                close(ManifestFD);
                printf("Client: Project directory '%s' and .Manifest made.\n",argv[2]);
                
            } else {                            // the project already exists , failure
                break;
            }
			break;	
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        case 'F': //configure <IP/hostname> <port>

			if(argc != 4){
				printf("Client: ERROR: must run as: ");
				printf("Client: './WTF configure <IP/hostname> <port>'\n");
				break;
			}
            
            // create the socket
            socketNum = socket(AF_INET, SOCK_STREAM, 0);
            if (socketNum < 0) {
                printf("Client: Trouble creating socket.\n");
                return 1;
            } else {
                printf("Client: Socket created.\n");
            }
            
            // check the port number to make sure that it is within the range of [8000-65535] (inclusive)
            // change the third argument from chars to an int
            PORT = atoi(argv[3]);
            if ((PORT >= 8000) && (PORT <= 65535)) {
                printf("Port number is VALID.\n");
            } else {
                printf("Port number is INVALID. Please use numbers between 8000 and 65535 inclusive for safety purposes\n");
                return 1;
            }
            
            //initialize the server address to 0
            memset(&serverAddress, '0', sizeof(serverAddress));
            serverAddress.sin_family = AF_INET;                 // normal communication
            serverAddress.sin_port = htons(PORT);               // put in the port number (last argument)
            
            // check the address
            address = inet_pton(AF_INET, argv[2] , &serverAddress.sin_addr);  // Convert IPv4 and IPv6 addresses from text to binary form
            if( address <= 0) {
                printf("Client: Invalid address given\n");
                return 1;
            } else {
                printf("Client: Address valid.");
                //printf(" Trying to connect to server.");
                printf("\n");
            }
            
            // *** Don't need to connect to the server, just need to make sure that the IP address and the Port Num are good and then make the .configure file *** //
            
            // if there is an old .configure file that this client used before, delete it
            int OldConfigured = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
            if ( OldConfigured >=  0) { remove("./.configure"); }
            
            // make a new .configure file with the new IP address and new port number
            configured = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
            if (configured < 0) {
                printf("Client: Problem creating the ./.configure file\n");
            } else {
                printf("Client: ./.configure file created\n");
            }
            
            // now put the IP and port number in the ./. configure file so that other methods can use it later on
            // the info is written in the format => <IP address>___\t___<PORT#>___\n___
            
            //write in the IP address
            char* ptr = argv[2];
            int count = 0;
            char c = ptr[count];
            while(c != '\0') {
                write(configured,&c,1);
                count++;
                c = ptr[count];
            }
            
            //write in the tab
            c = '\t';
            write(configured,&c,1);
            
            //write in the Port Number
            ptr = argv[3];
            count = 0;
            c = ptr[count];
            while(c != '\0') {
                write(configured,&c,1);
                count++;
                c = ptr[count];
            }
            
            // wrtite in a new line at the end
            c = '\n';
            write(configured,&c,1);
            close(configured);
            
            printf("Client: ./.configure file has the following information: IP address and PORT\n");
            
			break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		case 'G': //upgrade <projectname>

			if(argc != 3){
				printf("Client: ERROR: must run as: ");
				printf("Client: './WTF upgrade <projectname>'\n");
				break;
			}

            configured = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
			if(configured < 0){
				printf("Client: ERROR: must first run: ");
				printf("Client: './WTF configure <IP/hostname> <port>'\n");
				break;
			}

			break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		case 'H': //history <projectname>

			if(argc != 3){
				printf("Client: ERROR: must run as: ");
				printf("Client: './WTF history <projectname>'\n");
				break;
			}

            configured = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
			if(configured < 0){
				printf("Client: ERROR: must first run: ");
				printf("Client: './WTF configure <IP/hostname> <port>'\n");
				break;
			}

			break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		case 'K': //checkout <projectname>

			if(argc != 3){
				printf("Client: ERROR: must run as: ");
				printf("Client: './WTF checkout <projectname>'\n");
				break;
			}

            configured = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
			if(configured < 0){
				printf("Client: ERROR: must first run: ");
				printf("Client: './WTF configure <IP/hostname> <port>'\n");
				break;
			}

			break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		case 'L': //rollback <projectname> <version>

			if(argc != 4){
				printf("Client: ERROR: must run as: ");
				printf("Client: './WTF rollback <projectname> <version>'\n");
				break;
			}

            configured = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
			if(configured < 0){
				printf("Client: ERROR: must first run: ");
				printf("Client: './WTF configure <IP/hostname> <port>'\n");
				break;
			}

			break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		case 'M': //commit <projectname>

			if(argc != 3){
				printf("Client: ERROR: must run as: ");
				printf("Client: './WTF commit <projectname>'\n");
				break;
			}

            //make sure that the configure command was called so that the client can use the IP address and port number
            configured = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
            if(configured < 0){
                printf("Client: ERROR: must first run: ");
                printf("Client: './WTF configure <IP/hostname> <port>'\n");
                break;
            }
            
            // check the .Update file of the project to make sure it either doen't have one or it is empty
            bzero(temp,100000);
            strcat(temp,argv[2]);
            strcat(temp,"/.Update"); // make the path file to the .Update file to go check the contents
            
            // look for the .Update => its okay to go if its not there or it is empty
            updateFD = open(temp, O_RDWR, 0644);
            if (updateFD >= 0) {
                isEmpty = read(updateFD,&c,1);
                if (isEmpty == 1) {
                    printf("Client: The .Update file is not empty. The user must first UPDATE this project.\n");
                    break;
                }
            }
            printf("Client: Finished checking the .Update file in the project. Good to go. Proceeding to connect with the server.\n");
            
            // make an array to store the IP address
            read(configured,&c,1);
            count = 0;
            while(c != '\t'){
                ++count;
                read(configured,&c,1);
            }
            getAdd = (char*)malloc((count+1)*sizeof(char));
            addressSize = count;
            
            // skip the tab
            read(configured,&c,1);
            
            // make an array to store the Port number
            count = 0;
            while(c != '\n') {
                ++count;
                read(configured,&c,1);
            }
            getPortNum = (char*)malloc((count+1)*sizeof(char));
            portNumSize = count;
            
            close(configured);
            
            // now that the arrays are made, store the info inside them
            configured2 = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
            
            // store the address
            count = 0;
            while(count < addressSize) {
                read(configured2,&c,1);
                getAdd[count] = c;
                count++;
            }
            getAdd[addressSize] = '\0';
            
            // skip this tab
            read(configured2,&c,1);
            
            // store the port number
            count = 0;
            while(count < portNumSize) {
                read(configured2,&c,1);
                getPortNum[count] = c;
                count++;
            }
            getPortNum[portNumSize] = '\0';
            
            // now connect to the server
            socketNum = socket(AF_INET, SOCK_STREAM, 0);
            if (socketNum < 0) {
                printf("Client: Trouble creating socket.\n");
                return 1;
            } else {
                printf("Client: Socket created.\n");
            }
            
            //initialize the server address to 0
            memset(&serverAddress, '0', sizeof(serverAddress));
            serverAddress.sin_family = AF_INET;                 // normal communication
            PORT = atoi(getPortNum);                            // change the third argument from chars to an int
            serverAddress.sin_port = htons(PORT);               // put in the port number (last argument)
            
            //check the address
            address = inet_pton(AF_INET, getAdd , &serverAddress.sin_addr);  // Convert IPv4 and IPv6 addresses from text to binary form
            if( address <= 0) {
                printf("Client: Invalid address given\n");
                return 1;
            } else {
                printf("Client: Address valid. Trying to connect to server.\n");
            }
            
            //connect to the server
            connection = connect(socketNum , (struct sockaddr *)&serverAddress , sizeof(serverAddress));
            if (connection < 0) {
                printf("Client: Connection Failed\n");
                return 1;
            } else {
                printf("Client: Connection to server successful\n");
            }
            
            //tell the server which command this client wants to run
            n = write(socketNum, "M", 1);
            if (n < 0) {
                printf("Client: ERROR writing to socket\n");
                break;
            }
            
            // *** NOW DO THE COMMAND SPECIFIC INSTRUTIONS *** //

            // ask the server for the .Manifest of the specific project
            bzero(buffer,100000);                            // zero out the bytes of memeory in the char array
            numberOfChars = (int)strlen(argv[2]);            // number of chars that the project name takes up
            
            snprintf(buffer,100000, "%d:%s", numberOfChars ,argv[2]);
            
            //send the server project name info (the number of chars and the name of the project => <numChar>:<project>
            good = write(socketNum,buffer,100000);
            if (good < 0) {
                printf("Client: Error writing to the server.\n");
                break;
            }
            
            bzero(buffer,100000);               // clear out the buffer so new messages can come in
            recv(socketNum,buffer,99999,0);
            printf("\tServer: %s\n",buffer);    // message from the server | can be either the project exists or it doesn't (will know based on the length of buffer)
            
            if (buffer[8] == 'n') {             // error has occurred becuase the project does not exist in the server
                printf("Client: Error noted.\n");
                break;
            } else {                            // the project already exists , the .Manifest will be sent
                bzero(buffer,100000);
                recv(socketNum,buffer,99999,0);
                printf("Client: .Manifest contents for project '%s' received.\n",argv[2]);
               
                //need to make a file path to the .Man for the client => 'ProjectName'/.Manifest
                bzero(temp,100000);
                snprintf(temp,100000,"%s/.Manifest",argv[2]);
                
                // open the .Manifest
                CliManFD = open(temp,O_RDWR, 0644);
                if(CliManFD < 0){
                    printf("Client: Error when trying to open .Manifest of project '%s'\n",argv[2]);
                    break;
                }
                printf("Client: Opened the .Manifest.\n");
                
                //read the chars into the temp buffer
                bzero(temp,100000);
                i = 0;
                n = read(CliManFD,&c,1);;
                while ( (i <= 99999) && (n == 1)) {
                    temp[i] = c;
                    ++i;
                    n = read(CliManFD,&c,1);
                }
                temp[i] = '\0';
                close(CliManFD);
                
                //now compare the version numbers of both Manifests (should be the first couple chars found in both arrays (buffer and temp)
                i = 0;
                while ((buffer[i] == temp[i]) && (buffer[i] != '\n') && (temp[i] != '\n')) {
                    ++i;
                }
                if ((buffer[i] != '\n') && (temp[i] != '\n')) {
                    printf("Client: The versions of the Manifest of the client and server do not match. User must update the local project.\n");
                    break;
                }
                printf("Client: The versions of the Manifest match. Starting to recompute hashes.\n");
                
                // create the .Commit file
                bzero(another,100000);
                snprintf(another,100000,"%s/.Commit",argv[2]);
                
                //if there is already a .Commit , delete it so you can start over
                tempCommitFD = open(another,O_RDWR, 0644);
                if (!(tempCommitFD < 0)){
                    remove(another);
                    close(tempCommitFD);
                }
                
                
                CommitFileFD = open(another,O_RDWR | O_CREAT | O_APPEND, 0644);
                if (CommitFileFD < 0) {
                    printf("Client: Error occurred when creating the .Commit file.\n");
                    break;
                }
                
                //now recompute the hash for all the projects found in the client's Manifest
                if ( (temp[i+1] != ' ') && (temp[i+1] != '\n') && (temp[i+1] != '\t') ) {
                    good = 1;
                } else {
                    good = 0;
                }
                ++i;        // do this so that the next loop starts on the first version number
                
                // will use this loop to go line by line and break up the version number, the file path and the hash
                while (temp[i] != '\0') {
                    //this is the version number
                    //get to the first tab, keep storing the chars into the char array 'another'
                    bzero(another,100000);
                    n = 0;
                    while(temp[i] != '\t'){
                        another[n] = temp[i];
                        ++i;
                        ++n;
                    }
                    another[n] = '\0';
                    // at this point temp[i] == '\t'
                    
                    //convert array 'another' into an int
                    versionNum = atoi(another);
                    
                    // do this so that the next loop starts at the beginning of the file path
                    ++i;
                    
                    //this is the file path
                    bzero(currentFilePath,100000);
                    n = 0;
                    while(temp[i] != '\t'){
                        currentFilePath[n] = temp[i];
                        ++i;
                        ++n;
                    }
                    currentFilePath[n] = '\0';
                    // at this point temp[i] == '\t'
                    
                    // find the same file inside the .Manifest of the server
                    p = strstr(buffer,currentFilePath);
                    if (p != NULL) {
                        
                        // now that you found the file name, figure out its verion number
                        while (*p != '\n') {
                            --p;
                        }
                        // at this point, *p == '\n'
                        
                        ++p;    // now *p is on the first digit of the version number of the file
                        
                        // put the version number into the serverVersionNum
                        bzero(another,100000);
                        n = 0;
                        while(*p != '\t') {
                            another[n] = *p;
                            ++p;
                            ++n;
                        }
                        another[n] = '\0';
                        
                        serverVersionNum = atoi(another);
                        
                        // check to make sure that the server version numb <= the client file's version numb
                        if (serverVersionNum <= versionNum) {
                            // **** now figure out if you need to actaully put it into the .Commit or not *****
                            
                            // do this so that the next loop starts at the beginning of the old hash
                            ++i;
                            
                            //this is the old hash
                            bzero(currentHash,65);
                            n = 0;
                            while(temp[i] != '\n'){
                                currentHash[n] = temp[i];
                                ++i;
                                ++n;
                            }
                            currentHash[n] = '\0';
                            // at this point temp[i] == '\n'
                            
                            //recompute hash and compare with the old hash
                            // use this for the first run
                            getFileCharsFD = open(currentFilePath,O_RDWR, 0644);
                            if (getFileCharsFD < 0) {
                                printf("Client: There was an error when opening the file path to construct a new hash.\n");
                                break;
                            }
                            printf("Client: File opened. Building new hash.\n");
                            
                            // count the number of chars in the file
                            j = 1;
                            numCharsInFile = 0;
                            n = 0;
                            while (j == 1) {
                                j = read(getFileCharsFD,&c,1);
                                another[n] = c;
                                ++numCharsInFile;
                                ++n;
                            }
                            another[n] = '\0';
                            
                            // do this so that the next loop starts at the next file or at a \0
                            ++i;
                            
                            
                            //allocate an array with enough room to accomodate all the chars in the file
                            allFileChars = (char*)malloc((numCharsInFile + 5)*sizeof(char));
                            memset(allFileChars,'\0',(numCharsInFile + 5));
                            
                            //copy all the chars from array 'another' into the allocated array
                            n = 0;
                            while (another[n] != '\0') {
                                allFileChars[n] = another[n];
                                n++;
                            }
                            allFileChars[n] = '\0';
                            
                            // make the new hash and store it in newHash[]
                            setHash(allFileChars, newHash);
                            
                            //compare the newHash and the currentHash and see if they match
                            n = strcmp(newHash,currentHash);
                            
                            if (n == 0) {
                                printf("Client: The hash on the .Manifest and the new hash are the same for file: %s\n",currentFilePath);
                            } else {
                                printf("Client: The hash on the .Manifest and the new hash are NOT the same for file: %s\n",currentFilePath);
                                printf("Client: Storing the new version number and new hash for the %s in the .Commit\n",currentFilePath);
                                
                                //now store the new version number, the file Path and the new Hash into the .Commit
                                ++versionNum;
                                
                                //use the 'another' array to store the entire string //
                                mode = 'M'; // modify
                                memset(another,'\0',100000);
                                snprintf(another,100000,"%c\t%d\t%s\t%s\n",mode,versionNum,currentFilePath,newHash);
                                
                                n = 0;
                                while (another[n] != '\0') {
                                    c = another[n];
                                    write(CommitFileFD,&c,1);
                                    n++;
                                }
                                
                                free(allFileChars);
                                printf("Client: A new entry has been added to the .Commit file.\n");
                            }
                            
                            // serverVersionNum > versionNum
                        } else {
                            bzero(another,100000);
                            snprintf(another,100000,"%s/.Commit",argv[2]);
                            
                            int tempCommitFD2 = open(another,O_RDWR, 0644);
                            remove(another);
                            close(tempCommitFD2);
                            
                            printf("Server: It appears that the server has a higher version of some of the files. The user must update before trying to commit.\n");
                            return 1;
                        }
                        
                        // this file does not exist inside the server's .Manifest
                    } else {
                        
                        // do this so that the next loop starts at the beginning of the old hash
                        ++i;
                        
                        //this is the old hash
                        bzero(currentHash,65);
                        n = 0;
                        while(temp[i] != '\n'){
                            currentHash[n] = temp[i];
                            ++i;
                            ++n;
                        }
                        currentHash[n] = '\0';
                        // at this point temp[i] == '\n'
                        
                        //recompute hash and compare with the old hash
                        // use this for the first run
                        getFileCharsFD = open(currentFilePath,O_RDWR, 0644);
                        if (getFileCharsFD < 0) {
                            printf("Client: There was an error when opening the file path to construct a new hash.\n");
                            break;
                        }
                        printf("Client: File opened. Building new hash.\n");
                        
                        // count the number of chars in the file
                        j = 1;
                        numCharsInFile = 0;
                        n = 0;
                        while (j == 1) {
                            j = read(getFileCharsFD,&c,1);
                            another[n] = c;
                            ++numCharsInFile;
                            ++n;
                        }
                        another[n] = '\0';
                        
                        // do this so that the next loop starts at the next file or at a \0
                        ++i;
                        
                        
                        //allocate an array with enough room to accomodate all the chars in the file
                        allFileChars = (char*)malloc((numCharsInFile + 5)*sizeof(char));
                        memset(allFileChars,'\0',(numCharsInFile + 5));
                        
                        //copy all the chars from array 'another' into the allocated array
                        n = 0;
                        while (another[n] != '\0') {
                            allFileChars[n] = another[n];
                            n++;
                        }
                        allFileChars[n] = '\0';
                        
                        // make the new hash and store it in newHash[]
                        setHash(allFileChars, newHash);
                        
                        //compare the newHash and the currentHash and see if they match
                        n = strcmp(newHash,currentHash);
                        
                        if (n == 0) {
                            printf("Client: The hash on the .Manifest and the new hash are the same for file: %s\n",currentFilePath);
                            //use the 'another' array to store the entire string
                            mode = 'A'; // add
                            memset(another,'\0',100000);
                            snprintf(another,100000,"%c\t%d\t%s\t%s\n",mode,versionNum,currentFilePath,newHash);
                            
                            n = 0;
                            while (another[n] != '\0') {
                                c = another[n];
                                write(CommitFileFD,&c,1);
                                n++;
                            }
                            
                            free(allFileChars);
                        } else {
                            printf("Client: The hash on the .Manifest and the new hash are NOT the same for file: %s\n",currentFilePath);
                            printf("Client: Storing the new version number and new hash for the %s in the .Commit\n",currentFilePath);
                            
                            //now store the new version number, the file Path and the new Hash into the .Commit
                            ++versionNum;
                            
                            //use the 'another' array to store the entire string
                            mode = 'A'; // add
                            memset(another,'\0',100000);
                            snprintf(another,100000,"%c\t%d\t%s\t%s\n",mode,versionNum,currentFilePath,newHash);
                            
                            n = 0;
                            while (another[n] != '\0') {
                                c = another[n];
                                write(CommitFileFD,&c,1);
                                n++;
                            }
                            
                            free(allFileChars);
                            printf("Client: A new entry has been added to the .Commit file.\n");
                        }
                    }
                    
                } // ends the main while loop
             
                // I COULDNT FIGURE THIS PART OUT ... KEPT GOING INTO SEG FAULT WHEN I TRIED .... I GAVE UP
                    // go through the server .Manifest and find files that do not exist on the client's .Manifest
                    // these files need to be removed (R) ( put it into the .Commit)
                    // SOME OF THE CODE I TRIED FOR THIS IS DOWN IN THE BOTTOM...
                
                
                close(getFileCharsFD);
                close(CommitFileFD);
                break;
            }
            
        
			break;
//----------------------------------------------------------------------------------------------------------------------------------------------
		case 'P': //push <projectname>

			if(argc != 3){
				printf("Client: ERROR: must run as: ");
				printf("Client: './WTF push <projectname>'\n");
				break;
			}

            configured = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
			if(configured < 0){
				printf("Client: ERROR: must first run: ");
				printf("Client: './WTF configure <IP/hostname> <port>'\n");
				break;
			}

			break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		case 'R': //remove <projectname> <filename>

			if(argc != 4){
				printf("Client: ERROR: must run as: ");
				printf("Client: './WTF remove <projectname> <filename>'\n");
				break;
			}

            configured = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
			if(configured < 0){
				printf("Client: ERROR: must first run: ");
				printf("Client: './WTF configure <IP/hostname> <port>'\n");
				break;
			}
            
            //SEE REMOVE FUNCTION
            _remove(argv[2], argv[3]);

			break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		case 'U': //update <projectname>

			if(argc != 3){
				printf("Client: ERROR: must run as: ");
				printf("Client: './WTF update <projectname>'\n");
				break;
			}

            configured = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
			if(configured < 0){
				printf("Client: ERROR: must first run: ");
				printf("Client: './WTF configure <IP/hostname> <port>'\n");
				break;
			}

			break;
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		case 'V': //currentversion <projectname>

            if(argc != 3){
                printf("Client: ERROR: must run as: ");
                printf("Client: './WTF currentversion <projectname>'\n");
                break;
            }
            
            //make sure that the configure command was called so that the client can use the IP address and port number
            configured = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
            if(configured < 0){
                printf("Client: ERROR: must first run: ");
                printf("Client: './WTF configure <IP/hostname> <port>'\n");
                break;
            }
            
            // make an array to store the IP address
            read(configured,&c,1);
            count = 0;
            while(c != '\t'){
                ++count;
                read(configured,&c,1);
            }
            getAdd = (char*)malloc((count+1)*sizeof(char));
            addressSize = count;
            
            // skip the tab
            read(configured,&c,1);
            
            // make an array to store the Port number
            count = 0;
            while(c != '\n') {
                ++count;
                read(configured,&c,1);
            }
            getPortNum = (char*)malloc((count+1)*sizeof(char));
            portNumSize = count;
            
            close(configured);
            
            // now that the arrays are made, store the info inside them
            configured2 = open("./.configure",O_RDWR | O_CREAT | O_APPEND, 0644);
            
            // store the address
            count = 0;
            while(count < addressSize) {
                read(configured2,&c,1);
                getAdd[count] = c;
                count++;
            }
            getAdd[addressSize] = '\0';
            
            // skip this tab
            read(configured2,&c,1);
            
            // store the port number
            count = 0;
            while(count < portNumSize) {
                read(configured2,&c,1);
                getPortNum[count] = c;
                count++;
            }
            getPortNum[portNumSize] = '\0';
            
            // now connect to the server
            socketNum = socket(AF_INET, SOCK_STREAM, 0);
            if (socketNum < 0) {
                printf("Client: Trouble creating socket.\n");
                return 1;
            } else {
                printf("Client: Socket created.\n");
            }
            
            //initialize the server address to 0
            memset(&serverAddress, '0', sizeof(serverAddress));
            serverAddress.sin_family = AF_INET;                 // normal communication
            PORT = atoi(getPortNum);                            // change the third argument from chars to an int
            serverAddress.sin_port = htons(PORT);               // put in the port number (last argument)
            
            //check the address
            address = inet_pton(AF_INET, getAdd , &serverAddress.sin_addr);  // Convert IPv4 and IPv6 addresses from text to binary form
            if( address <= 0) {
                printf("Client: Invalid address given\n");
                return 1;
            } else {
                printf("Client: Address valid. Trying to connect to server.\n");
            }
            
            //connect to the server
            connection = connect(socketNum , (struct sockaddr *)&serverAddress , sizeof(serverAddress));
            if (connection < 0) {
                printf("Client: Connection Failed\n");
                return 1;
            } else {
                printf("Client: Connection to server successful\n");
            }
            
            n = write(socketNum, "V", 1);           // tell the server which command the client is going to run (this case, the create - E)
            if (n < 0) {
                printf("Client: ERROR writing to socket\n");
                exit(1);
            }
            
            // *** NOW DO THE COMMAND SPECIFIC INSTRUCTIONS *** //
            
            //make room in the buffer to send the name of the project
            bzero(buffer,100000);                               // zero out the bytes of memeory in the char array
            numberOfChars = (int)strlen(argv[2]);               // number of chars that the project name takes up
            snprintf(buffer,100000, "%d:%s", numberOfChars ,argv[2]);
            
            //send the server project name info (the number of chars and the name of the project => <numChar>:<project>
            good = write(socketNum,buffer,100000);
            if (good < 0) {
                printf("Client: Error writing to the server.\n");
                break;
            }
            
            //wait for the server to send back a message about the project
            bzero(buffer,100000);                  // clear out the buffer so new messages can come in
            recv(socketNum,buffer,99999,0);
            printf("\tServer: %s\n",buffer);     // message from the server | can be either the project exists or it doesn't (will know based on
                                                 // the length of buffer)
            
            
            if (buffer[8] == 'n') {             // error has occurred becuase the project does not exist in the server
                printf("Client: Error noted.\n");
            } else {                            // the project already exists , the .Manifest will be sent
                bzero(buffer,100000);
                recv(socketNum,buffer,99999,0);
                printf("Client: .Manifest contents for project '%s' received.\n",argv[2]);
                
                // Check if there are any files in the the server's version of the .Manifest
                if ( buffer[0] == ' ' || buffer[0] == '\n' || buffer[0] == '\t') {
                    printf("Client: *** Project: '%s' has no files in the Server's .Manifest ***\n",argv[2]);
                    break;
                }
                
                //Now go through the .Manifest and read out the files and its version number => <versionNum> \t <file name>
                printf("Clinet: Here are the file names in the project: '%s'\n",argv[2]);
            
                i = 0;
                numTabs = 0;
                while (i <= (int)strlen(buffer)) {
                    // need to count the number of tabs that appear
                    if (buffer[i] == '\t') {
                        ++numTabs;
                        if (numTabs <= 1) {
                            printf("%c",buffer[i]);
                            ++i;
                        } else {
                            // this is the second tab that the program comes accross, skip the hash and go to the next file
                            printf("\n");
                            while (buffer[i] != '\n') {
                                ++i;
                            }
                            
                            // restart the pattern
                            numTabs = 0;
                            ++i;
                        }
                    } else {
                        printf("%c",buffer[i]);
                        ++i;
                    }
                } // ends the main while loop
                
                break;
            }
            
			break;
	} // end of the switch statements

	return 1;
} // end of the main() method


/*
 
 COMMENTED OUT CODE FROM THE CONFIGURE COMMAND
 
 connection = connect(socketNum , (struct sockaddr *)&serverAddress , sizeof(serverAddress));
 if (connection < 0) {
 printf("Client: Connection Failed\n");
 return 1;
 } else {
 printf("Client: Connection to server successful\n");
 
 }
 
 n = write(socketNum, "F", 1);           // tell the server which command the client is going to run (this case, the create - E)
 if (n < 0) {
 printf("Client: ERROR writing to socket\n");
 exit(1);
 }
 
 
 //NOW DO THE COMMAND SPECIFIC INSTRUCTIONS
 
 // if you reach this point with no problems, then the connection is secured and the IP address and the PORT number work for this client
 close(socketNum);
 printf("Client: Disconnected from the server.\n");
 
 */


/*

// go through the server .Manifest and find files that do not exist on the client's .Manifest
// these files need to be removed (R) ( put it into the .Commit)

i = 0;
while (buffer[i] != '\n') {
    ++i;
}

++i;        // do this so that the next loop starts on the first version number


while (buffer[i] != '\0') {
    //this is the version number
    //get to the first tab, keep storing the chars into the char array 'another'
    bzero(another,100000);
    n = 0;
    while(buffer[i] != '\t'){
        another[n] = buffer[i];
        ++i;
        ++n;
    }
    another[n] = '\0';
    // at this point temp[i] == '\t'
    
    //convert array 'another' into an int
    versionNum = atoi(another);
    printf("%d\t",versionNum);
    // do this so that the next loop starts at the beginning of the file path
    ++i;
    
    //this is the file path
    bzero(currentFilePath,100000);
    n = 0;
    while(buffer[i] != '\t'){
        currentFilePath[n] = buffer[i];
        ++i;
        ++n;
    }
    currentFilePath[n] = '\0';
    // at this point temp[i] == '\t'
    printf("%s\t",currentFilePath);
    
    // do this so that the next loop starts at the beginning of the old hash
    ++i;
    
    //this is the old hash
    bzero(currentHash,65);
    n = 0;
    while(buffer[i] != '\n'){
        currentHash[n] = buffer[i];
        ++i;
        ++n;
    }
    currentHash[n] = '\0';
    // at this point temp[i] == '\n'
    printf("%s\n",currentHash);
    
    // do this so that the next loop starts at the next file or at a \0
    ++i;
}


 ------------------------------------------------------------------------------------
 while (buffer[i] != '\0') {
 //this is the version number
 //get to the first tab, keep storing the chars into the char array 'another'
 bzero(another,100000);
 n = 0;
 while(buffer[i] != '\t'){
 another[n] = buffer[i];
 ++i;
 ++n;
 }
 another[n] = '\0';
 // at this point temp[i] == '\t'
 
 //convert array 'another' into an int
 versionNum = atoi(another);
 
 // do this so that the next loop starts at the beginning of the file path
 ++i;
 
 //this is the file path
 bzero(currentFilePath,100000);
 n = 0;
 while(buffer[i] != '\t'){
 currentFilePath[n] = buffer[i];
 ++i;
 ++n;
 }
 currentFilePath[n] = '\0';
 // at this point temp[i] == '\t'
 
 // find the same file inside the .Manifest of the client
 p = strstr(temp,currentFilePath);
 
 //this file name does not exist in the client's .Manifest and needs to be removed
 if (p == NULL) {
 // do this so that the next loop starts at the beginning of the old hash
 ++i;
 
 //this is the old hash
 bzero(currentHash,65);
 n = 0;
 while(buffer[i] != '\n'){
 currentHash[n] = buffer[i];
 ++i;
 ++n;
 }
 currentHash[n] = '\0';
 // at this point temp[i] == '\n'
 
 // open the .Commit and put in new lines of info
 bzero(another,100000);
 snprintf(another,100000,"%s/.Commit",argv[2]);
 
 int tempCommitFD2 = open(another,O_RDWR | O_CREAT | O_APPEND, 0644);
 if (tempCommitFD2 < 0) {
 printf("Client: Error occurred when opening the .Commit file.\n");
 return 1;
 }
 
 mode = 'R'; // remove
 memset(another,'\0',100000);
 snprintf(another,100000,"%c\t%d\t%s\t%s\n",mode,versionNum,currentFilePath,currentHash);
 
 n = 0;
 while (another[n] != '\0') {
 c = another[n];
 write(tempCommitFD2,&c,1);
 n++;
 }
 close(tempCommitFD2);
 // do this so that the next loop starts at the next file or at a \0
 ++i;
 
 }
 // if (p != NULL) , nothing needs to be done since the first loop dealt with the case , so just move on to
 // the next file
 else {
 // do this so that the next loop starts at the beginning of the old hash
 ++i;
 
 //this is the old hash
 bzero(currentHash,65);
 n = 0;
 while(buffer[i] != '\n'){
 currentHash[n] = buffer[i];
 ++i;
 ++n;
 }
 currentHash[n] = '\0';
 // at this point temp[i] == '\n'
 
 // do this so that the next loop starts at the next file or at a \0
 ++i;
 }
 
 } // ends the main while loop (for the buffer[])
 
 */
