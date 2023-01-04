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
	char receiver[MAX_USERNAME_LENGTH];
	char mtext[MAX_MESSAGE_LENGTH];
}msgbuff;

typedef struct{
	char username[MAX_USERNAME_LENGTH];
	char password[MAX_USERNAME_LENGTH];
	int loggedin;
}user;

user *hash_table[MAX_USERS];

int hash(char *name){
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
	printf("\n");
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

void set_message(msgbuff *msg, long mtype, char *sender, char *receiver, char *body){
	msg->mtype=mtype;
	strcpy(msg->sender,sender);
	strcpy(msg->receiver,receiver);
	strcpy(msg->mtext,body);
}

char *strip_name(char username[MAX_USERNAME_LENGTH]){
	username[strlen(username)-1]='\0';
	return username;
}

char *dump_groups(int *groups[], char output[MAX_MESSAGE_LENGTH]){
	char start[]="List of user groups:\n";
	strncat(output,start,MAX_MESSAGE_LENGTH);
	char space[1]=" ";
	char newline[1]="\n";
	char colon[1]=":";
	char a[1]=" ";
	for(int i=0; i<3; i++){
		a[0]=i+'0';
		char group_name[7]="group";
		strncat(group_name,a,1);
		strncat(output,group_name,MAX_MESSAGE_LENGTH);
		strncat(output,colon,1);
		strncat(output,space,1);
		for(int j=0; j<MAX_USERS; j++){
			if(groups[i][j]==-1) break;
			strncat(output,hash_table[groups[i][j]]->username,MAX_USERNAME_LENGTH);
			strncat(output,space,1);
		}
		strncat(output,newline,1);
	}
	strncat(output,newline,1);
	return output;
}

int main(){
	init_hash_table();
	user test1={.username="test1", .password="1"};
	user test2={.username="test2", .password="2"};
	user test3={.username="test3", .password="3"};
	user test4={.username="test4", .password="4"};
	user filip={.username="filip", .password="krol"};

	add_user(&test1);
	add_user(&test2);
	add_user(&test3);
	add_user(&test4);
	add_user(&filip);

	int group1[4]={hash("test1"),hash("test2"),hash("test3"),-1};
	int group2[3]={hash("test1"),hash("test4"),-1};
	int group3[5]={hash("test2"),hash("test3"),hash("test4"),hash("filip"),-1};

	int *groups[3]={group1,group2,group3};
	
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
		if (msgrcv(server,&request,256,-MAX_USERS+1,MSG_NOERROR)==-1){
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
					set_message(&response,hash(name)+MAX_USERS,"server",name,"Welcome back!\n");
				}
				else{
					set_message(&response,403,"server",name,"Incorrect credentials!\n");
				}
				if(msgsnd(server,&response,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
				break;
			case 2:
				name=strip_name(request.sender);
				printf("Request from %s for userlist\n",name);
				char output[MAX_MESSAGE_LENGTH]="";
				char newline[1]="\n";
				for(int i=0; i<MAX_USERS; i++){
					if(hash_table[i]!=NULL){
						if(hash_table[i]->loggedin==1){
							strncat(output,hash_table[i]->username,MAX_USERNAME_LENGTH);
							strncat(output,newline,1);
						}
					}
				}
				strncat(output,newline,1);
				set_message(&response,hash(name)+MAX_USERS,"server",name,output);
				if(msgsnd(server,&response,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
				break;
			case 3:
				name=strip_name(request.sender);
				printf("Logout request from %s\n",name);
				if(logout(name)) set_message(&response,hash(name)+MAX_USERS,"server",name,"Goodbye!\n");
				else printf("Something went wrong...\n");
				if(msgsnd(server,&response,MAX_MESSAGE_LENGTH,0)==-1) perror("msgsnd");
				break;
			case 4:
				char *from=strip_name(request.sender);
				char *to=strip_name(request.receiver);
				printf("User %s wants to send a message to %s\t farwarding...\n",from,to);
				set_message(&response,hash(to)+MAX_USERS,from,to,request.mtext);
				msgsnd(server,&response,MAX_MESSAGE_LENGTH,0);
				break;
			case 5:
				name=strip_name(request.sender);
				memset(output,0,strlen(output));
				printf("Request from %s for group list\n",name);
				char *groups_string=dump_groups(groups,groups_string);
				printf("%s\n",groups_string);
				set_message(&response,hash(name)+MAX_USERS,"server",name,groups_string);
				break;
		}
	}

	if(!msgctl(server,IPC_RMID,NULL)) printf("Queue removed successfully\n");
	return 0;
}
