#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>  // for strtol
#include <string.h>
#include <stdbool.h>
#define MSGPERM 0640    // msg queue permission
long id;
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
    strcpy(msg.text,text);
    printf("%s\n",msg.text);

}
bool is_correct_message(char text[]){
    if (!strcmp(text,"connect")) return true;
    return false;
}




void main_read(){
    printf("Information from server:\n");
}



void main_write(){
    char text[1024];
    while(1){
        scanf("%s",text);
        if(is_correct_message(text)) sendMsg(text);
        else printf("Incorrect message\n");
        break;
    }
}






int main(int argc, char *argv[]) {
    id = strtol(argv[1], NULL, 10);
    const long rw=strtol(argv[2], NULL, 10);
    if(rw==0)main_read();
    else if(rw==1)main_write();
    else printf("BAD ARGUMENT");
    return 0;
}