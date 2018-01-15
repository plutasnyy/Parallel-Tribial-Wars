#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>

#define MSGPERM 0640    // msg queue permission
const int id;
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
void read(){
    printf("Information from server:\n");
}

void write(){
    char text[1024];
    while(1){
        scanf("%s",text);
        sendMsg(text);
        printf("%s\n",text);
    }
}
int main(int argc, char *argv[]) {

    id = argv[1];
    printf("your id:\n",id);
    const int rw = argv[2];
    if(rw==0)read();
    else if(rw==1)write();
    else printf("BAD ARGUMENT");
    return 0;
}