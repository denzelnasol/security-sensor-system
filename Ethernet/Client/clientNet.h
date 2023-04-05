#ifndef ETHERNET_ECLIENT_H_
#define ETHERNET_ECLIENT_H_


void ClientNet_init(void);

// will add a 0 character at the end of message
bool ClientNet_receive(char *message);


// sends everything in message to host
void ClientNet_send(const char *message, int numBytes);

void ClientNet_cleanup(void);

#endif
