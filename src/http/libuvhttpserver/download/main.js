(function () {
    'use strict';


    /**
     * Displays logging information on the screen and in the console.
     * @param {string} msg - Message to log.
     */
    function log(msg) {
        var logsEl = document.getElementById('logs');

        if (msg) {
            // Update logs
            console.log('[Download]: ', msg);
            logsEl.innerHTML += msg + '<br />';
        } else {
            // Clear logs
            logsEl.innerHTML = '';
        }
        logsEl.scrollTop = logsEl.scrollHeight;
    }

    /**
     * Displays information about downloaded files.
     * @param {string} msg - Message to log.
     */
    function logFile(msg) {
        var logsEl = document.getElementById('fileList');

        if (msg) {
            // Update logs
            logsEl.innerHTML += msg + '<br />';
        } else {
            // Clear logs
            logsEl.innerHTML = '';
        }
    }

    /**
     * Register keys used in this application
     */
    function registerKeys() {
        var usedKeys = [
            'MediaPause', 'MediaPlay', 'MediaStop', 'MediaPlayPause',
            '0', '1', '2', '3'
        ];

        usedKeys.forEach(
            function (keyName) {
                tizen.tvinputdevice.registerKey(keyName);
            }
        );
    }
    
    var paused = false;

    /**
     * Handle input from remote
     */
    function registerKeyHandler() {
        var url1 = 'http://techslides.com/demos/sample-videos/small.mp4';
        var url2 = 'http://download.blender.org/peach/bigbuckbunny_movies/big_buck_bunny_1080p_h264.mov';

        document.addEventListener('keydown', function (e) {
            switch (e.keyCode) {
                // key 0
                case 48:
                    log();
                    break;
                // key 1
                case 49:
                    filesystem.listDownloadedFiles();
                    break;
                // key 2
                case 50:
                    download.start(url1);
                    break;
                // key 3
                case 51:
                    download.start(url2);
                    break;
                //key PLAY_PAUSE
                case 10252: 
                    if (paused) {
                    	download.resume();
                    } else {
                    	download.pause();
                    }
                    break;
                //key MediaPause
                case 19:
                    download.pause();
                    break;
                //key MediaPlay
                case 415:
                    download.resume();
                    break;
                //key MediaStop
                case 413:
                    download.cancel();
                    break;
                // Key exit
                case 10009:
                    tizen.application.getCurrentApplication().exit();
                    break;
                // Just log not used key
                default:
                    log('key pressed: ' + e.keyCode);
            }
        });
    }

    /**
     * Display application version
     */
    function displayVersion() {
        var el = document.createElement('div');
        el.id = 'version';
        el.innerHTML = 'ver: ' + tizen.application.getAppInfo().version;
        document.body.appendChild(el);
    }

    /**
     * Object handling file downloading
     */
    var download = {
        getInfo: function () {
            var downloadPossible = tizen.systeminfo.getCapability(
                'http://tizen.org/feature/download');

            var wifiCapable = tizen.systeminfo.getCapability(
                'http://tizen.org/feature/network.wifi');

            log('Download possible: ' + downloadPossible);
            log('Wifi capable: ' + wifiCapable);
        },

        start: function (url) {
            var req = new tizen.DownloadRequest(url, 'downloads', 'small' + Date.now(), 'ALL');
            req.httpHeader['Pragma'] = 'no-cache';

            var listener = {
                onprogress:  this.onProgress.bind(this),
                onpaused: this.onPaused.bind(this),
                oncanceled:  this.onCancelled.bind(this),
                oncompleted: this.onComplete.bind(this),
                onfailed: this.onFailed.bind(this)
            };

            this.id = tizen.download.start(req, listener);

            progress.reset();
        },

        pause: function () {
            if (this.id && tizen.download.getState(this.id) === 'DOWNLOADING') {
                try {
                	paused = true;
                    tizen.download.pause(this.id);
                } catch (e) {
                    log(e.message);
                }
            }
        },

        resume: function () {

            if (this.id && tizen.download.getState(this.id) === 'PAUSED') {
                try {
                	paused = false;
                    tizen.download.resume(this.id);
                } catch (e) {
                    log(e.message);
                }
            }
        },

        cancel: function () {
            var state;
            var isCorrectState;

            if (! this.id) {
                return;
            }

            state = tizen.download.getState(this.id);

            isCorrectState = state === 'QUEUED' || state === 'DOWNLOADING' || state === 'PAUSED';

            if (this.id && isCorrectState) {
                try {
                    tizen.download.cancel(this.id);
                } catch (e) {
                    log(e.message);
                }
            }
        },

        onProgress: function (id, receivedSize, totalSize) {
            log('Downloaded: ' + receivedSize + ' of ' + totalSize);
            progress.updateProgress(receivedSize, totalSize);
        },

        onPaused: function (id) {
            log('Download paused: ' + id);
        },

        onComplete: function (id, fullPath) {
            this.id = null;
            log('Download completed: ' + fullPath);

            progress.hide();

            filesystem.listDownloadedFiles();
        },

        onCancelled: function (id) {
            log('Download cancelled: ' + id);
            progress.hide();

            filesystem.listDownloadedFiles();
        },

        onFailed: function (error) {
            this.id = null;
            log('Download fail: ' + error);
        }

    };


    /**
     * Object handling filesystem operations
     */
    var filesystem = {

        deleteAllDownloads: function () {

            function deleteFile (handle, file) {
                handle.deleteFile(file.fullPath, function () {});
            }

            this.iterate(deleteFile);
        },

        listDownloadedFiles: function () {

            function listFile(handle, file) {
                logFile('File ' + file.name + ' size: ' + file.fileSize);
            }

            logFile();
            this.iterate(listFile);
        },

        iterate: function (callback) {

            tizen.filesystem.resolve(
                'downloads',
                function (handle) {
                    if (handle.isDirectory) {
                        handle.listFiles(function (files) {
                            files.forEach(function (file) {
                                callback(handle, file);
                            });
                        });
                    }
                },
                function (e) {
                    log('Error' + e.message);
                },
                'rw'
            );
        }
    };


    /**
     * Object handling updating of download progress bar
     */
    var progress = {
        init: function () {
            this.dom = document.getElementById('progress');
            this.barEl = document.getElementById('bar');
        },

        reset: function () {
            this.barEl.style.width = 0;
            this.show();
        },

        updateProgress: function (elapsed, total) {
            var progress = 100 * elapsed / total;

            this.barEl.style.width = progress + '%';
        },

        show: function () {
            this.dom.style.visibility = "visible";
        },

        hide: function () {
            this.dom.style.visibility = "hidden";
        }
    };

    /**
     * Start the application once loading is finished
     */
    window.onload = function init() {
        if (window.tizen === undefined) {
            log('This application needs to be run on Tizen device');
            return;
        }

        displayVersion();
        registerKeys();
        registerKeyHandler();


        filesystem.deleteAllDownloads();
        download.getInfo();
        progress.init();
    };
}());
