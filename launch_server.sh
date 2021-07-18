if [[ "$#" -ne 2 ]]; then
	echo "Use $0 <port> <RPC server host>"
	exit 1
fi

cd WebService/textConversor/bin
java textConversor.Publisher &
sleep 1
cd ../../../
server/bin/server -p $1 -s $2

