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

#define queue 812360
#define MSGPERM 0640    // msg queue permission
#define PROD_QUEUE_SIZE 100

int end_capture = 1;
/*
 * State:
 * 0-unconnected
 * 1-connected
 *
 * Message type:
 * 1-user->server:
     * 1.connect
     * 2.attack
     * 3.build
 * 2-server->user
     * 2+id
 *
 * ARMY PARAMETEERS
 * ROW: 0-li 1-ci 2-ride 3-workers
 * COL: 0-cost 1-at 2-deff 3-prod time
 *
 * SEMAFORS:
 * 0,1,2 - resources for 0,1,2
 * 3,4,5 - workers
 * 6,7,8 - army
 * 9,10,11 - build queue
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
    sem.sem_op=-1;
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
    double resources;
    struct Army army;
    int points;
    int build_queue[PROD_QUEUE_SIZE];
}*players;

struct Message {
    long type;
    int id;
    char text[1024];
    int array[4];
};

void send_msg(int id, char text[]){
    struct Message msg;
    msg.type=2;
    msg.id=id;
    msg.type=2+id;
    strcpy(msg.text,text);
    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    int result = msgsnd(msqid, &msg, sizeof(msg), 0);
   // printf("Do: %d, msqid: %d, result: %d\n",msg.id,msqid,result);

}

void fight(int agg_id,int input_number[]){
    int deff_id=input_number[0];
    printf("FIGHTY sb\n");
    struct Player army_in_attack;
    //[0] - defender ID
    army_in_attack.army.lightInf = input_number[1];
    army_in_attack.army.heavyInf = input_number[2];
    army_in_attack.army.ride = input_number[3];

    printf("HEAVY: %d\n",players[agg_id].army.heavyInf);
    printf("ARMY HEAVY: %d\n",army_in_attack.army.heavyInf);

    sem_down(6+agg_id);
    players[agg_id].army.lightInf -= input_number[1];
    players[agg_id].army.heavyInf -= input_number[2];
    players[agg_id].army.ride -= input_number[3];
    sem_up(6+agg_id);

    printf("HEAVY: %d\n",players[agg_id].army.heavyInf);
    sleep(5);


    sem_down(6+deff_id);
    sem_down(6+agg_id);

    double attack_force = army_in_attack.army.lightInf*army_parameters[0][1]+army_in_attack.army.heavyInf*army_parameters[1][1]+army_in_attack.army.ride*army_parameters[2][1];
    double defend_force = players[deff_id].army.lightInf*army_parameters[0][2]+players[deff_id].army.heavyInf*army_parameters[1][2]+players[deff_id].army.ride*army_parameters[2][2];

    double at_wsp=1;
    if(attack_force!=0)at_wsp=defend_force/attack_force;//wspolczynnik strat

    double def_wsp=1;
    if(defend_force!=0)def_wsp=attack_force/defend_force;

    if(attack_force-defend_force>0){
        //SUCCES
        printf("%d pokonuje %d\n",agg_id,deff_id);

        players[deff_id].army.ride=0;
        players[deff_id].army.lightInf=0;
        players[deff_id].army.heavyInf=0;

        players[agg_id].points+=1;
        players[agg_id].army.lightInf += army_in_attack.army.lightInf*(1-at_wsp);
        players[agg_id].army.heavyInf += army_in_attack.army.heavyInf*(1-at_wsp);
        players[agg_id].army.ride += army_in_attack.army.ride*(1-at_wsp);

    }
    else{
        //DEFEND
        //W PDFie wzory byly chyba bledne i napisalem wlasna interpretacje,

        printf("%d pokonuje %d\n",deff_id,agg_id);

        players[deff_id].army.lightInf  *= (1-def_wsp);
        players[deff_id].army.heavyInf  *= (1-def_wsp);
        players[deff_id].army.ride  *= (1-def_wsp);

    }

    sem_up(6+deff_id);
    sem_up(6+agg_id);

    printf("HEAVY: %d\n",players[agg_id].army.heavyInf);
}

bool can_fight(int a[],int id){
    if(a[0]==id)
        return false;
    return a[1]<=players[id].army.lightInf && a[2]<=players[id].army.heavyInf && a[3]<=players[id].army.ride;
}

void add_to_production(int pl_id, int army_id, int quantity, double cost){
    int index = -1;

    printf("ADD\n");
    sem_down(9+pl_id);

    for(int i=0;i<PROD_QUEUE_SIZE-2;i++){
        if(players[pl_id].build_queue[i]==-1){
            index=i;
            break;
        }
    }
    printf("Index: %d Player id:%d\n",index,pl_id);
    if(index==-1){
        sem_up(9+pl_id);
        return;
    }
    players[pl_id].build_queue[index]=army_id;
    players[pl_id].build_queue[index+1]=quantity;

    sem_down(pl_id);
    players[pl_id].resources-=cost;
    sem_up(pl_id);

    sem_up(9+pl_id);


    for(int i=0;i<10;i++)
        printf("%d ",players[0].build_queue[i]);
    printf("\n");
}

void handle_request(struct Message msg) {
    if (msg.type == 1){
        printf("connect\n");
        if (players[msg.id].state == 0) {
            players[msg.id].state = 1;
            send_msg(msg.id, "Waiting for players");
        }
    }
    else if (msg.type == 2) {
        int input_numbers[4];
        memcpy(input_numbers, msg.array, sizeof(int) * 4);
        printf("%d %d %d %d\n",input_numbers[0],input_numbers[1],input_numbers[2],input_numbers[3]);
        if (can_fight(input_numbers, msg.id)) {
            if (fork() == 0) {
                fight(msg.id, input_numbers);
                exit(1);
            }
        } else {
            send_msg(msg.id, "Cant fight\n");
            printf("Cant fight\n");
        }
    }
    else if (msg.type == 3) {
        int input_numbers[4];
        memcpy(input_numbers, msg.array, sizeof(int) * 4);
        int army_id = input_numbers[0];
        int quantity = input_numbers[1];
        double cost = army_parameters[army_id][0] * quantity;
        printf("%d %d %lf\n",army_id,quantity,cost);
        if (players[msg.id].resources < cost) {
            send_msg(msg.id, "You are too poor");
        } else{
            printf("Uruchamiam produkcje:\n");
            add_to_production(msg.id,army_id,quantity,cost);
        }
    }
    else printf("Bad message\n");
}

void read_msg(){
    struct Message msg;
    int msqid = msgget(queue, MSGPERM|IPC_CREAT);
    int result = msgrcv(msqid, &msg, sizeof(msg)-sizeof(long), 0, 0);

    if (result==-1){
        perror("Error:");
        printf("%s,Something went wrong %s\n",errno, strerror(errno));
    }
    else{
        handle_request(msg);
        printf("From od %d \n",msg.id);
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
    sprintf(str, "%lf", players[i].resources);
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

bool stop_condition(){
    return players[0].points<5 && players[1].points<5 && players[2].points<5 && end_capture;
}

void send_state(){
    while(stop_condition()){
        for(int i=0;i<1;i++){
            char array[1024];
            generate_state_message(i,array);
            send_msg(i,array);
        }

        sleep(1);
    }
}

void build_army(int id){
    while(stop_condition()){
        sem_down(9+id);
        if(players[id].build_queue[0]==-1){
            sem_up(9+id);
            continue;
        }
        else{
            int army_id = players[id].build_queue[0];
            int quantity = players[id].build_queue[1];
            printf("PRODUCTION: %d %d\n",army_id,quantity);
            printf("SWAP\n");
            for(int i=0;i<10;i++)
                printf("%d ",players[0].build_queue[i]);
            printf("\n");
            for(int i=0;i<PROD_QUEUE_SIZE-3;i++){
                if(players[id].build_queue[i]==-1)break;
                else players[id].build_queue[i]=players[id].build_queue[i+2];
            }
            for(int i=0;i<10;i++)
                printf("%d ",players[0].build_queue[i]);
            printf("\n");
            sem_up(9+id);

            for(int i=0;i<quantity;i++){
                sleep(((unsigned int) army_parameters[army_id][3]));
                if(army_id==3)sem_down(3+id);
                else sem_down(6+id);
                printf("UP: %d\n",army_id);
                if(army_id==0) players[id].army.lightInf+=1;
                else if (army_id==1) players[id].army.heavyInf+=1;
                else if (army_id==2) players[id].army.ride+=1;
                else if (army_id==3) players[id].army.workers+=1;
                else printf("BAD ID???\n");

                if(army_id==3)sem_up(3+id);
                else sem_up(6+id);
            }
        }
        usleep(33);
    }
}

void start_build_army(){
    for(int i=0;i<3;i++){
        if(fork()==0){
            build_army(i);
            exit(1);
        }
    }
}

void production(int id){
    while(stop_condition()){
        sem_down(id);
        sem_down(3+id);
        players[id].resources += 50 + players[id].army.workers * 5;
        sem_up(3+id);
        sem_up(id);
        sleep(1);
    }
}

void start_production(){
    for(int i=0;i<3;i++){
        if(fork()==0){
            production(i);
           // exit(1);
        }
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
        players[i].army.heavyInf=100/(i+1);
        players[i].army.lightInf=(i+1)*20;
        players[i].army.ride=50+2*i;
        players[i].army.workers=i;
        players[i].points=0;
        for(int j=0;j<PROD_QUEUE_SIZE;j++)
            players[i].build_queue[j]=-1;
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
    int id = shmget(12354,sizeof(ptr[3]),IPC_CREAT|0640);
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
    start_build_army();

    if(fork()==0)send_state();

    while(stop_condition()){
        for(int i=0;i<10;i++)
            printf("%d ",players[0].build_queue[i]);
        printf("\n");
        read_msg();
    }

    printf("Exit\n");
    msqid = msgget(queue, MSGPERM|IPC_CREAT);
    msgctl(msqid,IPC_RMID,0);
    shmctl(id,IPC_RMID,SHM_RND);

    return 0;
}