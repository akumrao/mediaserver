# Realtime communication with WebRTC

[Realtime communication with WebRTC](https://codelabs.developers.google.com/codelabs/webrtc-web/#0).

## What it uses
* Get video from your webcam
* Stream video with RTCPeerConnection
* Stream data with RTCDataChannel
* Set up a signaling service to exchange messages
* Combine peer connection and signaling

## What it needs
* Chrome 90 or above. Or Latest Edge browser on Windows
* Web Server for Chrome, or use your own web server of choice.
* The sample code.
* A text editor.
* Basic knowledge of HTML, CSS and JavaScript, Node.JS.

# Components provided by webrtc (in NVR/3rdParty/rtc/web)
* adapter-latest.js
* sdp.js
* package.json

# Components developed by  (NVR/xStore/rtc/web)
* index.js (server side code)
* index.html
* app.js
* main.js
* config.js for ssl certificates

# Step To test
`cd NVR/xStore/rtc/web`

Run `npm install` before running the code.

To Run: `node index.js`

To view with Chrome or Firefox browser, view: https://localhost:9093/
