#!/usr/bin/env node
'use strict';

var fs = require('fs');
var path = require('path');
var request = require('request');

var ca = fs.readFileSync(path.join(__dirname, 'certs', 'client', 'chain.pem'));
var port = process.argv[2] || 8043;
var hostname = process.argv[3] || 'localhost.daplie.com';

var options = {
      url: 'https://' + hostname + ':' + port
    , agentOptions: {
        ca: ca
      }
    };

request.get(options, function (err, resp) {
  console.log(resp.body);
});
