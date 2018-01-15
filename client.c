#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>  // for strtol
#include <string.h>

#define MSGPERM 0640    // msg queue permission
int id;
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
} msg;

void sendMsg(char text[]){
    printf("%s\n",text);
}
void main_read(){
    printf("Information from server:\n");
}

void main_write(){
    char text[1024];
    while(1){
        scanf("%s",text);
        sendMsg(text);
        printf("%s\n",text);
    }
}
int main(int argc, char *argv[]) {
    id = strtol(argv[1], NULL, 10);
    printf("your id: %d\n",id);
    const int rw=strtol(argv[2], NULL, 10);
    if(rw==0)main_read();
    else if(rw==1)main_write();
    else printf("BAD ARGUMENT");
    return 0;
}