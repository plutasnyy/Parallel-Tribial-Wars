#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <errno.h>
#define MSGPERM 0640    // msg queue permission

struct message {
    long type;
    int a;
} msg;


int main(int argc, char *argv[])
{
    int msqid = msgget(812355, MSGPERM|IPC_CREAT|IPC_EXCL);
    int result = msgrcv(msqid, &msg, sizeof(msg.a), 1, 0);

    if (result==-1){
        printf("%s,Oh dear, something went wrong with read()! %s\n",errno, strerror(errno));
    }
    printf("Wiadomosc: %s \n",msg.text);
    return 0;
}