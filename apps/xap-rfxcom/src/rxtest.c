#include <stdio.h>
#include <sys/msg.h>

#define MSGQUEUE_X10RF	8008	// Message queue ID for X10RF commands to send
#define BYTE			unsigned char

// X10 Message
typedef struct {
	long msg_id;
	char device[4];				// X10 Address (A1 to P16)
	BYTE funct;					// X10 Function
} t_X10RF_MSG;

int main(void) {
	
	t_X10RF_MSG x10RfMsg;
	int x10RfMsgQueue;			// message queue
	int i;

	// Create a public message queue with access only for the owner
	if ((x10RfMsgQueue = msgget(MSGQUEUE_X10RF, IPC_CREAT | 0644)) == -1)
		return -1;
		
	printf("Running...\n");
		
	while(1) {
		msgrcv(x10RfMsgQueue, (struct msgbuf *)&x10RfMsg, sizeof(x10RfMsg.device)+sizeof(x10RfMsg.funct), 2, 0);
		printf("Disp: %s - Funcion: %d\n", x10RfMsg.device, x10RfMsg.funct);
	}
}
