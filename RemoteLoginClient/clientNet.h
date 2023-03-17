/**
 * The ClientNet module handles sending/receiving messages, and socket cleanup
 * for a UDP connection.
 */
#ifndef CLIENT_NETWORK_H_
#define CLIENT_NETWORK_H_

// initialize networking module
void ClientNet_init(void);

// will end message with the '0' character to make it a string
void ClientNet_receive(char *message);

// sends everything in message to host
void ClientNet_send(const char *message, int numBytes);

void ClientNet_cleanup(void);

#endif