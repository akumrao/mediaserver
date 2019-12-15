#!/usr/bin/env node
'use strict';

var https = require('https');
var port = process.argv[2] || 8043;
var fs = require('fs');
var path = require('path');
var server;
var options = {
      key: fs.readFileSync('/var/tmp/key/privkey.pem')
    , cert: fs.readFileSync('/var/tmp/key/fullchain.pem')
    };

function app(req, res) {
  res.setHeader('Content-Type', 'text/plain');
  res.end('Hello, encrypted world!');
}

server = https.createServer(options, app).listen(port, function () {
  port = server.address().port;
  console.log('Listening on https://127.0.0.1:' + port);
  console.log('Listening on https://' + server.address().address + ':' + port);
  console.log('Listening on https://localhost' + port);
});
