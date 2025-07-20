#!/bin/bash

pulseaudio

export LIBVA_DRIVER_NAME=i965 

export LD_LIBRARY_PATH=/usr/lib64/pango/1.8.0/modules:/opt/americandynamics/3rdParty/lib:/opt/americandynamics/venvr/lib:/opt/intel-media/lib64:/usr/X11R6/lib64:/usr/local/lib/:/usr/local/lib64/

export GST_PLUGIN_PATH=/opt/americandynamics/venvr/lib/gstreamer-1.0:/usr/lib64/gstreamer-1.0:/opt/americandynamics/venvr/device_handlers/intellex/gst-plugins:/opt/americandynamics/3rdParty/lib/gstreamer-1.0

export REMOTE_USER=nvrserviceuser
export TM_INTERLEAVE_SNDBUF=65536
export MDB_SOCKET_NOTIFICATION=1

export PYTHONPATH=/usr/lib64/pango/1.8.0/modules:/opt/americandynamics/3rdParty/lib:/opt/americandynamics/venvr/lib:/opt/intel-media/lib64:/usr/X11R6/lib64

export GST_PLUGIN_SCANNER=/usr/lib/gstreamer-1.0/gst-plugin-scanner-x86_64

export PATH=/opt/americandynamics/venvr/bin:/opt/americandynamics/3rdParty/bin:/sbin:/usr/sbin:/usr/local/sbin:/usr/local/bin:/usr/bin:

cd /workspace/acvs-VideoEdge-webrtc/NVR/xStore/rtc/src/webrtc/main

./runWebrtc
