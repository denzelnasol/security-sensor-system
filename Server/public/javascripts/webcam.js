// Code adapted from the sample code from jsmpeg's GitHub:
// https://github.com/phoboslab/jsmpeg/blob/master/view-stream.html const WS_ADDR = "192.168.7.2";
const WS_PORT = 8088;
const options = {
      canvas: $("#self-view")
 };
new JSMpeg.Player(`ws://${WS_ADDR}:${WS_PORT}`, options);