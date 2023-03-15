// Code adapted from the sample code from jsmpeg's GitHub:
// https://github.com/phoboslab/jsmpeg/blob/master/view-stream.html

const WS_ADDR = location.hostname;
const WS_PORT = 8088;
const options = {
	canvas: document.getElementById("self-view")
};

new JSMpeg.Player(`ws://${WS_ADDR}:${WS_PEER_PORT}`, options);