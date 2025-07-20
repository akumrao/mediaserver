
const fs = require('fs');
const express = require('express');
const WebSocket = require('ws');
const logging = require('../modules/logging.js');
logging.RegisterConsoleLogger();

//const WebSocketServer = WebSocket.Server;

// ----------------------------------------------------------------------------------------

const app = express();
app.use(express.static('client-datachannel')); // Use datachannel example for now

//const httpsServer = https.createServer(serverConfig, app);
//httpsServer.listen(HTTPS_PORT, '0.0.0.0');

// ----------------------------------------------------------------------------------------





const defaultConfig = {
  UseFrontend: false,
  UseMatchmaker: false,
  UseHTTPS: false,
  UseAuthentication: false,
  LogToFile: true,
  HomepageFile: 'player.htm',
  AdditionalRoutes: new Map(),
  EnableWebserver: true
};

const argv = require('yargs').argv;
var configFile = (typeof argv.configFile != 'undefined') ? argv.configFile.toString() : './config.json';
console.log(`configFile ${configFile}`);
const config = require('../modules/config.js').init(configFile, defaultConfig);

console.log("Config: " + JSON.stringify(config, null, '\t'));


if (config.UseFrontend) {
  var httpPort = 3000;
  var httpsPort = 8000;

  //Required for self signed certs otherwise just get an error back when sending request to frontend see https://stackoverflow.com/a/35633993
  process.env.NODE_TLS_REJECT_UNAUTHORIZED = "0"

  const httpsClient = require('./modules/httpsClient.js');
  var webRequest = new httpsClient();
} else {
  var httpPort = 8000;
  var httpsPort = 1111;
}



var http = require('http').Server(app);

//var sessionMonitor = require('./modules/sessionMonitor.js');
//sessionMonitor.init();

if (config.UseHTTPS) {
  //HTTPS certificate details
  const options = {

    key: fs.readFileSync('key.pem'),
    cert: fs.readFileSync('cert.pem')

  };

  var https = require('https').Server(options, app);
}

//If not using authetication then just move on to the next function/middleware
var isAuthenticated = redirectUrl => function (req, res, next) { return next(); }



const helmet = require('helmet');
var hsts = require('hsts');
var net = require('net');



if (config.UseHTTPS) {
  app.use(helmet());

  app.use(hsts({
    maxAge: 15552000  // 180 days in seconds
  }));

  //Setup http -> https redirect
  console.log('Redirecting http->https');
  app.use(function (req, res, next) {
    if (!req.secure) {
      if (req.get('Host')) {
        var hostAddressParts = req.get('Host').split(':');
        var hostAddress = hostAddressParts[0];
        if (httpsPort != 443) {
          hostAddress = `${hostAddress}:${httpsPort}`;
        }
        return res.redirect(['https://', hostAddress, req.originalUrl].join(''));
      } else {
        console.error(`unable to get host name from header. Requestor ${req.ip}, url path: '${req.originalUrl}', available headers ${JSON.stringify(req.headers)}`);
        return res.status(400).send('Bad Request');
      }
    }
    next();
  });
}



//Setup http and https servers
http.listen(httpPort, function () {
  console.logColor(logging.Green, 'Http listening on *: ' + httpPort);
});

if (config.UseHTTPS) {
  https.listen(httpsPort, function () {
    console.logColor(logging.Green, 'Https listening on *: ' + httpsPort);
  });
}





// Create a server for handling websocket calls
const wss = new WebSocket.Server({ server: config.UseHTTPS ? https : http});


wss.on('connection', function connection(ws) {
  ws.on('message', function incoming(message) {
       //console.log('received: %s',message.length );
    wss.clients.forEach(function each(client) {
      if (client !== ws && client.readyState === WebSocket.OPEN) {
        client.send(message);
      }
    });

  

  });


ws.on('error', function incoming(err) {
       console.log('error: %s', err );

  });



});





