#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>  // for strtol
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#define MSGPERM 0640    // msg queue permission
#define queue 812359
long id;
/*
 * to run:
 * ./client id rw
 * id 0-2
 * rw = 0 READ 1 WRITE
 */
struct Message {
    long type;
    int id;
    char text[1024];
};

void read_msg(){
    struct Message msg;
    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    int result = msgrcv(msqid, &msg, sizeof(msg), 2+id, 0);

    if (result==-1){
        printf("%s,Oh dear, something went wrong with read()! %s\n",errno, strerror(errno));
    }
    else{
        if(msg.id == id){
            printf("%s\n",msg.text);
        }
    }
}
void send_msg(char text[]){
    struct Message msg;
    msg.type=1;
    msg.id = (int)id;
    strcpy(msg.text,text);
    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    int result = msgsnd(msqid, &msg, sizeof(msg), 0);
    printf("%s,msqid:%d, result:%d\n",msg.text,msqid,result);

}


void main_read(){
    while(1){
        read_msg();
        usleep(500);
    }
}
void send_request(int number, char text[],int input_values[]){
    char str[100];
    for(int i=0;i<number;i++){
        sprintf(str, "%d",input_values[i]);
        strcat(text,str);
        strcat(text," ");
    }
    send_msg(text);
}
void read_3_numbers(int input_values[],char text[]){
    strcat(text," ");
    scanf("%d %d %d",&input_values[0],&input_values[1],&input_values[2]);
    for(int i=0;i<3;i++)
        if(input_values[i]<0){
            printf("Bad values\n");
            return;
        }
    send_request(3,text,input_values);

}
void read_2_numbers(int input_values[],char text[]){
    strcat(text," ");
    scanf("%d %d %d %d",&input_values[0],&input_values[1],&input_values[2],&input_values[3]);
    for(int i=0;i<2;i++)
        if(input_values[i]<0 || input_values[0]>3){
            printf("Bad values\n");
            return;
        }
    send_request(2,text,input_values);
}

void main_write(){
    char text[1024];
    int input_values[4];
    int target;
    char str[100];
    while(1){
        scanf("%s",text);
        if(!strcmp(text,"connect")) {
            send_msg(text);
        }
        else if(!strcmp(text,"build")){
            read_2_numbers(input_values,text);
        }
        else if((!strcmp(text,"attack"))){
            scanf("%d",&target);
            if(target<0 || target > 3 ||target==id){
                printf("Bad values\n");
                return;
            }
            strcat(text," ");
            sprintf(str, "%d",target);
            strcat(text,str);
            read_3_numbers(input_values,text);

        }
        else printf("Incorrect message\n");
    }
}


int main(int argc, char *argv[]) {
    id = strtol(argv[1], NULL, 10);
    const long rw=strtol(argv[2], NULL, 10);
    if(rw==0)main_read();
    else if(rw==1)main_write();
    else printf("BAD ARGUMENT");
    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    msgctl(msqid,IPC_RMID,0);
    return 0;
}