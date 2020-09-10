if [ $# -eq 0 ]
	then
		echo "No arguments supplied use 32 or 64"
		exit
fi

if [ "$1" -eq "32" ]
	then
		bash build32.sh
		exit
fi

if [ "$1" -eq "64" ]
	then
		bash build64.sh
		exit
fi