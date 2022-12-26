#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX_MESSAGE_LENGTH 256
#define MAX_USERS 10
#define MAX_USERNAME_LENGTH 20

typedef struct{
	long mtype;
	char sender[MAX_USERNAME_LENGTH];
	char mtext[MAX_MESSAGE_LENGTH];
}msgbuff;

typedef struct{
	char username[MAX_USERNAME_LENGTH];
	char password[MAX_USERNAME_LENGTH];
	int loggedin;
}user;

user *hash_table[MAX_USERS];

unsigned int hash(char *name){
	int length=strnlen(name,MAX_USERNAME_LENGTH);
	unsigned int hash_value=0;
	for(int i=0; i<length; i++){
		hash_value+=name[i];
		hash_value=(hash_value*name[i])%MAX_USERS;
	}
	return hash_value;
}

void init_hash_table(){
	for(int i=0; i<MAX_USERS; i++){
		hash_table[i]=NULL;
	}
}

void print_table(){
	for(int i=0; i<MAX_USERS; i++){
		if(hash_table[i]==NULL) printf("\t%i\t---\n",i);
		else printf("\t%i\t%s\n",i,hash_table[i]);
	}
}

int add_user(user *u){
	if(u==NULL) return 0;
	int index=hash(u->username);
	if(hash_table[index]!=NULL){
		return 0;
	}
	hash_table[index]=u;
	hash_table[index]->loggedin=0;
	return 1;
}

user *login(char *name, char *passwd){
	if(name==NULL) return NULL;
	int index=hash(name);
	if(hash_table[index]!=NULL){
		if(strncmp(hash_table[index]->password,passwd,MAX_USERS)==0){
			hash_table[index]->loggedin=1;
			return hash_table[index];	
		}
		else{
			printf("passwords do not match!\n");
			return NULL;
		}
	}
}

user *logout(char *name){
	if(name==NULL) return NULL;
	int index=hash(name);
	if(hash_table[index]!=NULL){
		hash_table[index]->loggedin=0;
		return hash_table[index];
	}
	return NULL;
}

user *hash_table_delete(char *name){
	int index=hash(name);
	if(hash_table[index]!=NULL && strncmp(hash_table[index]->username,name,MAX_USERS)==0){
		user *tmp=hash_table[index];
		hash_table[index]=NULL;
		return tmp;
	}
	else return NULL;
}

user *hash_table_find(char *name){
	int index=hash(name);
	if(hash_table[index]!=NULL && strncmp(hash_table[index]->username,name,MAX_USERS)==0) return hash_table[index];
	else return NULL;
}

void set_message(msgbuff *msg, long mtype, char *sender, char *body){
	msg->mtype=mtype;
	strcpy(msg->sender,sender);
	strcpy(msg->mtext,body);
}

char *strip_name(char username[MAX_USERNAME_LENGTH]){
	username[strlen(username)-1]='\0';
	return username;
}

int main(){
	init_hash_table();
	user joe={.username="joe", .password="mama"};
	user ignor={.username="ignor", .password="admin"};
	user twojstary={.username="twojstary", .password="pijany"};

	add_user(&joe);
	add_user(&ignor);
	add_user(&twojstary);
	print_table();

	msgbuff request;
	msgbuff response;
	int server=msgget(1234,0644 | IPC_CREAT);
	if(server==-1){
		perror("msgget");
		msgctl(server,IPC_RMID,NULL);
		exit(1);
	}

	while(1){
		if (msgrcv(server,&request,256,0,MSG_NOERROR)==-1){
			perror("msgrcv");
			exit(1);
		}	
		switch(request.mtype){
			case 1:
				char *name=strip_name(request.sender);
				char *passwd=strip_name(request.mtext);
				printf("Login request from %s...\t",name);
				user *tmp=login(name,passwd); 
				if(tmp){
					printf("OK!\n");
					set_message(&response,hash(name)+MAX_USERS,"server","Welcome back!\n");
				}
				else{
					set_message(&response,4,"server","Incorrect credentials!\n");
				}
				if(msgsnd(server,&response,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
				break;
			case 3:
				printf("Request from %s for userlist\n",strip_name(request.sender));
				char output[MAX_MESSAGE_LENGTH]="";
				char newline[1]="\n";
				for(int i=0; i<MAX_USERS; i++){
					if(hash_table[i]!=NULL){
						if(hash_table[i]->loggedin==1){
							printf("%s\n",hash_table[i]->username);
							strncat(output,hash_table[i]->username,MAX_USERNAME_LENGTH);
							strncat(output,newline,1);
						}
					}
				}
				strncat(output,newline,1);
				set_message(&response,2,"server",output);
				if(msgsnd(server,&response,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
				break;
			case 5:
				name=strip_name(request.sender);
				printf("Logout request from %s\n",name);
				if(logout(name)) set_message(&response,2,"server","Goodbye!\n");
				else printf("Something went wrong...\n");
				if(msgsnd(server,&response,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
		}
	}

	if(!msgctl(server,IPC_RMID,NULL)) printf("Queue removed successfully\n");
	return 0;
}
