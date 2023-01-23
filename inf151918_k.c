#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#define MAX_MESSAGE_LENGTH 256
#define MAX_USERNAME_LENGTH 20
#define MAX_USERS 31

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

int hash(char *name){
	int length=strnlen(name,MAX_USERNAME_LENGTH);
	unsigned int hash_value=0;
	for(int i=0; i<length; i++){
		hash_value+=name[i];
		hash_value=hash_value*name[i];
		hash_value=hash_value%MAX_USERS;
	}
	return hash_value;
}

char *strip_name(char username[MAX_USERNAME_LENGTH]){
    username[strlen(username)-1]='\0';
    return username;
}

int main(){
	struct msgbuff request;
	struct msgbuff response;
	char input[MAX_MESSAGE_LENGTH];
	char username[MAX_USERNAME_LENGTH];
	int blocked_users[MAX_USERS];
	int joined_groups[3];
	for(int i=0; i<MAX_USERS; i++) blocked_users[i]=0;
	for(int i=0; i<3; i++) joined_groups[i]=0;

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
		printf("1) List logged in users\n2) List user groups\n3) Send message to user\n4) Send message to user group\n5) Check for messages\n6) Join group\n7) Leave group\n8) Block user\n9) Logout\n:");
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
				if(joined_groups[atoi(rec)]!=1){
					printf("You are not a member of this group!\n");
					break;
				}
				printf("Message content:\n");
				fgets(input,MAX_MESSAGE_LENGTH,stdin);
				set_message(&request,6,username,rec,input);
				if(msgsnd(server,&request,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
				break;
			case 5:
				msgrcv(server,&response,MAX_MESSAGE_LENGTH,id,IPC_NOWAIT);
				if(blocked_users[hash(response.sender)]==1) break;
				printf("Got message from %s\n%s\n",response.sender,response.mtext);
				break;
			case 9:
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
				joined_groups[atoi(input)]=1;
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
				joined_groups[atoi(input)]=0;
				printf("%s",response.mtext);
				break;
			case 8:
				printf("Block user: ");
				memset(input,0,strlen(input));
				fgets(input,MAX_USERNAME_LENGTH,stdin);
				blocked_users[hash(strip_name(input))]=1;
				break;
		}
	}
	return 0;
}
