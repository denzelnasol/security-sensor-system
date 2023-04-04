
// var socketio_server = require('./socketio_server');
var socketio_server = require('./udp_server');

const MAX_DATA_POINTS = 14;

var dataPointsDangerLevel = [];
// var dataPointsTimestamp = [];
var dataLabels = [];
var currentDangerLevel = 0;


for (let i = 1; i <= MAX_DATA_POINTS; i++) {
    dataLabels.push(i.toString());
}

for (let i = 1; i <= MAX_DATA_POINTS; i++) {
    let r = Math.random();
    dataPointsDangerLevel.push(r);
    currentDangerLevel = r;
}


exports.addDataPoint = function(data) {
    currentDangerLevel = data.dangerLevel;

    if (dataPointsDangerLevel.length >= MAX_DATA_POINTS) {
        dataPointsDangerLevel.shift();
        // dataPointsTimestamp.shift();
    }
    dataPointsDangerLevel.push(data.dangerLevel);
    // dataPointsTimestamp.push(data.timestamp);

    socketio_server.broadcast(this.getDataPoints());
};

exports.getDataPoints = function() {
    return {
        dangerLevel: currentDangerLevel,
        dangerPoints: dataPointsDangerLevel,
        dangerLabels: dataLabels,
    };
}




