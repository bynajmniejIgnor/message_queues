#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#define MAX_MESSAGE_LENGTH 256
#define MAX_USERNAME_LENGTH 20

struct msgbuff{
	long mtype;
	int code;
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
		printf("1) List logged in users\n2) List user groups\n3) Send message to user\n4) Send message to user group\n5) Check for messages\n6) Join group\n7) Leave group\n8) Logout\n: ");
		fgets(input,MAX_MESSAGE_LENGTH,stdin);
		switch(atoi(input)){
			case 1:
				set_message(&request,2,username,"server","");
				if(msgsnd(server,&request,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
				if(msgrcv(server,&response,MAX_MESSAGE_LENGTH,id,MSG_NOERROR)==-1) perror("msgrcv");
				printf("Logged in users are:\n%s",response.mtext);
				break;
			case 2:
				set_message(&request,5,username,"server","");
				if(msgsnd(server,&request,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
				if(msgrcv(server,&response,MAX_MESSAGE_LENGTH,id,MSG_NOERROR)==-1) perror("msgrcv");
				printf("%s",response.mtext);
				break;
			case 3:
				char rec[MAX_USERNAME_LENGTH]="";
				printf("Send message to: ");
				fgets(rec,MAX_USERNAME_LENGTH,stdin);
				printf("Message content:\n");
				fgets(input,MAX_MESSAGE_LENGTH,stdin);
				set_message(&request,4,username,rec,input);
				msgsnd(server,&request,MAX_MESSAGE_LENGTH,0);
				if(msgrcv(server,&response,MAX_MESSAGE_LENGTH,id,MSG_NOERROR)==-1) perror("msgrcv");
				printf("%s",response.mtext);
				break;
			case 4:
				memset(rec,0,strlen(rec));
				printf("Send message to users form group: ");
				fgets(rec,MAX_USERNAME_LENGTH,stdin);
				printf("Message content:\n");
				fgets(input,MAX_MESSAGE_LENGTH,stdin);
				set_message(&request,6,username,rec,input);
				if(msgsnd(server,&request,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
				break;
			case 5:
				msgrcv(server,&response,MAX_MESSAGE_LENGTH,id,IPC_NOWAIT); 
				printf("Got message from %s\n%s\n",response.sender,response.mtext);
				break;
			case 8:
				set_message(&request,3,username,"server","");
				if(msgsnd(server,&request,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
				if(msgrcv(server,&response,MAX_MESSAGE_LENGTH,id,MSG_NOERROR)==-1) perror("msgrcv");
				printf("%s",response.mtext);
				return 0;
			case 6:
				printf("Join to group nr: ");
				memset(input,0,strlen(input));
				fgets(input,MAX_USERNAME_LENGTH,stdin);
				set_message(&request,7,username,"server",input);
				if(msgsnd(server,&request,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
				if(msgrcv(server,&response,MAX_MESSAGE_LENGTH,id,MSG_NOERROR)==-1) perror("msgrcv");
				if(response.code==403){
					printf("Can't join this group :\"(\n");
					break;
				}
				printf("%s",response.mtext);
				break;
			case 7:
				printf("Leave group nr: ");
				memset(input,0,strlen(input));
				fgets(input,MAX_USERNAME_LENGTH,stdin);
				set_message(&request,8,username,"server",input);
				if(msgsnd(server,&request,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
				if(msgrcv(server,&response,MAX_MESSAGE_LENGTH,id,MSG_NOERROR)==-1) perror("msgrcv");
				if(response.code==403){
					printf("Something went wrong :\"(\n");
					break;
				}
				printf("%s",response.mtext);
				break;
		}
	}
	return 0;
}