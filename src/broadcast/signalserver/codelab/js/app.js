// (c) 2023 Johnson Controls.  All Rights reserved.

function sendStartLatencyTest() {
    // We need WebRTC to be active to do a latency test.
    if (!webRtcPlayerObj) {
        return;
    }

    let onTestStarted = function(StartTimeMs) {
        let descriptor = {
            StartTime: StartTimeMs
        };
    };

    webRtcPlayerObj.startLatencyTest(onTestStarted);
}

let VideoEncoderQP = "N/A";

function setupWebRtcPlayer( config) {
    webRtcPlayerObj = new webRtcPlayer(config);
}

function onWebRtcAnswer() {
    let printInterval = 5 * 60 * 1000; // every 5 minutes
    let nextPrintDuration = printInterval;

    webRtcPlayerObj.onAggregatedStats = (aggregatedStatsMap) => 
    {
        const orangeQP = 26;
        const redQP = 35;

        let statsText = '';

        let color = "lime";
        if (VideoEncoderQP > redQP) {
            color = "red";
            blinkQualityStatus(2);
            statsText += `<div style="color: ${color}">Bad network connection</div>`;
        } else if (VideoEncoderQP > orangeQP) {
            color = "orange";
            blinkQualityStatus(1);
            statsText += `<div style="color: ${color}">Spotty network connection</div>`;
        }

        qualityStatus.className = `${color}Status`;

         for (let [key, aggregatedStats] of aggregatedStatsMap) {
            let numberFormat = new Intl.NumberFormat(window.navigator.language, {
                maximumFractionDigits: 0
            });
            let timeFormat = new Intl.NumberFormat(window.navigator.language, {
                maximumFractionDigits: 0,
                minimumIntegerDigits: 2
            });

            // Calculate duration of run
            let runTime = (aggregatedStats.timestamp - aggregatedStats.timestampStart) / 1000;
            let timeValues = [];
            let timeDurations = [60, 60];
            for (let timeIndex = 0; timeIndex < timeDurations.length; timeIndex++) {
                timeValues.push(runTime % timeDurations[timeIndex]);
                runTime = runTime / timeDurations[timeIndex];
            }
            timeValues.push(runTime);

            let runTimeSeconds = timeValues[0];
            let runTimeMinutes = Math.floor(timeValues[1]);
            let runTimeHours = Math.floor([timeValues[2]]);

            receivedBytesMeasurement = 'B';
            receivedBytes = aggregatedStats.hasOwnProperty('bytesReceived') ? aggregatedStats.bytesReceived : 0;
            let dataMeasurements = ['kB', 'MB', 'GB'];
            for (let index = 0; index < dataMeasurements.length; index++) {
                if (receivedBytes < 100 * 1000)
                    break;
                receivedBytes = receivedBytes / 1000;
                receivedBytesMeasurement = dataMeasurements[index];
            }

            let qualityStatus = document.getElementById("qualityStatus");

            // "blinks" quality status element for 1 sec by making it transparent, speed = number of blinks
            let blinkQualityStatus = function(speed) {
                let iter = speed;
                let opacity = 1; // [0..1]
                let tickId = setInterval(
                    function() {
                        opacity -= 0.1;
                        // map `opacity` to [-0.5..0.5] range, decrement by 0.2 per step and take `abs` to make it blink: 1 -> 0 -> 1
                        qualityStatus.style = `opacity: ${Math.abs((opacity - 0.5) * 2)}`;
                        if (opacity <= 0.1) {
                            if (--iter == 0) {
                                clearInterval(tickId);
                            } else { // next blink
                                opacity = 1;
                            }
                        }
                    },
                    100 / speed // msecs
                );
            };

            statsText += `<div>Trackid: ${aggregatedStats.trackIdentifier}</div>`;
            statsText += `<div>Duration: ${timeFormat.format(runTimeHours)}:${timeFormat.format(runTimeMinutes)}:${timeFormat.format(runTimeSeconds)}</div>`;
            statsText += `<div>Video Resolution: ${
    			aggregatedStats.hasOwnProperty('frameWidth') && aggregatedStats.frameWidth && aggregatedStats.hasOwnProperty('frameHeight') && aggregatedStats.frameHeight ?
    				aggregatedStats.frameWidth + 'x' + aggregatedStats.frameHeight : 'Chrome only'
    			}</div>`;
            statsText += `<div>Received (${receivedBytesMeasurement}): ${numberFormat.format(receivedBytes)}</div>`;
            statsText += `<div>Frames Decoded: ${aggregatedStats.hasOwnProperty('framesDecoded') ? numberFormat.format(aggregatedStats.framesDecoded) : 'Chrome only'}</div>`;
            statsText += `<div>Packets Lost: ${aggregatedStats.hasOwnProperty('packetsLost') ? numberFormat.format(aggregatedStats.packetsLost) : 'Chrome only'}</div>`;
            statsText += `<div style="color: ${color}">Bitrate (kbps): ${aggregatedStats.hasOwnProperty('bitrate') ? numberFormat.format(aggregatedStats.bitrate) : 'Chrome only'}</div>`;
            statsText += `<div>Framerate: ${aggregatedStats.hasOwnProperty('framerate') ? numberFormat.format(aggregatedStats.framerate) : 'Chrome only'}</div>`;
            statsText += `<div>Frames dropped: ${aggregatedStats.hasOwnProperty('framesDropped') ? numberFormat.format(aggregatedStats.framesDropped) : 'Chrome only'}</div>`;
            //  RTT, QP and latency 
            // statsText += `<div>Net RTT (ms): ${aggregatedStats.hasOwnProperty('currentRoundTripTime') ? numberFormat.format(aggregatedStats.currentRoundTripTime * 1000) : 'Can\'t calculate'}</div>`;
            // statsText += `<div>Browser receive to composite (ms): ${aggregatedStats.hasOwnProperty('receiveToCompositeMs') ? numberFormat.format(aggregatedStats.receiveToCompositeMs) : 'Chrome only'}</div>`;
            // statsText += `<div style="color: ${color}">Video Quantization Parameter: ${VideoEncoderQP}</div>`;
            statsText += `<div>................................................................... </div>`;
        }

        let statsDiv = document.getElementById("stats");
        statsDiv.innerHTML = statsText;      
    };

    webRtcPlayerObj.aggregateStats(1 * 1000 /*Check every 1 second*/ );

    webRtcPlayerObj.latencyTestTimings.OnAllLatencyTimingsReady = function(timings) {
        if (!timings.BrowserReceiptTimeMs) {
            return;
        }

        let latencyExcludingDecode = timings.BrowserReceiptTimeMs - timings.TestStartTimeMs;
        let uePixelStreamLatency = timings.UEPreEncodeTimeMs == 0 || timings.UEPreCaptureTimeMs == 0 ? "???" : timings.UEPostEncodeTimeMs - timings.UEPreCaptureTimeMs;
        let captureLatency = timings.UEPostCaptureTimeMs - timings.UEPreCaptureTimeMs;
        let encodeLatency = timings.UEPostEncodeTimeMs - timings.UEPreEncodeTimeMs;
        let ueLatency = timings.UETransmissionTimeMs - timings.UEReceiptTimeMs;
        let networkLatency = latencyExcludingDecode - ueLatency;
        let browserSendLatency = latencyExcludingDecode - networkLatency - ueLatency;

        // These ones depend on FrameDisplayDeltaTimeMs
        let endToEndLatency = null;
        let browserSideLatency = null;

        if (timings.FrameDisplayDeltaTimeMs && timings.BrowserReceiptTimeMs) {
            endToEndLatency = timings.FrameDisplayDeltaTimeMs + latencyExcludingDecode;
            browserSideLatency = endToEndLatency - networkLatency - ueLatency;
        }

        let latencyStatsInnerHTML = '';
        latencyStatsInnerHTML += `<div>Net latency RTT (ms): ${networkLatency}</div>`;
        latencyStatsInnerHTML += `<div>UE Capture+Encode (ms): ${uePixelStreamLatency}</div>`;
        latencyStatsInnerHTML += `<div>UE Capture (ms): ${captureLatency}</div>`;
        latencyStatsInnerHTML += `<div>UE Encode (ms): ${encodeLatency}</div>`;
        latencyStatsInnerHTML += `<div>Total UE latency (ms): ${ueLatency}</div>`;
        latencyStatsInnerHTML += `<div>Browser send latency (ms): ${browserSendLatency}</div>`
        latencyStatsInnerHTML += timings.FrameDisplayDeltaTimeMs && timings.BrowserReceiptTimeMs ? `<div>Browser receive latency (ms): ${timings.FrameDisplayDeltaTimeMs}</div>` : "";
        latencyStatsInnerHTML += browserSideLatency ? `<div>Total browser latency (ms): ${browserSideLatency}</div>` : "";
        latencyStatsInnerHTML += `<div>Total latency (excluding browser) (ms): ${latencyExcludingDecode}</div>`;
        latencyStatsInnerHTML += endToEndLatency ? `<div>Total latency (ms): ${endToEndLatency}</div>` : "";
        document.getElementById("LatencyStats").innerHTML = latencyStatsInnerHTML;
    }
}

function start() {
    // update "quality status" to "disconnected" state
    let qualityStatus = document.getElementById("qualityStatus");
    if (qualityStatus) {
        qualityStatus.className = "grey-status";
    }

    let statsDiv = document.getElementById("stats");
    if (statsDiv) {
        statsDiv.innerHTML = 'Not connected';
    }
}

function load() {
    start();
}
