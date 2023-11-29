IP=$1
PORT=$2
FILE=$3
TIMEOUT=$4
PROG_ID=$5
./client "new" $IP $PORT $FILE $TIMEOUT $PROG_ID
REQ_ID=$(cat client_request_id_$PROG_ID.txt)

Exit_status=0
while [ $Exit_status -ne 3 ];
do
    sleep 2
    ./client "status" $IP $PORT $REQ_ID $TIMEOUT $PROG_ID
    Exit_status=$?
done
