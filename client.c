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
bool is_correct_message(char text[]){
    if (!strcmp(text,"connect")) return true;
    return false;
}




void main_read(){
    while(1){
        read_msg();
        sleep(10);
    }
}



void main_write(){
    char text[1024];
    while(1){
        scanf("%s",text);
        if(is_correct_message(text)) send_msg(text);
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