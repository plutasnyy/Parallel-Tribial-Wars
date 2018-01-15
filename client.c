#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>

#define MSGPERM 0640    // msg queue permission

struct Message {
    long type;
    char text[1024];
} msg;

int main(int argc, char *argv[]) {


}