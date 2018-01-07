#include <stdio.h>
//IPC
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
//SLEEP
#include <unistd.h>
//strcpy
#include <string.h>

#define MSGPERM 0640    // msg queue permission

struct message {
    long type;
    char* a;
} msg;

int main(int argc, char *argv[]) {
    int msqid = msgget(812355, MSGPERM|IPC_CREAT);
    int result = msgsnd(msqid, &msg, sizeof(msg.a), 0);
    char a[50];
    int i = fork();
    if (i==0) {
        while(1) {
            printf("Wiadomosc z serwera\n");
            sleep(50);
        }
    }
    else {
        while(1){
            printf("Podaj wiadomosc:");
            sleep(50);
            scanf("%s",&a);
            printf("Komenda do wyslania: %s\n",a);
        }
    }
}