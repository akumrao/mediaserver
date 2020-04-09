class utils{

static clone (obj)
{
    if (typeof obj !== 'object')
        return {};

    return JSON.parse(JSON.stringify(obj));
}

/**
 * Generates a random positive integer.
 *
 * @returns {Number}
 */
static generateRandomNumber()
{
    return Math.round(Math.random() * 10000000);
}

}

class Handler extends EnhancedEventEmitter
{
    constructor(
        {
            iceParameters,
            iceCandidates,
            dtlsParameters,
            iceServers,
            iceTransportPolicy,
            proprietaryConstraints
        }
    )
    {
        super(logger);

        // Got transport local and remote parameters.
        // @type {Boolean}
        this._transportReady = false;

        // Remote SDP handler.
        // @type {RemoteSdp}
        this._remoteSdp = new RemoteSdp(
            {
                iceParameters,
                iceCandidates,
                dtlsParameters
            });

        // RTCPeerConnection instance.
        // @type {RTCPeerConnection}
        this._pc = new RTCPeerConnection(
            {
                iceServers         : iceServers || [],
                iceTransportPolicy : iceTransportPolicy || 'all',
                bundlePolicy       : 'max-bundle',
                rtcpMuxPolicy      : 'require',
                sdpSemantics       : 'unified-plan'
            },
            proprietaryConstraints);

        // Map of RTCTransceivers indexed by MID.
        // @type {Map<String, RTCTransceiver>}
        this._mapMidTransceiver = new Map();

        // Handle RTCPeerConnection connection status.
        this._pc.addEventListener('iceconnectionstatechange', () =>
        {
            switch (this._pc.iceConnectionState)
            {
                case 'checking':
                    this.emit('@connectionstatechange', 'connecting');
                    break;
                case 'connected':
                case 'completed':
                    this.emit('@connectionstatechange', 'connected');
                    break;
                case 'failed':
                    this.emit('@connectionstatechange', 'failed');
                    break;
                case 'disconnected':
                    this.emit('@connectionstatechange', 'disconnected');
                    break;
                case 'closed':
                    this.emit('@connectionstatechange', 'closed');
                    break;
            }
        });
    }

    close()
    {
        logger.debug('close()');

        // Close RTCPeerConnection.
        try { this._pc.close(); }
        catch (error) {}
    }

    async getTransportStats()
    {
        return this._pc.getStats();
    }

    async updateIceServers({ iceServers })
    {
        logger.debug('updateIceServers()');

        const configuration = this._pc.getConfiguration();

        configuration.iceServers = iceServers;

        this._pc.setConfiguration(configuration);
    }

    async _setupTransport({ localDtlsRole, localSdpObject = null })
    {
        if (!localSdpObject)
            localSdpObject = parse(this._pc.localDescription.sdp);

        // Get our local DTLS parameters.
        const dtlsParameters =
            extractDtlsParameters({ sdpObject: localSdpObject });

        // Set our DTLS role.
        dtlsParameters.role = localDtlsRole;

        // Update the remote DTLS role in the SDP.
        this._remoteSdp.updateDtlsRole(
            localDtlsRole === 'client' ? 'server' : 'client');

        // Need to tell the remote transport about our parameters.
        await this.safeEmitAsPromise('@connect', { dtlsParameters });

        this._transportReady = true;
    }
}

class SendHandler extends Handler
{
    constructor(data)
    {
        super(data);

        // Generic sending RTP parameters for audio and video.
        // @type {RTCRtpParameters}
        this._sendingRtpParametersByKind = data.sendingRtpParametersByKind;

        // Generic sending RTP parameters for audio and video suitable for the SDP
        // remote answer.
        // @type {RTCRtpParameters}
        this._sendingRemoteRtpParametersByKind = data.sendingRemoteRtpParametersByKind;

        // Local stream.
        // @type {MediaStream}
        this._stream = new MediaStream();
    }

    async send({ track, encodings, codecOptions })
    {
        logger.debug('send() [kind:%s, track.id:%s]', track.kind, track.id);

        if (encodings && encodings.length > 1)
        {
            encodings.forEach((encoding, idx) =>
            {
                encoding.rid = `r${idx}`;
            });
        }

        const transceiver = this._pc.addTransceiver(
            track,
            {
                direction     : 'sendonly',
                streams       : [ this._stream ],
                sendEncodings : encodings
            });
        const offer = await this._pc.createOffer();
        let localSdpObject = parse(offer.sdp);
        const sendingRtpParameters =
            utils.clone(this._sendingRtpParametersByKind[track.kind]);

        if (!this._transportReady)
            await this._setupTransport({ localDtlsRole: 'server', localSdpObject });

   logger.debug(
      'send() | calling pc.setLocalDescription() [offer:%o]', offer);

        logger.debug(
            'send() | localSdpObject [offer:%o]', localSdpObject);

        await this._pc.setLocalDescription(offer);

        // We can now get the transceiver.mid.
        const localId = transceiver.mid;

        // Set MID.
        sendingRtpParameters.mid = localId;

        localSdpObject = parse(this._pc.localDescription.sdp);

    logger.debug(
      'send() | localSdpObject after parse [offer:%o]', localSdpObject);

        const offerMediaObject = localSdpObject.media[localSdpObject.media.length - 1];

        logger.debug(
      'send() | offerMediaObject [offer:%o]', offerMediaObject);
        console.log( JSON.stringify(offerMediaObject.candidates));

        // Set RTCP CNAME.
        sendingRtpParameters.rtcp.cname =
            getCname({ offerMediaObject });

        // Set RTP encodings by parsing the SDP offer if no encodings are given.
        if (!encodings)
        {
            sendingRtpParameters.encodings =
                sdpUnifiedPlanUtils.getRtpEncodings({ offerMediaObject });
        }
        // Set RTP encodings by parsing the SDP offer and complete them with given
        // one if just a single encoding has been given.
        else if (encodings.length === 1)
        {
            const newEncodings =
                sdpUnifiedPlanUtils.getRtpEncodings({ offerMediaObject });

            Object.assign(newEncodings[0], encodings[0]);

            sendingRtpParameters.encodings = newEncodings;
        }
        // Otherwise if more than 1 encoding are given use them verbatim.
        else
        {
            sendingRtpParameters.encodings = encodings;
        }

        // If VP8 and there is effective simulcast, add scalabilityMode to each
        // encoding.
        if (
            sendingRtpParameters.encodings.length > 1 &&
            sendingRtpParameters.codecs[0].mimeType.toLowerCase() === 'video/vp8'
        )
        {
            for (const encoding of sendingRtpParameters.encodings)
            {
                encoding.scalabilityMode = 'L1T3';
            }
        }

        this._remoteSdp.send(
            {
                offerMediaObject,
                offerRtpParameters  : sendingRtpParameters,
                answerRtpParameters : this._sendingRemoteRtpParametersByKind[track.kind],
                codecOptions
            });




    logger.debug(
      'send() | sendingRtpParameters [offer:%o]', sendingRtpParameters);

     logger.debug(
      'send() | answerRtpParameters [offer:%o]', this._sendingRemoteRtpParametersByKind[track.kind]);

        const answer = { type: 'answer', sdp: this._remoteSdp.getSdp() };

        logger.debug(
            'send() | calling pc.setRemoteDescription() [answer:%o]', answer);

        await this._pc.setRemoteDescription(answer);

        // Store in the map.
        this._mapMidTransceiver.set(localId, transceiver);

        return { localId, rtpParameters: sendingRtpParameters };
    }

    async stopSending({ localId })
    {
        logger.debug('stopSending() [localId:%s]', localId);

        const transceiver = this._mapMidTransceiver.get(localId);

        if (!transceiver)
            throw new Error('associated RTCRtpTransceiver not found');

        transceiver.sender.replaceTrack(null);
        this._pc.removeTrack(transceiver.sender);
        this._remoteSdp.disableMediaSection(transceiver.mid);

        const offer = await this._pc.createOffer();

        logger.debug(
            'stopSending() | calling pc.setLocalDescription() [offer:%o]', offer);

        await this._pc.setLocalDescription(offer);

        const answer = { type: 'answer', sdp: this._remoteSdp.getSdp() };

        logger.debug(
            'stopSending() | calling pc.setRemoteDescription() [answer:%o]', answer);

        await this._pc.setRemoteDescription(answer);
    }

    async replaceTrack({ localId, track })
    {
        logger.debug('replaceTrack() [localId:%s, track.id:%s]', localId, track.id);

        const transceiver = this._mapMidTransceiver.get(localId);

        if (!transceiver)
            throw new Error('associated RTCRtpTransceiver not found');

        await transceiver.sender.replaceTrack(track);
    }

    async setMaxSpatialLayer({ localId, spatialLayer })
    {
        logger.debug(
            'setMaxSpatialLayer() [localId:%s, spatialLayer:%s]',
            localId, spatialLayer);

        const transceiver = this._mapMidTransceiver.get(localId);

        if (!transceiver)
            throw new Error('associated RTCRtpTransceiver not found');

        const parameters = transceiver.sender.getParameters();

        parameters.encodings.forEach((encoding, idx) =>
        {
            if (idx <= spatialLayer)
                encoding.active = true;
            else
                encoding.active = false;
        });

        await transceiver.sender.setParameters(parameters);
    }

    async getSenderStats({ localId })
    {
        const transceiver = this._mapMidTransceiver.get(localId);

        if (!transceiver)
            throw new Error('associated RTCRtpTransceiver not found');

        return transceiver.sender.getStats();
    }

    async restartIce({ iceParameters })
    {
        logger.debug('restartIce()');

        // Provide the remote SDP handler with new remote ICE parameters.
        this._remoteSdp.updateIceParameters(iceParameters);

        if (!this._transportReady)
            return;

        const offer = await this._pc.createOffer({ iceRestart: true });

        logger.debug(
            'restartIce() | calling pc.setLocalDescription() [offer:%o]', offer);

        await this._pc.setLocalDescription(offer);

        const answer = { type: 'answer', sdp: this._remoteSdp.getSdp() };

        logger.debug(
            'restartIce() | calling pc.setRemoteDescription() [answer:%o]', answer);

        await this._pc.setRemoteDescription(answer);
    }
}

class RecvHandler extends Handler
{
    constructor(data)
    {
        super(data);

        // MID value counter. It must be converted to string and incremented for
        // each new m= section.
        // @type {Number}
        this._nextMid = 0;
    }

    async receive({ id, kind, rtpParameters })
    {
        logger.debug('receive() [id:%s, kind:%s]', id, kind);

        const localId = String(this._nextMid);

        this._remoteSdp.receive(
            {
                mid                : localId,
                kind,
                offerRtpParameters : rtpParameters,
                streamId           : rtpParameters.rtcp.cname,
                trackId            : id
            });

        const offer = { type: 'offer', sdp: this._remoteSdp.getSdp() };

        logger.debug(
            'receive() | calling pc.setRemoteDescription() [offer:%o]', offer);

        await this._pc.setRemoteDescription(offer);

        let answer = await this._pc.createAnswer();
        const localSdpObject = parse(answer.sdp);
        const answerMediaObject = localSdpObject.media
            .find((m) => String(m.mid) === localId);

        // May need to modify codec parameters in the answer based on codec
        // parameters in the offer.
        applyCodecParameters(
            {
                offerRtpParameters : rtpParameters,
                answerMediaObject
            });

        answer = { type: 'answer', sdp: write1(localSdpObject) };

        if (!this._transportReady)
            await this._setupTransport({ localDtlsRole: 'client', localSdpObject });

        logger.debug(
            'receive() | calling pc.setLocalDescription() [answer:%o]', answer);

        await this._pc.setLocalDescription(answer);

        const transceiver = this._pc.getTransceivers()
            .find((t) => t.mid === localId);

        if (!transceiver)
            throw new Error('new RTCRtpTransceiver not found');

        // Store in the map.
        this._mapMidTransceiver.set(localId, transceiver);

        // Increase next MID.
        this._nextMid++;

        return { localId, track: transceiver.receiver.track };
    }

    async stopReceiving({ localId })
    {
        logger.debug('stopReceiving() [localId:%s]', localId);

        const transceiver = this._mapMidTransceiver.get(localId);

        if (!transceiver)
            throw new Error('associated RTCRtpTransceiver not found');

        this._remoteSdp.disableMediaSection(transceiver.mid);

        const offer = { type: 'offer', sdp: this._remoteSdp.getSdp() };

        logger.debug(
            'stopReceiving() | calling pc.setRemoteDescription() [offer:%o]', offer);

        await this._pc.setRemoteDescription(offer);

        const answer = await this._pc.createAnswer();

        logger.debug(
            'stopReceiving() | calling pc.setLocalDescription() [answer:%o]', answer);

        await this._pc.setLocalDescription(answer);
    }

    async getReceiverStats({ localId })
    {
        const transceiver = this._mapMidTransceiver.get(localId);

        if (!transceiver)
            throw new Error('associated RTCRtpTransceiver not found');

        return transceiver.receiver.getStats();
    }

    async restartIce({ iceParameters })
    {
        logger.debug('restartIce()');

        // Provide the remote SDP handler with new remote ICE parameters.
        this._remoteSdp.updateIceParameters(iceParameters);

        if (!this._transportReady)
            return;

        const offer = { type: 'offer', sdp: this._remoteSdp.getSdp() };

        logger.debug(
            'restartIce() | calling pc.setRemoteDescription() [offer:%o]', offer);

        await this._pc.setRemoteDescription(offer);

        const answer = await this._pc.createAnswer();

        logger.debug(
            'restartIce() | calling pc.setLocalDescription() [answer:%o]', answer);

        await this._pc.setLocalDescription(answer);
    }
}
