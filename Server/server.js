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

app.get('/recordings', function (req, res) {
    const dirPath = './recordings';

    fs.readdir(dirPath, function (err, files) {
        if (err) {
            console.log(err);
            return res.status(500).send('Error reading directory');
        }
        res.json(files);
    });
});


app.get('/recordings/:id', function (req, res) {
    const filePath = path.join(__dirname, `recordings/${req.params.id}`);
    const fileStream = fs.createReadStream(filePath);
    fileStream.pipe(res);
});


const server = app.listen(WEBSOCKET_PORT, WEBSOCKET_IP_ADDRESS, () => {
    console.log(`server listening on port ${WEBSOCKET_PORT} with address`, server.address());
})

const io = new SocketIOServer(server);

io.sockets.on('connection', (socket) => {
    console.log('A client has connected');

    // Handle disconnection of clients
    socket.on('disconnect', () => {
        console.log('Client disconnected');
    });
});

// Initialize a file stream to send stream data to
let fileStream;

// Initialize socket to listen for the webcam streaming data
const udpServer = dgram.createSocket('udp4');
udpServer.bind(STREAM_PORT, STREAM_IP_ADDRESS);

// Send the data to the web socket to relay to clients
// and create a file recording
udpServer.on('message', (msg, rinfo) => {
    if (!fileStream) {
        /**
         * Source to get date
         * https://stackoverflow.com/questions/12409299/how-to-get-current-formatted-date-dd-mm-yyyy-in-javascript-and-append-it-to-an-i
         */
        const today = new Date();
        const year = today.getFullYear();
        const month = today.getMonth() + 1; // Months start at 0!
        const day = today.getDate();
        const minutes = today.getMinutes();
        const seconds = today.getSeconds();
        const formattedToday = year + ":" + month + ":" + day + ":" + minutes + ":" + seconds;
        fileStream = fs.createWriteStream(`recordings/${formattedToday}.mp4`);
    } else {
        fileStream.write(msg);
    }

    io.emit('stream', msg);
});

udpServer.on('close', () => {
    if (fileStream) {
        fileStream.end();
    }
});

udpServer.on('error', (err) => {
    console.log(`server error: ${err.stack}`);
});