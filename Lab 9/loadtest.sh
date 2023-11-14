#!/usr/bin/bash

argc=$#
if [ $argc -ne 7 ]
then
echo "Usage : ./loadtest.sh <ip-addr> <port> <studentCode.cpp> <numClients> <loopNum> <sleepTimeSeconds> <timeout-seconds>"
exit 1
fi


ipaddr=$1
port=$2
studentCode=$3
numClients=$4
loopNum=$5
sleepTimeSeconds=$6
ct=$4
waittime=$7

for ((i=1;i<=$ct;i++));
do 
./client $1 $2 $3 $5 $6 $7 > "clientoutput$i.txt" &
done
echo "Launching $numClients clients in background"

wait

overallthroughput=0
avgResponseTime=0
numberofResponses=0
avgreqrate=0.0
avgsuccessfulrate=0
avgtimeoutrate=0
avgerrorrate=0

for ((j = 1; j <= numClients; j++)); do
  avgResponseTime_i=$(grep "Average Response Time:" "clientoutput$j.txt" | cut -d ':' -f 2)
#   echo "client $j's avgResponseTime = $avgResponseTime_i"
  successfulResponses_i=$(grep "Successful Responses:" "clientoutput$j.txt" | cut -d ':' -f 2)
#   echo "client $j's successfulResponses = $successfulResponses_i"
  avgreqrate_i=$(grep "Request Sent Rate:" "clientoutput$j.txt" | cut -d ':' -f 2)
#   echo "client $j's avgreqrate = $avgreqrate_i"
  avgsuccessfulrate_i=$(grep "Successful Request Rate:" "clientoutput$j.txt" | cut -d ':' -f 2)
#   echo "client $j's avgsuccessfulrate = $avgsuccessfulrate_i"
  avgtimeoutrate_i=$(grep "Timeout Rate:" "clientoutput$j.txt" | cut -d ':' -f 2)
  #echo "client $j's avgtimeoutrate = $avgtimeoutrate_i"
  avgerrorrate_i=$(grep "Error Rate:" "clientoutput$j.txt" | cut -d ':' -f 2)
#   echo "client $j's avgerrorrate = $avgerrorrate_i"
  avgResponseTime=$(echo "$avgResponseTime + $avgResponseTime_i" | bc -l)
#   echo "avgresponsetime = $avgResponseTime"
  throughput_i=$(grep "Throughput" "clientoutput$j.txt" | cut -d ':' -f 2)
  overallthroughput=$(echo "$overallthroughput + $throughput_i" | bc -l)
  # echo "overallthroughput = $overallthroughput"
  numberofResponses=$(echo "$numberofResponses + $successfulResponses_i" | bc -l)
#   echo "numberOfResponses = $numberofResponses"
  avgreqrate=$(echo "$avgreqrate + $avgreqrate_i" | bc -l)
#   echo "avgreqrate = $avgreqrate"
  avgsuccessfulrate=$(echo "$avgsuccessfulrate + $avgsuccessfulrate_i" | bc -l)
#   echo "avgsuccessfulrate = $avgsuccessfulrate"
  avgtimeoutrate=$(echo "$avgtimeoutrate + $avgtimeoutrate_i" | bc -l)
  #echo "avgtimeoutrate = $avgtimeoutrate"
  avgerrorrate=$(echo "$avgerrorrate + $avgerrorrate_i" | bc -l)
  #echo "avgerrorrate = $avgerrorrate"
done

echo "overallthroughput = $overallthroughput"
echo "avgresponsetime = $avgResponseTime"

avgResponseTime=$(echo "$avgResponseTime / $numClients" | bc -l)
echo "$numClients $overallthroughput" >> throughput_data.txt
echo "$numClients $avgResponseTime" >> response_time_data.txt
echo "$numClients $avgreqrate" >> request_rate.txt
echo "$numClients $avgsuccessfulrate" >> goodput_data.txt
echo "$numClients $avgtimeoutrate" >> timeout_rate.txt
echo "$numClients $avgerrorrate" >> error_rate.txt


rm clientoutput*