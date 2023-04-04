/*
    Code adapted from the sample code from jsmpeg's GitHub:
    https://github.com/phoboslab/jsmpeg/blob/master/websocket-relay.js
*/
var fs   = require('fs');
var path = require('path');
var express = require('express');

var analyzer = require('./lib/analyzer');
var sioserver = require('./lib/udp_server');
// var sioserver = require('./lib/socketio_server');

const WEBSOCKET_PORT = 8088;
const WEBSOCKET_IP_ADDRESS = '10.128.0.2';

const app = express();

app.use(express.static('public'));

app.get('/client', (req, res) => {
    res.sendFile(path.join(__dirname, '/public/index.html'));
});

app.get('/client/recordings', function (req, res) {
    const dirPath = './recordings';

    fs.readdir(dirPath, function (err, files) {
        if (err) {
            console.log(err);
            return res.status(500).send('Error reading directory');
        }
        res.json(files);
    });
});

app.get('/client/data', function (req, res) {
    res.json(analyzer.getDataPoints());
});
app.post('/client/data', function (req, res) {
    analyzer.addDataPoint(req.body);
    res.sendStatus(201);
});


app.get('/client/recordings/:id', function (req, res) {
    const filePath = path.join(__dirname, `recordings/${req.params.id}`);
    const fileStream = fs.createReadStream(filePath);
    fileStream.pipe(res);
});


const server = app.listen(WEBSOCKET_PORT, () => {
    console.log(`server listening on port ${WEBSOCKET_PORT} with address`, server.address());
})

// set up socket io for sending data to clients
sioserver.listen(server);
