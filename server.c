#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
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
 *
 * SEMAFORS:
 * 0,1,2 - resources for 0,1,2
 * 3,4,5 - workers
 * 6,7,8 - army
 */


int id_group_sem;
struct sembuf sem;
void sem_up(int num){
    sem.sem_num=num;
    sem.sem_op=1;//podnies
    sem.sem_flg=0;
    semop(id_group_sem, &sem, 1);
}
void sem_down(int num){
    sem.sem_num=num;
    sem.sem_op=0;
    sem.sem_flg=0;
    semop(id_group_sem, &sem, 1);
}

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
}*players;

struct Message {
    long type;
    int id;
    char text[1024];
};

void send_msg(int id, char text[]){
    struct Message msg;
    msg.type=2;
    msg.id=id;
    msg.type=2+id;
    strcpy(msg.text,text);
    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    int result = msgsnd(msqid, &msg, sizeof(msg), 0);
    printf("Do: %d, msqid: %d, result: %d\n",msg.id,msqid,result);

}

bool can_fight(int a[],int id){
    if(a[0]==id)
        return false;
    return a[1]<=players[id].army.lightInf && a[2]<=players[id].army.heavyInf && a[3]<=players[id].army.ride;
}

void read_long_request(int index, int items, char text[],int input_numbers[]){
    int j;
    for(int i=0;i<items;i++){
        char number[100];
        j=0;
        while(text[index]!=' '){
            number[j++]=text[index++];
        }
        index++;
        input_numbers[i] = atoi(number);
    }
}

void handle_request(struct Message msg){
    if(!strcmp(msg.text,"connect")){
        players[msg.id].state = 1;
        send_msg(msg.id,"Waiting for players");
    }
    else{
        char text[1024];
        int i=0;
        while(msg.text[i]!=' '){
            text[i]=msg.text[i];
            i++;
            if(i==1023)break;
        }
        i++;
        if(!strcmp(text,"attack")){
            int input_numbers[3];
            read_long_request(i,3,msg.text,input_numbers);
            if(can_fight(input_numbers,msg.id)){
                printf("Walka!\n");
            }
            else{
                printf("Walka niemozliwa\n");
            }
        }
        else if(!strcmp(text,"build")){
            int input_numbers[2];
            read_long_request(i,2,msg.text,input_numbers);
            int army_id=input_numbers[1];
            int quantity=input_numbers[2];
            double cost=army_parameters[army_id][0]*quantity;
            if(players[msg.id].resources<cost)
                send_msg(msg.id,"You are too poor");
            else
                printf("Production... %d\n",msg.id);

        }
        else printf("bad message\n");
    }
}

void read_msg(){
    struct Message msg;
    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    int result = msgrcv(msqid, &msg, sizeof(msg), 1, 0);

    if (result==-1){
        perror("Error:");
        printf("%s,Oh dear, something went wrong with read()! %s\n",errno, strerror(errno));
    }
    else{
        handle_request(msg);
        printf("From od %d: %s \n",msg.id,msg.text);
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


void generate_state_message(int i, char array[]){
    char str[100];
    strcpy(array,"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nResources: ");
    sprintf(str, "%d", players[i].resources);
    strcat(array,str);
    strcat(array,"\nLight Inf: ");
    sprintf(str, "%d", players[i].army.lightInf);
    strcat(array,str);
    strcat(array,"\nHeavy Inf: ");
    sprintf(str, "%d", players[i].army.heavyInf);
    strcat(array,str);
    strcat(array,"\nRide: ");
    sprintf(str, "%d", players[i].army.ride);
    strcat(array,str);
    strcat(array,"\nWorkers: ");
    sprintf(str, "%d", players[i].army.workers);
    strcat(array,str);

}

void send_state(){
    for(int i=0;i<3;i++){
        char array[1024];
        generate_state_message(i,array);
       // printf("%s",array);
        send_msg(i,array);
    }
    exit(1);
}

bool stop_condition(){
    return true;
}

void production(int id){
    while(1){
        sem_down(id);
        players[id].resources += 50 + players[id].army.workers * 5;
        sem_up(id);
        sleep(1);
    }
}

void start_production(){
    for(int i=0;i<3;i++){
        if(fork()==0)
            production(i);
    }
}

void waiting(){
    for(int i=0;i<3;i++){
        if(players[i].state==1)
            send_msg(i,"Waiting for players");
    }
    while(count_connected() < 1){
        read_msg();
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
        players[i].army.workers=i;
        players[i].points=0;
    }
}

int main(int argc, char *argv[])
{
    //SEMAFORY
    int temp = semget(12345,9,IPC_CREAT);
    id_group_sem = temp;
    semctl (id_group_sem, 0, SETALL, 1);

    //PAMIEC WSPOLDZIELONA
    struct Player ptr[3];
    int id = shmget(12352,sizeof(ptr[3]),IPC_CREAT|0640);
    printf("id: %d\n",id);
    players = (struct Player*)shmat(id,NULL,0);
    printf("Shmat: %d\n",ptr);

    //SKASOWANIE KOLEJKI DO CZYSZCZENIA
    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    msgctl(msqid,IPC_RMID,0);

    printf("Hi\n");
    initial_values();

    waiting();


    printf("Ready\n");
    send_all("Ready");

    start_production();

    while(stop_condition()){
        if(fork()==0)send_state();
        else read_msg();
        sleep(1);
    }
    printf("wyszedlem\n");
    msqid = msgget(queue, MSGPERM|IPC_CREAT);
    msgctl(msqid,IPC_RMID,0);
    shmctl(id,IPC_RMID,SHM_RND);

    return 0;
}