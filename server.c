#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define MSGPERM 0640    // msg queue permission
/*
 * State:
 * 0-unconnected
 * 1-connected
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
    Army army;
};

struct Message {
    long type;
    char text[1024];
} msg;

void initial_values(Player players[]){
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
int main(int argc, char *argv[])
{
    Player players[3];
    void initial_values(players);
    return 0;
}