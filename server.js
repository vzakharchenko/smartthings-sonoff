const express = require('express');
const server = express();
const port = process.env.PORT || 8080;


server.use(function(req, res, next) {
    res.header("Access-Control-Allow-Origin", "*");
    res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    res.header("Access-Control-Allow-Credentials", "true");
    next();
});

server.use('/', express.static(`${__dirname}/ui-module/public`));

server.listen(port, () => {
    console.info(`HTTP  listening on port ${port}`);
});
