/*
	Code adapted from the sample code from jsmpeg's GitHub:
	https://github.com/phoboslab/jsmpeg/blob/master/websocket-relay.js
*/

import WebSocket, {WebSocketServer} from "ws";
import dgram from "dgram";

const UDP_STREAM_ADDR = "192.168.7.1";
const UDP_STREAM_PORT = 8080;
const WS_PORT = 8088;
const wSocketServer = new WebSocketServer({
	perMessageDeflate: false,
	port: WS_PORT
});

wSocketServer.broadcast = (data) => {
	wSocketServer.clients.forEach((client) => {
		if (client.readyState === WebSocket.OPEN) {
			client.send(data);
		}
	});
};
wSocketServer.connectionCount = 0;
wSocketServer.on("connection", (socket, upgradeReq) => {
	wSocketServer.connectionCount++;
	wSocketServer.on("close", (code, message) => wSocketServer.connectionCount--);
});

export default {
	listen: () => {
		const udpSocket = dgram.createSocket("udp4");

		udpSocket.bind(UDP_STREAM_PORT, UDP_STREAM_ADDR);
		udpSocket.on("listening", () => {});
		udpSocket.on("message", (chunk, rinfo) => wSocketServer.broadcast(chunk));
	}
};