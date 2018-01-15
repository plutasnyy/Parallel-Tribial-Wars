#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#define queue 812357
#define MSGPERM 0640    // msg queue permission
/*
 * State:
 * 0-unconnected
 * 1-connected
 *
 * Message type:
 * 1-user->server
 * 2-server->user
 *
 */
struct Army{
    int lightInf;
    int heavyInf;
    int ride;
    int workers;
};
struct Player{
    int id;
    int state;
    int resources;
    struct Army army;
}players[3];

struct Message {
    long type;
    int id;
    char text[1024];
}msg;

void handle_request(struct Message msg){
    if(!strcmp(msg.text,"connect"))
        players[msg.id].state = 1;
}

void read_msg(){
    printf("la");
    struct Message msg;
    printf("lala");
    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    printf("j");
    int result = msgrcv(msqid, &msg, sizeof(msg), 1, 0);

    if (result==-1){
        printf("%s,Oh dear, something went wrong with read()! %s\n",errno, strerror(errno));
    }
    else{
        handle_request(msg);
        printf("From od %d: %s \n",msg.id,msg.text);
    }
}
void send_msg(int id, char text[]){
    struct Message msg;
    msg.type=2;
    msg.id=id;
    strcpy(msg.text,text);
    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    int result = msgsnd(msqid, &msg, sizeof(msg), 0);
    printf("%s,msqid:%d, result:%d\n",msg.text,msqid,result);

}
void initial_values(){
    for(int i=0;i<3;i++){
        players[i].id=i;
        players[i].state=0;
        players[i].resources=300;
        players[i].army.heavyInf=0;
        players[i].army.lightInf=0;
        players[i].army.ride=0;
        players[i].army.workers=0;
    }
}
int count_connected(){
    int sum=0;
    for(int i=0;i<3;i++)
        if(players[i].state!=0)
            sum++;
    return sum;
}
void send_all(char text[]){
    for(int i=0;i<3;i++){
        send_msg(i,text);
    }
}
int main(int argc, char *argv[])
{
    initial_values();
    while(count_connected() < 1){
        printf("la\n");
        read_msg();
        //send_all("Waiting for players");
    }
    printf("Ready\n");

    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    msgctl(msqid,IPC_RMID,0);

    return 0;
}