#!/bin/bash

# Check if the correct number of arguments are provided
if [ "$#" -ne 6 ]; then
  echo "Usage: <ipaddress> <port> <numClients> <codefile> <loopNum> <sleepTimeSeconds>"
  exit 1
fi

ipaddress=$1
port=$2
numClients=$3
codefile=$4
loopNum=$5
sleepTime=$6

# Create a directory for client output files
mkdir -p client_output

# Create a file to store the performance data
data_file="performance_data.dat"

# Start M clients in the background
for ((i = 0; i < $numClients; i++)); do
  ./client "$ipaddress" "$port" "$codefile" "$loopNum" "$sleepTime" > "./client_output/client_$i.txt" &
done

# Wait for all clients to finish
wait


totalThroughput=0
totalResponseTime=0
totalResponses=0
averageResponseTime=0

for ((i = 0; i < numClients; i++)); do
  responses=$(grep "Successful Responses:" "client_output/client_$i.txt" | cut -d ':' -f 2)
  echo "responses: $responses"
  throughput=$(grep "Throughput:" "client_output/client_$i.txt" | cut -d ':' -f 2)
  echo "throughput: $throughput"
  responseTime=$(grep "Total Time:" "client_output/client_$i.txt" | cut -d ':' -f 2)  # Corrected index here
  echo "responseTime: $responseTime"
  
  totalResponses=$((totalResponses + responses))
  totalThroughput=$(echo "$totalThroughput + ($responses * $throughput)" | bc)
  totalResponseTime=$(echo "$totalResponseTime + $responses * $responseTime" | bc)
 # Corrected variable name here

  # Save this data into a file
  
done
if [ "$totalResponses" -gt 0 ]; then
  averageResponseTime=$(echo "scale=6; $totalResponseTime / $totalResponses" | bc)
  echo "$numClients $totalThroughput $averageResponseTime" >> "$data_file"
fi
