/**
 * The ServerNet module handles sending/receiving messages, and socket cleanup
 * for a UDP connection.
 */
#ifndef SERVER_NETWORK_H_
#define SERVER_NETWORK_H_

// initialize networking module
void ServerNet_init(void);

// will end message with the '0' character to make it a string
void ServerNet_receive(char *message);

// sends everything in message to host
void ServerNet_send(const char *message, int numBytes);

void ServerNet_cleanup(void);

#endif