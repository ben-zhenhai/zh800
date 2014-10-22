#!/bin/bash

SERVER_VERSION="0.0.1"

#dateString=`date --date='yesterday'`
dateString="2014-10-09"

SCRIPT_DIR=$(dirname $0)

REMOTE_IP="192.168.0.181"
REMOTE_SCRIPT_DIR="/home/zhenhai/dailyDump"
REMOTE_SCRIPT_DUMP="dump.sh"
REMOTE_SCRIPT_CLEAN="clean.sh"

ssh zhenhai@$REMOTE_IP $REMOTE_SCRIPT_DIR/$REMOTE_SCRIPT_DUMP
scp zhenhai@$REMOTE_IP:$REMOTE_SCRIPT_DIR/$dateString.tar.gz $SCRIPT_DIR/$dateString.tar.gz

tar -xvzf $SCRIPT_DIR/$dateString.tar.gz
mongorestore --db zhenhaiDaily --drop export$dateString/zhenhaiDaily
rm -rvf $dateString.tar.gz
rm -rvf export$dateString/

java -cp $SCRIPT_DIR/CommunicationServer-assembly-$SERVER_VERSION.jar tw.com.zhenhai.main.DailyImport $dateString

ssh zhenhai@$REMOTE_IP $REMOTE_SCRIPT_DIR/$REMOTE_SCRIPT_CLEAN

