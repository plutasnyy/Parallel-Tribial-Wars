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
#define queue 812360
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
    int array[4];
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
void send_msg(int type, int array[]){
    struct Message msg;
    msg.type = type;
    msg.id = (int)id;
    memcpy(msg.array, array, sizeof(int) * 4);
    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    int result = msgsnd(msqid, &msg, sizeof(msg)-sizeof(long), 0);
    printf("%s,msqid:%d, result:%d\n",msg.text,msqid,result);

}

void main_read(){
    while(1){
        read_msg();
        usleep(1000);
    }
}

void read_3_numbers(int input_values[]){
    scanf("%d %d %d",&input_values[1],&input_values[2],&input_values[3]);
    for(int i=1;i<4;i++)
        if(input_values[i]<0){
            printf("Bad values\n");
            return;
        }
    send_msg(2,input_values);
}

void read_2_numbers(int input_values[]){
    scanf("%d %d",&input_values[0],&input_values[1]);
    for(int i=0;i<2;i++)
        if(input_values[i]<0 || input_values[0]>3){
            printf("Bad values\n");
            return;
        }
    send_msg(3,input_values);
}

void main_write(){
    char command[10];
    int input_values[4];
    int target;
    while(1){
        scanf("%s",command);
        if(!strcmp(command,"connect")) {
            send_msg(1, input_values) ;
        }
        else if(!strcmp(command,"build")){
            read_2_numbers(input_values);
        }
        else if((!strcmp(command,"attack"))){
            scanf("%d",&target);
            if(target<0 || target > 3 ||target==id){
                printf("Bad values\n");
                continue;
            }
            read_3_numbers(input_values);
        }
        else if((!strcmp(command,"exit"))) {
            break;
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