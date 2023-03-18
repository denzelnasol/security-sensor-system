// import stream from "./lib/stream.js";
// stream.listen();
/*
    Code adapted from the sample code from jsmpeg's GitHub:
    https://github.com/phoboslab/jsmpeg/blob/master/websocket-relay.js
*/

import fs from "fs";
import dgram from "dgram";
import path from "path";


import express from 'express';
import { Server as SocketIOServer } from 'socket.io';

const __dirname = path.resolve();

const STREAM_PORT = 8080;
const STREAM_IP_ADDRESS = '192.168.7.1';
const WEBSOCKET_PORT = 8088;
const WEBSOCKET_IP_ADDRESS = '192.168.7.1';

const app = express();

app.use(express.static('public'));

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, '/public/index.html'));
});

const server = app.listen(WEBSOCKET_PORT, WEBSOCKET_IP_ADDRESS, () => {
    console.log(`server listening on port ${WEBSOCKET_PORT} with address`, server.address());
})

const io = new SocketIOServer(server);

io.on('connection', (socket) => {
    console.log('A client has connected');

    // Handle disconnection of clients
    socket.on('disconnect', () => {
        console.log('Client disconnected');
    });
});

// Initialize a file stream to send stream data to
const fileStream = fs.createWriteStream(`${Date.now().toString()}.mp4`);

// Initialize socket to listen for the webcam streaming data
const udpServer = dgram.createSocket('udp4');
udpServer.bind(STREAM_PORT, STREAM_IP_ADDRESS);

// Send the data to the web socket to relay to clients
// and create a file recording
udpServer.on('message', (msg, rinfo) => {
    // console.log("MESSAGES RECEIVED: ", msg);
    io.emit('stream', msg);
    fileStream.write(msg);
});
