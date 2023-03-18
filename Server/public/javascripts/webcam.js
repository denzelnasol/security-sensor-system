
/**
 * Code referenced from jsmpeg github issues by user jvictorsoto:
 * https://github.com/phoboslab/jsmpeg/issues/199
 */
class JSMpegWritableSource {
      constructor(url, options) {
            this.destination = null;

            this.completed = false;
            this.established = false;
            this.progress = 1;

            // Streaming is obiously true when using a stream
            this.streaming = true;
      }

      connect(destination) {
            this.destination = destination;
      }

      start() {
            this.established = true;
            this.completed = true;
            this.progress = 1;
      }

      resume() { // eslint-disable-line class-methods-use-this

      }

      destroy() { // eslint-disable-line class-methods-use-this
      }

      write(data) {
            this.destination.write(data);
      }
}

const socket = io.connect('http://192.168.7.1:8088');

const canvas = $("#self-view")[0];

const streamPlayer = new JSMpeg.Player(null, {
      source: JSMpegWritableSource,
      canvas
});

socket.on('stream', (data) => {
      streamPlayer.source.write(data);
});
