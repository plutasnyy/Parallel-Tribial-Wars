#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <memory.h>
#include <unistd.h>
#define queue 812359
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

void send_msg(int id, char text[]){
    struct Message msg;
    msg.type=2;
    msg.id=id;
    msg.type=2+id;
    strcpy(msg.text,text);
    printf("wysylam:%s %s do %d\n",msg.text, text,msg.id);
    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    int result = msgsnd(msqid, &msg, sizeof(msg), 0);
    printf("%s,msqid:%d, result:%d\n",msg.text,msqid,result);

}
void handle_request(struct Message msg){
    if(!strcmp(msg.text,"connect")){
        players[msg.id].state = 1;
        send_msg(msg.id,"Waiting for players");
    }
}

void read_msg(){
    struct Message msg;
    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    int result = msgrcv(msqid, &msg, sizeof(msg), 1, 0);

    if (result==-1){
        printf("%s,Oh dear, something went wrong with read()! %s\n",errno, strerror(errno));
    }
    else{
        handle_request(msg);
        printf("From od %d: %s \n",msg.id,msg.text);
    }
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
}void waiting()
{
    for(int i=0;i<2;i++){
        if(players[i].state==1)
            send_msg(i,"Waiting for players");
    }
    while(count_connected() < 3){
        read_msg();
    }
}
int main(int argc, char *argv[])
{
    sleep(3);
    printf("Hi\n");
    initial_values();
    waiting();
    printf("Ready\n");
    send_all("Ready");
    sleep(20);
    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    msgctl(msqid,IPC_RMID,0);

    return 0;
}