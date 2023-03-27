var dgram = require('dgram');
var fs = require('fs');
var SocketIOServer = require('socket.io').Server;
var io;
var udpServer;

// Initialize a file stream to send stream data to
var fileStream;

const STREAM_PORT = 8080;
const STREAM_IP_ADDRESS = '192.168.7.1';

exports.listen = function(server) {
    io = new SocketIOServer(server);

    io.sockets.on('connection', (socket) => {
        console.log('A client has connected');
    
        // Handle disconnection of clients
        socket.on('disconnect', () => {
            console.log('Client disconnected');
        });
    });

    // Initialize socket to listen for the webcam streaming data
    udpServer = dgram.createSocket('udp4');
    udpServer.bind(STREAM_PORT, STREAM_IP_ADDRESS);

    // Send the data to the web socket to relay to clients
    // and create a file recording
    udpServer.on('message', (msg, rinfo) => {
        if (!fileStream) {
            fileStream = fs.createWriteStream(`recordings/${formatDate()}.mp4`);
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
}

exports.broadcast = function(data) {
    io.emit('data', JSON.stringify(data));
}

function formatDate() {
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
    return year + ":" + month + ":" + day + ":" + minutes + ":" + seconds;
}