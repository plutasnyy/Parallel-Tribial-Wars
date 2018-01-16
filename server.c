#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <memory.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
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
 * ARMY PARAMETEERS
 * ROW: 0-li 1-ci 2-ride 3-workers
 * COL: 0-cost 1-at 2-deff 3-prod time
 */

double army_parameters[4][4] = {{100,1,1.2,2},{250,1.5,3,3},{550,3.5,1.2,5},{150,0,0,2}};

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
    int points;
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
        players[i].points=0;
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
    while(count_connected() < 1){
        read_msg();
    }
}
bool stop_condition(){
    return true;
}
void generate_state_message(int i, char array[]){
    char str[100];
    strcpy(array,"Resources: ");
    sprintf(str, "%d", players[i].resources);
    strcat(array,str);
    strcat(array,"\nWorkers: ");
    sprintf(str, "%d", players[i].army.workers);
    strcat(array,str);
    strcat(array,"\nRide: ");
    sprintf(str, "%d", players[i].army.ride);
    strcat(array,str);
    strcat(array,"\nLight Inf: ");
    sprintf(str, "%d", players[i].army.lightInf);
    strcat(array,str);
    strcat(array,"\nHeavy Inf: ");
    sprintf(str, "%d", players[i].army.heavyInf);
    strcat(array,str);

}
void send_state(){
    for(int i=0;i<3;i++){
        char array[1024];
        generate_state_message(i,array);
        printf("%s",array);
        send_msg(i,array);
    }
}
int main(int argc, char *argv[])
{

    printf("Hi\n");
    initial_values();
    waiting();
    printf("Ready\n");
    send_all("Ready");
    sleep(10);
    while(stop_condition()){
        send_state();
        sleep(3);
    }
    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    msgctl(msqid,IPC_RMID,0);

    return 0;
}