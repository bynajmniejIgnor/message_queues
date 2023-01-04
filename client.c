#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#define MAX_MESSAGE_LENGTH 256
#define MAX_USERNAME_LENGTH 20
#define MAX_USERS 10

/*
1 - login request
2 - server OK response
3 - get userlist request
4 - server ERROR response
5 - logout request
MAX_USERS+ - user id 
*/

struct msgbuff{
	long mtype;
	char sender[MAX_USERNAME_LENGTH];
	char receiver[MAX_USERNAME_LENGTH];
	char mtext[MAX_MESSAGE_LENGTH];
};

void set_message(struct msgbuff *msg, long mtype, char *sender, char *receiver, char *body){
	msg->mtype=mtype;
	strcpy(msg->sender,sender);
	strcpy(msg->receiver,receiver);
	strcpy(msg->mtext,body);
}

int main(){
	struct msgbuff request;
	struct msgbuff response;
	char input[MAX_MESSAGE_LENGTH];
	char username[MAX_USERNAME_LENGTH];
	printf("Server id: ");
	fgets(input,MAX_MESSAGE_LENGTH,stdin);
	int server=msgget(atoi(input),0664);

	printf("Login: ");
	fgets(username,MAX_USERNAME_LENGTH,stdin);
	strcpy(request.sender,username);

	printf("Password: ");
	fgets(input,MAX_MESSAGE_LENGTH,stdin);
	strcpy(request.mtext,input);
	request.mtype=1;

	if(msgsnd(server,&request,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
	if(msgrcv(server,&response,MAX_MESSAGE_LENGTH,0,MSG_NOERROR)==-1) perror("msgrcv");
	printf("%s",response.mtext);
	if(response.mtype==403) return 0;
	int id=response.mtype;
	while(1){
		printf("My id is %i\n",id);
		printf("1) List logged in users\n2) Send message to user\n3) Check for messages\n4) Logout\n: ");
		fgets(input,MAX_MESSAGE_LENGTH,stdin);
		switch(atoi(input)){
			case 1:
				set_message(&request,2,username,"server","");
				if(msgsnd(server,&request,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
				if(msgrcv(server,&response,MAX_MESSAGE_LENGTH,id,MSG_NOERROR)==-1) perror("msgrcv");
				printf("Logged in users are:\n%s",response.mtext);
				break;
			case 2:
				char rec[MAX_USERNAME_LENGTH]="";
				printf("Send message to: ");
				fgets(rec,MAX_USERNAME_LENGTH,stdin);
				printf("Message content:\n");
				fgets(input,MAX_MESSAGE_LENGTH,stdin);
				set_message(&request,4,username,rec,input);
				msgsnd(server,&request,MAX_MESSAGE_LENGTH,0);
				printf("Message sent!\n");
				break;
			case 3:
				while(msgrcv(server,&response,MAX_MESSAGE_LENGTH,id,IPC_NOWAIT)){
					printf("Got message from %s\n%s\n",response.sender,response.mtext);
				}
				break;
			case 4:
				set_message(&request,3,username,"server","");
				if(msgsnd(server,&request,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
				if(msgrcv(server,&response,MAX_MESSAGE_LENGTH,id,MSG_NOERROR)==-1) perror("msgrcv");
				printf("%s",response.mtext);
				return 0;
		}
	}
	return 0;
}
