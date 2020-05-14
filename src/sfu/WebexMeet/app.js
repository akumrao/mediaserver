const express = require("express");
const mongoose = require("mongoose");
const bodyParser = require("body-parser");
const cookieParser = require("cookie-parser");
const session = require("express-session");
const mongoStore = require("connect-mongo")(session);
const methodOverride = require("method-override");
const path = require("path");
const fs = require("fs");
const config = require('./config');
const logger = require("morgan");


//const http = require("http").Server(app);
var https = require('https');



//////////////////////////////////////////////////////////////
// Global variables
let webServer;
let socketServer;
let expressApp;
let io;

(async () => {
  try {
    await runExpressApp();
    await runWebServer();
    await runSocketServer();
  } catch (err) {
    console.error(err);
  }
})();



async function runExpressApp() {

//db connection
const dbPath = "mongodb://localhost/socketChatDB";
mongoose.connect(dbPath, { useNewUrlParser: true });
mongoose.connection.once("open", function() {
  console.log("Database Connection Established Successfully.");
});


expressApp = express();


expressApp.use(logger("dev"));

//http method override middleware
expressApp.use(
  methodOverride(function(req, res) {
    if (req.body && typeof req.body === "object" && "_method" in req.body) {
      var method = req.body._method;
      delete req.body._method;
      return method;
    }
  })
);

//session setup
const sessionInit = session({
  name: "userCookie",
  secret: "Mob-918105534224",
  resave: true,
  httpOnly: false,
  saveUninitialized: true,
  store: new mongoStore({ mongooseConnection: mongoose.connection }),
  cookie: { maxAge: 80 * 80 * 800 }
});

expressApp.use(sessionInit);

//public folder as static
expressApp.use(express.static(path.resolve(__dirname, "./public")));

//views folder and setting ejs engine
expressApp.set("views", path.resolve(__dirname, "./app/views"));
expressApp.set("view engine", "ejs");

//parsing middlewares
expressApp.use(bodyParser.json({ limit: "10mb", extended: true }));
expressApp.use(bodyParser.urlencoded({ limit: "10mb", extended: true }));
expressApp.use(cookieParser());

//including models files.
fs.readdirSync("./app/models").forEach(function(file) {
  if (file.indexOf(".js")) {
    require("./app/models/" + file);
  }
});

//including controllers files.
fs.readdirSync("./app/controllers").forEach(function(file) {
  if (file.indexOf(".js")) {
    var route = require("./app/controllers/" + file);
    //calling controllers function and passing app instance.
    route.controller(expressApp);
  }
});

//handling 404 error.
expressApp.use(function(req, res) {
  res.status(404).render("message", {
    title: "404",
    msg: "Page Not Found.",
    status: 404,
    error: "",
    user: req.session.user,
    chat: req.session.chat
  });
});

//expressApp level middleware for setting logged in user.

const userModel = mongoose.model("User");

expressApp.use(function(req, res, next) {
  if (req.session && req.session.user) {
    userModel.findOne({ email: req.session.user.email }, function(err, user) {
      if (user) {
        req.user = user;
        delete req.user.password;
        req.session.user = user;
        delete req.session.user.password;
        next();
      }
    });
  } else {
    next();
  }
}); //end of set Logged In User.



}




async function runWebServer() {

  console.error('runWebServer');

  const { sslKey, sslCrt } = config;
  if (!fs.existsSync(sslKey) || !fs.existsSync(sslCrt)) {
    console.error('SSL files are not found. check your config.js file');
    process.exit(0);
  }
  const tls = {
    cert: fs.readFileSync(sslCrt),
    key: fs.readFileSync(sslKey),
  };
  webServer = https.createServer(tls, expressApp);
  webServer.on('error', (err) => {
    console.error('starting web server failed:', err.message);
  });

  
  await new Promise((resolve) => {
    const { listenIp, listenPort } = config;
    webServer.listen(listenPort, listenIp, () => {
      console.log('server is running');
      console.log(`open https://127.0.0.1:${listenPort} in your web browser`);

    //  listenIps = config.webRtcTransport.listenIps;
      //const ip = listenIps.announcedIp || listenIps.ip;
     // console.log('listen ips ' + JSON.stringify(listenIps, null, 4) );

      resolve();
    });
  });
}

async function runSocketServer() {

    console.error('runSocketServer');


    require("./libs/chat.js").sockets(webServer);


}


  // socketServer = socketIO(webServer, {
  //   serveClient: false,
  //   path: '/server',
  //   log: false,
  // });
            





///////////////////////////////////////////////////////////////












//port setup
const port = process.env.PORT || 5000;

//socket.io





// http.listen(port, function() {
//   console.log("Chat App started at port :" + port);
// });
