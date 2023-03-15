"use strict";

import fs from "fs";
import http from "http";
import mime from "mime";
import path from "path";
import stream from "./lib/stream.js";

const PORT = 2048;
let server = http.createServer((request, response) => {
    let file;

    if (request.url === "/") {
        file = "./login.html";
    }
    else {
        file = "." + request.url;
    }

    fs.readFile(file, (err, data) => {
        let headers;

        if (err) {
            headers = {
                "Content-Type": "text/plain"
            };
            response.writeHead(404, headers);
            response.write("Error 404: Resource not found.");
            response.end();
        }
        else {
            headers = {
                "content-type": mime.getType(path.basename(file))
            };
            response.writeHead(200, headers);
            response.end(data);
        }
    });
});

server.listen(PORT, () => console.log(`Server has been created. Listening on port ${PORT}...`));
stream.listen();