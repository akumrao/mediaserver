#!/bin/bash
SECONDS=1
i=1

while true
do
        echo "`date`: Loop $i"
       
        curl -k -X DELETE https://103.130.108.250:3000   -H 'Content-Type: application/json'   -d '{    "7": {         "rtsp": "rtsp://localhost/test.264",        "state": "streaming"     },     "8": {        "rtsp":"rtsp://localhost/test1.264",        "state": "streaming"    }    }'

        i=$(( $i+1 ))

        sleep 1

        echo "`date`: Loop $i"
        
        curl -k -X PUT https://103.130.108.250:3000   -H 'Content-Type: application/json'   -d '{    "7": {         "rtsp": "rtsp://localhost/test.264",        "state": "streaming"     },     "8": {        "rtsp":"rtsp://localhost/test1.264",        "state": "streaming"    }    }'


        i=$(( $i+1 ))

        sleep 1
done
