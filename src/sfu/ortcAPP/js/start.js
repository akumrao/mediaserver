



class Device
{
	/**
	 * Create a new Device to connect to mediaserver server.
	 *
	 * @param {Class} [Handler] - An optional RTC handler class for unsupported or
	 *   custom devices. Don't set it when in a browser.
	 *
	 * @throws {UnsupportedError} if device is not supported.
	 */
	constructor({ Handler } = {})
	{
		// RTC handler class.
		this._Handler = Handler || detectDevice();

		if (!this._Handler)
			throw new UnsupportedError('device not supported');

		console.log('constructor() [Handler:%s]', this._Handler.name);

		// Loaded flag.
		// @type {Boolean}
		this._loaded = false;

		// Extended RTP capabilities.
		// @type {Object}
		this._extendedRtpCapabilities = null;

		// Local RTP capabilities for receiving media.
		// @type {RTCRtpCapabilities}
		this._recvRtpCapabilities = null;

		// Whether we can produce audio/video based on computed extended RTP
		// capabilities.
		// @type {Object}
		this._canProduceByKind =
		{
			audio : false,
			video : false
		};
	}

	/**
	 * The RTC handler class name ('Chrome70', 'Firefox65', etc).
	 *
	 * @returns {String}
	 */
	get handlerName()
	{
		return this._Handler.name;
	}

	/**
	 * Whether the Device is loaded.
	 *
	 * @returns {Boolean}
	 */
	get loaded()
	{
		return this._loaded;
	}

	/**
	 * RTP capabilities of the Device for receiving media.
	 *
	 * @returns {RTCRtpCapabilities}
	 * @throws {InvalidStateError} if not loaded.
	 */
	get rtpCapabilities()
	{
		if (!this._loaded)
			throw new InvalidStateError('not loaded');

		return this._recvRtpCapabilities;
	}

	/**
	 * Initialize the Device.
	 *
	 * @param {RTCRtpCapabilities} routerRtpCapabilities - Router RTP capabilities.
	 *
	 * @async
	 * @throws {TypeError} if missing/wrong arguments.
	 * @throws {InvalidStateError} if already loaded.
	 */
	async load({ routerRtpCapabilities } = {})
	{
		console.log('load() [routerRtpCapabilities:%o]', routerRtpCapabilities);

		if (this._loaded)
			throw new InvalidStateError('already loaded');
		else if (typeof routerRtpCapabilities !== 'object')
			throw new TypeError('missing routerRtpCapabilities');

		const nativeRtpCapabilities =  await this._Handler.getNativeRtpCapabilities();

		console.log(
			'load() | got native RTP capabilities:%o', nativeRtpCapabilities);

		// Get extended RTP capabilities.
		this._extendedRtpCapabilities = getExtendedRtpCapabilities(
			nativeRtpCapabilities, routerRtpCapabilities);

		console.log(
			'load() | got extended RTP capabilities:%o', this._extendedRtpCapabilities);

		// Check whether we can produce audio/video.
		this._canProduceByKind.audio =
			canSend('audio', this._extendedRtpCapabilities);
		this._canProduceByKind.video =
			canSend('video', this._extendedRtpCapabilities);

		// Generate our receiving RTP capabilities for receiving media.
		this._recvRtpCapabilities =
			getRecvRtpCapabilities(this._extendedRtpCapabilities);

		console.log(
			'load() | got receiving RTP capabilities:%o', this._recvRtpCapabilities);

		console.log('load() succeeded');

		this._loaded = true;
	}

	/**
	 * Whether we can produce audio/video.
	 *
	 * @param {String} kind - 'audio' or 'video'.
	 *
	 * @returns {Boolean}
	 * @throws {InvalidStateError} if not loaded.
	 * @throws {TypeError} if wrong arguments.
	 */
	canProduce(kind)
	{
		if (!this._loaded)
			throw new InvalidStateError('not loaded');
		else if (kind !== 'audio' && kind !== 'video')
			throw new TypeError(`invalid kind "${kind}"`);

		return this._canProduceByKind[kind];
	}

	/**
	 * Creates a Transport for sending media.
	 *
	 * @param {String} - Server-side Transport id.
	 * @param {RTCIceParameters} iceParameters - Server-side Transport ICE parameters.
	 * @param {Array<RTCIceCandidate>} [iceCandidates] - Server-side Transport ICE candidates.
	 * @param {RTCDtlsParameters} dtlsParameters - Server-side Transport DTLS parameters.
	 * @param {Array<RTCIceServer>} [iceServers] - Array of ICE servers.
	 * @param {RTCIceTransportPolicy} [iceTransportPolicy] - ICE transport
	 *   policy.
	 * @param {Object} [proprietaryConstraints] - RTCPeerConnection proprietary constraints.
	 * @param {Object} [appData={}] - Custom app data.
	 *
	 * @returns {Transport}
	 * @throws {InvalidStateError} if not loaded.
	 * @throws {TypeError} if wrong arguments.
	 */
	createSendTransport(
		{
			id,
			iceParameters,
			iceCandidates,
			dtlsParameters,
			iceServers,
			iceTransportPolicy,
			proprietaryConstraints,
			appData = {}
		} = {}
	)
	{
		console.log('createSendTransport()');

		return this._createTransport(
			{
				direction : 'send',
				id,
				iceParameters,
				iceCandidates,
				dtlsParameters,
				iceServers,
				iceTransportPolicy,
				proprietaryConstraints,
				appData
			});
	}

	/**
	 * Creates a Transport for receiving media.
	 *
	 * @param {String} - Server-side Transport id.
	 * @param {RTCIceParameters} iceParameters - Server-side Transport ICE parameters.
	 * @param {Array<RTCIceCandidate>} [iceCandidates] - Server-side Transport ICE candidates.
	 * @param {RTCDtlsParameters} dtlsParameters - Server-side Transport DTLS parameters.
	 * @param {Array<RTCIceServer>} [iceServers] - Array of ICE servers.
	 * @param {RTCIceTransportPolicy} [iceTransportPolicy] - ICE transport
	 *   policy.
	 * @param {Object} [proprietaryConstraints] - RTCPeerConnection proprietary constraints.
	 * @param {Object} [appData={}] - Custom app data.
	 *
	 * @returns {Transport}
	 * @throws {InvalidStateError} if not loaded.
	 * @throws {TypeError} if wrong arguments.
	 */
	createRecvTransport(
		{
			id,
			iceParameters,
			iceCandidates,
			dtlsParameters,
			iceServers,
			iceTransportPolicy,
			proprietaryConstraints,
			appData = {}
		} = {}
	)
	{
		console.log('createRecvTransport()');

		return this._createTransport(
			{
				direction : 'recv',
				id,
				iceParameters,
				iceCandidates,
				dtlsParameters,
				iceServers,
				iceTransportPolicy,
				proprietaryConstraints,
				appData
			});
	}

	/**
	 * @private
	 */
	_createTransport(
		{
			direction,
			id,
			iceParameters,
			iceCandidates,
			dtlsParameters,
			iceServers,
			iceTransportPolicy,
			proprietaryConstraints,
			appData = {}
		}
	)
	{
		console.log('createTransport()');

		if (!this._loaded)
			throw new InvalidStateError('not loaded');
		else if (typeof id !== 'string')
			throw new TypeError('missing id');
		else if (typeof iceParameters !== 'object')
			throw new TypeError('missing iceParameters');
		else if (!Array.isArray(iceCandidates))
			throw new TypeError('missing iceCandidates');
		else if (typeof dtlsParameters !== 'object')
			throw new TypeError('missing dtlsParameters');
		else if (appData && typeof appData !== 'object')
			throw new TypeError('if given, appData must be an object');

		// Create a new Transport.
		const transport = new Transport(
			{
				direction,
				id,
				iceParameters,
				iceCandidates,
				dtlsParameters,
				iceServers,
				iceTransportPolicy,
				proprietaryConstraints,
				appData,
				Handler                 : this._Handler,
				extendedRtpCapabilities : this._extendedRtpCapabilities,
				canProduceByKind        : this._canProduceByKind
			});

		return transport;
	}
}


function get_browser() {
    var ua=navigator.userAgent,tem,M=ua.match(/(opera|chrome|safari|firefox|msie|trident(?=\/))\/?\s*(\d+)/i) || []; 
    if(/trident/i.test(M[1])){
        tem=/\brv[ :]+(\d+)/g.exec(ua) || []; 
        return {name:'IE',version:(tem[1]||'')};
        }   
    if(M[1]==='Chrome'){
        tem=ua.match(/\bOPR|Edge\/(\d+)/)
        if(tem!=null)   {return {name:'Opera', version:tem[1]};}
        }   
    M=M[2]? [M[1], M[2]]: [navigator.appName, navigator.appVersion, '-?'];
    if((tem=ua.match(/version\/(\d+)/i))!=null) {M.splice(1,1,tem[1]);}
    return {
      name: M[0],
      version: M[1]
    };
 }

function detectDevice() {

if (typeof navigator === 'object' && navigator.product === 'ReactNative')
	{
		if (typeof RTCPeerConnection !== 'undefined')
		{
			return ReactNative;
		}
		else
		{
			console.log('unsupported ReactNative without RTCPeerConnection');

			return null;
		}
	}
	// browser.
	else if (typeof navigator === 'object' && typeof navigator.userAgent === 'string')
	{
		const ua = navigator.userAgent;
		//const browser = bowser.getParser(ua);
		//const engine = browser.getEngine();
		var browser=get_browser();

		
		if(browser.name == 'Chrome')
				return Chrome75;
		// browser.version = '40'


		



		// Chrome and Chromium.
		// if (browser.satisfies({ chrome: '>=75', chromium: '>=75' }))
		// {
		// 	return Chrome75;
		// }
		// else if (browser.satisfies({ chrome: '>=70', chromium: '>=70' }))
		// {
		// 	return Chrome70;
		// }
		// else if (browser.satisfies({ chrome: '>=67', chromium: '>=67' }))
		// {
		// 	return Chrome67;
		// }
		// else if (browser.satisfies({ chrome: '>=55', chromium: '>=55' }))
		// {
		// 	return Chrome55;
		// }
		// // Opera.
		// else if (browser.satisfies({ opera: '>=57' }))
		// {
		// 	return Chrome70;
		// }
		// else if (browser.satisfies({ opera: '>=44' }))
		// {
		// 	return Chrome55;
		// }
		// // Edge (Chromium based).
		// else if (browser.satisfies({ 'microsoft edge': '>=75' }))
		// {
		// 	return Chrome75;
		// }
		// else if (browser.satisfies({ 'microsoft edge': '>=74' }))
		// {
		// 	return Chrome70;
		// }
		// // Old Edge with ORTC support.
		// else if (browser.satisfies({ 'microsoft edge': '>=11' }))
		// {
		// 	return Edge11;
		// }
		// // Firefox.
		// else if (browser.satisfies({ firefox: '>=60' }))
		// {
		// 	return Firefox60;
		// }
		// // Safari with Unified-Plan support.
		// else if (
		// 	browser.satisfies({ safari: '>=12.1' }) &&
		// 	typeof RTCRtpTransceiver !== 'undefined' &&
		// 	RTCRtpTransceiver.prototype.hasOwnProperty('currentDirection')
		// )
		// {
		// 	return Safari12;
		// }
		// // Safari with Plab-B support.
		// else if (browser.satisfies({ safari: '>=11' }))
		// {
		// 	return Safari11;
		// }
		// // Best effort for Chromium based browsers.
		// else if (engine.name.toLowerCase() === 'blink')
		// {
		// 	console.log('best effort Chromium based browser detection');

		// 	const match = ua.match(/(?:(?:Chrome|Chromium))[ /](\w+)/i);

		// 	if (match)
		// 	{
		// 		const version = Number(match[1]);

		// 		if (version >= 75)
		// 			return Chrome75;
		// 		else if (version >= 70)
		// 			return Chrome70;
		// 		else if (version >= 67)
		// 			return Chrome67;
		// 		else
		// 			return Chrome55;
		// 	}
		// 	else
		// 	{
		// 		return Chrome75;
		// 	}
		// Unsupported browser.
		
	}
	// Unknown device.
	else
	{
		console.log('unknown device');

		return null;
	}
}


//import * as sdpTransform from "parser.js";

//import * as sdpTransform1 from "js/parser.js";

class Chrome75
{
	static async getNativeRtpCapabilities()
	{
		console.log('getNativeRtpCapabilities()');

		const pc = new RTCPeerConnection(
			{
				iceServers         : [],
				iceTransportPolicy : 'all',
				bundlePolicy       : 'max-bundle',
				rtcpMuxPolicy      : 'require',
				sdpSemantics       : 'unified-plan'
			});

		try
		{
			pc.addTransceiver('audio');
			pc.addTransceiver('video');

			const offer =  await pc.createOffer();

			try { pc.close(); }
			catch (error) {}

			//const sdpTransform = require('./parser');

			
			const sdpObject = parse(offer.sdp);

			//const sdpObject1 = sdpTransform1.parse(offer.sdp);
			const nativeRtpCapabilities =
				extractRtpCapabilities({ sdpObject });

			return nativeRtpCapabilities;
		}
		catch (error)
		{
			try { pc.close(); }
			catch (error2) {}

			throw error;
		}
	}

	constructor(
		{
			direction,
			iceParameters,
			iceCandidates,
			dtlsParameters,
			iceServers,
			iceTransportPolicy,
			proprietaryConstraints,
			extendedRtpCapabilities
		}
	)
	{
		console.log('constructor() [direction:%s]', direction);

		switch (direction)
		{
			case 'send':
			{
				const sendingRtpParametersByKind =
				{
					audio : getSendingRtpParameters('audio', extendedRtpCapabilities),
					video : getSendingRtpParameters('video', extendedRtpCapabilities)
				};

				const sendingRemoteRtpParametersByKind =
				{
					audio : getSendingRemoteRtpParameters('audio', extendedRtpCapabilities),
					video : getSendingRemoteRtpParameters('video', extendedRtpCapabilities)
				};

				return new SendHandler(
					{
						iceParameters,
						iceCandidates,
						dtlsParameters,
						iceServers,
						iceTransportPolicy,
						proprietaryConstraints,
						sendingRtpParametersByKind,
						sendingRemoteRtpParametersByKind
					});
			}

			case 'recv':
			{
				return new RecvHandler(
					{
						iceParameters,
						iceCandidates,
						dtlsParameters,
						iceServers,
						iceTransportPolicy,
						proprietaryConstraints
					});
			}
		}
	}
}





device = new Device();

 async function loadDevice(routerRtpCapabilities) {

  	console.log("loadDevice");
  // try {
  
  //  // device = new Device();

  // } catch (error) {
  // 	console.error(error);
  //   if (error.name === 'UnsupportedError') {
  //     console.error('browser not supported');
  //   }
  // }
  await device.load({ routerRtpCapabilities });
}


async function getUserMedia1(transport, isWebcam) {
  if (!device.canProduce('video')) {
    console.error('cannot produce video');
    return;
  }

  let stream;
  try {

  stream =  await navigator.mediaDevices.getUserMedia({ video: true });

    // stream = isWebcam ?
    //   await navigator.mediaDevices.getUserMedia({ video: true }) :
    //   await navigator.mediaDevices.getDisplayMedia({ video: true });
  } catch (err) {
    console.error('getUserMedia() failed:', err.message);
    throw err;
  }
  return stream;
}

/////////////////////////////////////////////////////////////////////////////

async function subscribe() {

  var parser = new URL(window.location.href); 
  var istcp = parser.searchParams;
  const tcpValue = istcp.get('forceTcp') ? true : false;


  socket.emit('createConsumerTransport', {  forceTcp: tcpValue }, async function (data) {

  console.log(data);	

  const transport = device.createRecvTransport(data);

  //////////////////
  transport.on('connect', async ({ dtlsParameters }, callback, errback) => {
    socket.emit('connectConsumerTransport', { transportId: transport.id, dtlsParameters }, async function () {
    	 console.log("connectConsumerTransport");
    	  callback();
    });
    

  });
  //////////////////
  transport.on('connectionstatechange', (state) => {
    switch (state) {
      case 'connecting':
        // $txtSubscription.innerHTML = 'subscribing...';
        // $fsSubscribe.disabled = true;
        console.log('consume subscribing...');
        break;

      case 'connected':
       // document.querySelector('#remote_video').srcObject = stream;
        // $txtSubscription.innerHTML = 'subscribed';
        // $fsSubscribe.disabled = true;
        console.log('consume subscribed.');
        break;

      case 'failed':
        transport.close();
        // $txtSubscription.innerHTML = 'failed';
        // $fsSubscribe.disabled = false;
        console.log('consumer failed');
        break;

      default: break;
    }
  });

  const stream = await consume(transport);

  /////////////////

});//end createconsumer
 
}//end subscribe 

async function consume(transport) {
  const { rtpCapabilities } = device;

   socket.emit('consume', {rtpCapabilities }, async function (data) {
    	 console.log("consume");
    	  const {    producerId,   id,   kind,   rtpParameters,  } = data;

    	    let codecOptions = {};
			  const consumer = await transport.consume({
			    id,
			    producerId,
			    kind,
			    rtpParameters,
			    codecOptions,
			  });
			  const stream = new MediaStream();
			  stream.addTrack(consumer.track);

			  socket.emit('resume');

			  document.querySelector('#remote_video').srcObject = stream;

			  return stream;
    });

}
    

///////////////////////////////////////////////////////////////////////////


async function publish() 
{
  // const isWebcam = (e.target.id === 'btn_webcam');
  // $txtPublish = isWebcam ? $txtWebcam : $txtScreen;

  var parser = new URL(window.location.href); 
  var istcp = parser.searchParams;
  const tcpValue = istcp.get('forceTcp') ? true : false;


  socket.emit('createProducerTransport', {  forceTcp: tcpValue,  rtpCapabilities: device.rtpCapabilities,
  }, async function (data) {

     console.log(data);

    const transport = device.createSendTransport(data);

/////////////////////////////////////////////////////////////

  transport.on('connect', async ({ dtlsParameters }, callback, errback) => {
    socket.emit('connectProducerTransport', { dtlsParameters }, async function () {
    	 console.log("connectProducerTransport");
    	  callback();
    });
    

  });

  transport.on('produce', async ({ kind, rtpParameters }, callback, errback) => {
    

     socket.emit('produce', {
        transportId: transport.id,
        kind,
        rtpParameters,
      }, function (id) {
    	 console.log("produceid " + id);
    	  callback({ id });
    });
    

  });

  transport.on('connectionstatechange', (state) => {
    switch (state) {
      case 'connecting':
       // $txtPublish.innerHTML = 'publishing...';
	   //$fsPublish.disabled = true;
       // $fsSubscribe.disabled = true;
        console.log( 'publishing...');
      break;

      case 'connected':
        document.querySelector('#local_video').srcObject = stream;
        // $txtPublish.innerHTML = 'published';
        // $fsPublish.disabled = true;
        // $fsSubscribe.disabled = false;
         console.log( 'published...');
      break;

      case 'failed':
        transport.close();
        // $txtPublish.innerHTML = 'failed';
        // $fsPublish.disabled = false;
        // $fsSubscribe.disabled = true;
        console.log( 'failed...');
      break;
      break;

      default: break;
    }
  });

  let stream;
  try {
    stream =  await getUserMedia1(transport, true);
    const track = stream.getVideoTracks()[0];
    const params = { track };
    // if ($chkSimulcast.checked) {
    //   params.encodings = [
    //     { maxBitrate: 100000 },
    //     { maxBitrate: 300000 },
    //     { maxBitrate: 900000 },
    //   ];
    //   params.codecOptions = {
    //     videoGoogleStartBitrate : 1000
    //   };
    // }
    producer = await transport.produce(params);

    var doit = 9;
  } catch (err) {

  	console.log( err);
    //$txtPublish.innerHTML = 'failed';
  }







    //////////////////////////////////////////////////////////////

   }

    ) ;

}
