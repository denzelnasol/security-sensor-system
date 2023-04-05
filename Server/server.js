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
const WEBSOCKET_IP_ADDRESS = 'localhost';

const app = express();

app.use(express.json());
app.use(express.urlencoded({extended:false}));
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

app.get('/data', function (req, res) {
    let data = analyzer.getDataPoints();
    let threshold = req.body.dangerThreshold;
    res.json({ ...data, dangerThreshold: threshold });
});
app.post('/data', function (req, res) {
    analyzer.addDataPoint(req.body);
    res.sendStatus(201);
});


app.get('/recordings/:id', function (req, res) {
    const filePath = path.join(__dirname, `recordings/${req.params.id}`);
    const fileStream = fs.createReadStream(filePath);
    fileStream.pipe(res);
});


const server = app.listen(WEBSOCKET_PORT, () => {
    console.log(`server listening on port ${WEBSOCKET_PORT} with address`, server.address());
})

// set up socket io for sending data to clients
sioserver.listen(server);
