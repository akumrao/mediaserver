// (c) 2023 Johnson Controls.  All Rights reserved.

(function(root, factory) {
    if (typeof define === 'function' && define.amd) {
        // AMD. Register as an anonymous module.
        define(["./adapter"], factory);
    } else if (typeof exports === 'object') {
        // Node. Does not work with strict CommonJS, but
        // only CommonJS-like environments that support module.exports,
        // like Node.
        module.exports = factory(require("./adapter"));
    } else {
        // Browser globals (root is window)
        root.webRtcPlayer = factory(root.adapter);
        SDPUtils = root.adapter.sdp;
    }
}(this, function(adapter) {

    function webRtcPlayer(pc) {
        var self = this;

        this.pcClient = pc;
        this.dcClient = null;
        this.tnClient = null;

        // Latency tester
        this.latencyTestTimings = {
            TestStartTimeMs: null,
            UEReceiptTimeMs: null,
            UEPreCaptureTimeMs: null,
            UEPostCaptureTimeMs: null,
            UEPreEncodeTimeMs: null,
            UEPostEncodeTimeMs: null,
            UETransmissionTimeMs: null,
            BrowserReceiptTimeMs: null,
            FrameDisplayDeltaTimeMs: null,
            Reset: function() {
                this.TestStartTimeMs = null;
                this.UEReceiptTimeMs = null;
                this.UEPreCaptureTimeMs = null;
                this.UEPostCaptureTimeMs = null;
                this.UEPreEncodeTimeMs = null;
                this.UEPostEncodeTimeMs = null;
                this.UETransmissionTimeMs = null;
                this.BrowserReceiptTimeMs = null;
                this.FrameDisplayDeltaTimeMs = null;
            },
            SetUETimings: function(UETimings) {
                this.UEReceiptTimeMs = UETimings.ReceiptTimeMs;
                this.UEPreCaptureTimeMs = UETimings.PreCaptureTimeMs;
                this.UEPostCaptureTimeMs = UETimings.PostCaptureTimeMs;
                this.UEPreEncodeTimeMs = UETimings.PreEncodeTimeMs;
                this.UEPostEncodeTimeMs = UETimings.PostEncodeTimeMs;
                this.UETransmissionTimeMs = UETimings.TransmissionTimeMs;
                this.BrowserReceiptTimeMs = Date.now();
                this.OnAllLatencyTimingsReady(this);
            },
            SetFrameDisplayDeltaTime: function(DeltaTimeMs) {
                if (this.FrameDisplayDeltaTimeMs == null) {
                    this.FrameDisplayDeltaTimeMs = Math.round(DeltaTimeMs);
                    this.OnAllLatencyTimingsReady(this);
                }
            },
            OnAllLatencyTimingsReady: function(Timings) {}
        }

        generateAggregatedStatsFunction = function() {
            if (!self.aggregatedStats) {
                self.aggregatedStats = new Map();
            }

            return function(stats) {
                // to print webtrc stats
                // console.log('Printing Stats');

                let newStatMap = new Map();

                stats.forEach(stat => {
                    let newStat = {};
                    let idd = stat.id;
                    if (stat.trackId) {
                        idd = stat.trackId;
                    }
                    if (newStatMap.has(idd)) {
                        newStat = newStatMap.get(idd);
                    }

                    let aggregatedStats = self.aggregatedStats.get(idd);

                    if (stat.type == 'inbound-rtp' &&
                        !stat.isRemote &&
                        (stat.mediaType == 'video' || stat.id.toLowerCase().includes('video'))) {

                        newStat.timestamp = stat.timestamp;
                        newStat.bytesReceived = stat.bytesReceived;
                        newStat.framesDecoded = stat.framesDecoded;
                        newStat.packetsLost = stat.packetsLost;
                        newStat.bytesReceivedStart = aggregatedStats && aggregatedStats.bytesReceivedStart ? aggregatedStats.bytesReceivedStart : stat.bytesReceived;
                        newStat.framesDecodedStart = aggregatedStats && aggregatedStats.framesDecodedStart ? aggregatedStats.framesDecodedStart : stat.framesDecoded;
                        newStat.timestampStart = aggregatedStats && aggregatedStats.timestampStart ? aggregatedStats.timestampStart : stat.timestamp;

                        if (aggregatedStats && aggregatedStats.timestamp) {
                            if (aggregatedStats.bytesReceived) {
                                // Bitrate = bits received since last time / number of ms since last time
                                // In kbits (where k=1000) since time is in ms and stat we want is in seconds (so a '* 1000' then a '/ 1000' would negate each other)
                                newStat.bitrate = 8 * (newStat.bytesReceived - aggregatedStats.bytesReceived) / (newStat.timestamp - aggregatedStats.timestamp);
                                newStat.bitrate = Math.floor(newStat.bitrate);
                                newStat.lowBitrate = aggregatedStats.lowBitrate && aggregatedStats.lowBitrate < newStat.bitrate ? aggregatedStats.lowBitrate : newStat.bitrate
                                newStat.highBitrate = aggregatedStats.highBitrate && aggregatedStats.highBitrate > newStat.bitrate ? aggregatedStats.highBitrate : newStat.bitrate
                            }

                            if (aggregatedStats.bytesReceivedStart) {
                                newStat.avgBitrate = 8 * (newStat.bytesReceived - aggregatedStats.bytesReceivedStart) / (newStat.timestamp - aggregatedStats.timestampStart);
                                newStat.avgBitrate = Math.floor(newStat.avgBitrate);
                            }

                            if (aggregatedStats.framesDecoded) {
                                // framerate = frames decoded since last time / number of seconds since last time
                                newStat.framerate = (newStat.framesDecoded - aggregatedStats.framesDecoded) / ((newStat.timestamp - aggregatedStats.timestamp) / 1000);
                                newStat.framerate = Math.floor(newStat.framerate);
                                newStat.lowFramerate = aggregatedStats.lowFramerate && aggregatedStats.lowFramerate < newStat.framerate ? aggregatedStats.lowFramerate : newStat.framerate
                                newStat.highFramerate = aggregatedStats.highFramerate && aggregatedStats.highFramerate > newStat.framerate ? aggregatedStats.highFramerate : newStat.framerate
                            }

                            if (aggregatedStats.framesDecodedStart) {
                                newStat.avgframerate = (newStat.framesDecoded - aggregatedStats.framesDecodedStart) / ((newStat.timestamp - aggregatedStats.timestampStart) / 1000);
                                newStat.avgframerate = Math.floor(newStat.avgframerate);
                            }
                        }

                        if (stat.trackIdentifier) {
                            newStat.trackIdentifier = stat.trackIdentifier;
                        }

                        if (stat.frameWidth) {
                            newStat.frameWidth = stat.frameWidth;
                            newStat.frameHeight = stat.frameHeight;
                        }

                        newStatMap.set(idd, newStat);
                    }

                    // Read video track stats
                    if (stat.type == 'track' && (stat.trackIdentifier == 'video_label' || stat.kind == 'video')) {
                        newStat.framesDropped = stat.framesDropped;
                        newStat.framesReceived = stat.framesReceived;
                        newStat.framesDroppedPercentage = stat.framesDropped / stat.framesReceived * 100;
                        newStat.frameHeight = stat.frameHeight;
                        newStat.frameWidth = stat.frameWidth;
                        newStat.frameHeightStart = aggregatedStats && aggregatedStats.frameHeightStart ? aggregatedStats.frameHeightStart : stat.frameHeight;
                        newStat.frameWidthStart = aggregatedStats && aggregatedStats.frameWidthStart ? aggregatedStats.frameWidthStart : stat.frameWidth;

                        if (stat.trackIdentifier) {
                            newStat.trackIdentifier = stat.trackIdentifier;
                        }

                        newStatMap.set(stat.id, newStat);
                    }
                });

                self.aggregatedStats = newStatMap;

                if (self.onAggregatedStats)
                    self.onAggregatedStats(newStatMap)
            }
        };

        this.startLatencyTest = function(onTestStarted) {

            self.latencyTestTimings.Reset();
            self.latencyTestTimings.TestStartTimeMs = Date.now();
            onTestStarted(self.latencyTestTimings.TestStartTimeMs);
        }

        this.getStats = function(onStats) {
            if (self.pcClient && onStats) {
                self.pcClient.getStats(null).then((stats) => {
                    onStats(stats);
                });
            }
        }

        this.aggregateStats = function(checkInterval) {
            let calcAggregatedStats = generateAggregatedStatsFunction();
            let printAggregatedStats = () => {
                self.getStats(calcAggregatedStats);
            }
            self.aggregateStatsIntervalId = setInterval(printAggregatedStats, checkInterval);
        }
    };

    return webRtcPlayer;
}));
