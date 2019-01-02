const express = require('express');
const fs = require('fs');
const server = express();
const port = 8082;
const remotePort=`${process.env.remotePort||8080}`;


server.use(function(req, res, next) {
    res.header("Access-Control-Allow-Origin", "http://localhost:"+remotePort);
    res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    res.header("Access-Control-Allow-Credentials", "true");
    next();
});

server.use('/', express.static(`${__dirname}/static`));

server.post('/config', (req, res) => {
    fs.readFile(`${__dirname}/static/info`, function read(err, data) {
        if (err) {
            throw err;
        }
        content = data;

        console.log(content);
        res.send(content);
    });
});

server.post('/toggle', (req, res) => {
    fs.readFile(`${__dirname}/static/info`, function read(err, data) {
        if (err) {
            throw err;
        }
        content = data;

        console.log(content);
        res.send(content);
    });
});

server.listen(port, () => {
    console.info(`HTTP  listening on port ${port}`);
});
