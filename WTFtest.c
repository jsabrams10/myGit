#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){

	int status = 0;
	int pid1 = fork();

	if(pid1 == 0){

	int pid2 = fork();

	if(pid2 == 0){

	int pid3 = fork();

	if(pid3 == 0){

	int pid4 = fork();

	if(pid4 == 0){

	int pid5 = fork();

	if(pid5 == 0){

	int pid6 = fork();

	if(pid6 == 0){

	int pid7 = fork();

	if(pid7 == 0){

	int pid8 = fork();

	if(pid8 == 0){

	int pid9 = fork();

	if(pid9 == 0){

		execl("./server/WTFserver", "WTFserver", "8888", (char*)NULL);
		perror("Exec call failed (server).\n");
	}

	else{

		while((wait(&status)) > 0);
		execl("./client/WTF", "WTF", "configure", "127.0.0.1", "8888", (char*)NULL);
		perror("Exec call failed (configure).\n");
	}
	}

	else{

	while((wait(&status)) > 0);
	execl("./client/WTF", "WTF", "create", "testProj", (char*)NULL);
	perror("Exec call failed (create).\n");

	
	}
	}

	else{

	while((wait(&status)) > 0);
	execl("/usr/bin/touch", "touch", "testProj/testFile", (char*)NULL); 
	perror("Exec call failed (write testFile).\n");

	}
	}

	else{

	while((wait(&status)) > 0);
	execl("./client/WTF", "WTF", "add", "testProj", "testFile", (char*)NULL);
	perror("Exec call failed (add).\n");

	}
	}

	else{

	while((wait(&status)) > 0);
	execl("./client/WTF", "WTF", "remove", "testProj", "testFile", (char*)NULL);
	perror("Exec call failed (remove).\n");

	}
	}

	else{

	while((wait(&status)) > 0);
	execl("./client/WTF", "WTF", "currentversion", "testProj", (char*)NULL);
	perror("Exec call failed (currentversion).\n");

	}
	}

	else{

	while((wait(&status)) > 0);

	//perror("Exec call failed.\n");

	}
	}

	else{

	while((wait(&status)) > 0);

	//perror("Exec call failed.\n");

	}
	}

	else{

	while((wait(&status)) > 0);

	//perror("Exec call failed.\n");

	}

	return 0;
}



