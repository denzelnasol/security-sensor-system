#ifndef ETHERNET_EHOST_H_
#define ETHERNET_EHOST_H_


void ServerNet_init(void);

// will add a 0 character at the end of message
void ServerNet_receive(char *message);


// sends everything in message to host
void ServerNet_send(const char *message, int numBytes);

void ServerNet_cleanup(void);


#endif