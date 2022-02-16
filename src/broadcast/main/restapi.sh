#!/bin/bash

i=1

while true
do
        echo "`date`: Loop $i"
       
        curl -k -X DELETE https://192.168.0.19:3000   -H 'Content-Type: application/json'   -d '{    "1": {         "rtsp": "rtsp://localhost/test.264",        "state": "streaming"     },     "2": {        "rtsp":"rtsp://localhost/test1.264",        "state": "streaming"    }    }'

        i=$(( $i+1 ))

        sleep 1

        echo "`date`: Loop $i"

        curl -k -X PUT https://192.168.0.19:3000   -H 'Content-Type: application/json'   -d '{    "1": {         "rtsp": "rtsp://localhost/test.264",        "state": "streaming"     },     "2": {        "rtsp":"rtsp://localhost/test1.264",        "state": "streaming"    }    }'

        i=$(( $i+1 ))

        sleep 1
done
