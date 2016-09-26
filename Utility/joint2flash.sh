#!/bin/bash
# Author: Minzhao
# Date: 2016-9-23
# Verison: 0.1
# This script is used to cat cpu* binary file into one flash image
# Note: if you run this script and get the following error on linux: 
#       /bin/bash^M bad interpreter:No such file or directory
# you could do like this to convert the file to linux line-ending format:
#       vi joint2flash.sh
#       :set ff=unix and :wq 

function helptext()
{
   echo "[Usage:] "$0" -i cpu0.txt cpu1.txt cpu2.txt -o flash.image"
   exit
}

function dec2hex()
{
	printf "%x" $1
}


if [ $# -lt 6 ]; then
	echo "***Too few parameters***"
	helptext
	exit
fi

while getopts ":i:o:h" OPTION
do
	case $OPTION in
	i) #"this is the input information"
		cpu0=$2	
		cpu1=$3
		cpu2=$4
	;;
	o) #"this is the output information"
		output=$6
	;;
	h) #"this is the help information"
                helptext
		exit
	;;
	esac
done

outputtxt=flash.txt
output=$6


#delete the line address of flash txt
sed -i 's/@.\{9\} \{3\}//g' $cpu0
sed -i 's/@.\{9\} \{3\}//g' $cpu1
sed -i 's/@.\{9\} \{3\}//g' $cpu2

#get the length of cpu1/cpu2.txt
cpu1length=`cat $cpu1 | wc -l`
cpu2length=`cat $cpu2 | wc -l`

cat $cpu0 > $outputtxt

#add size of cpu1 to flash.image
for i in {0..3}
do
	shiftlen=$[ i * 8 ]
	tmp=`echo $cpu1length $shiftlen | awk '{print rshift($1,$2)}'`
	tmp=`echo $tmp | awk '{print and($1,255)}'`
	tmphex=$(dec2hex $tmp)
	echo $tmphex >> $outputtxt
done

cat $cpu1 >> $outputtxt

#add size of cpu2 to flash.image
for i in {0..3}
do
	shiftlen=$[ i * 8 ]
	tmp=`echo $cpu2length $shiftlen | awk '{print rshift($1,$2)}'`
	tmp=`echo $tmp | awk '{print and($1,255)}'`
	tmphex=$(dec2hex $tmp)
	echo $tmphex >> $outputtxt
done
cat $cpu2 >> $outputtxt

#transfer the ascii format to hexadecimal
../../Utility/txt2bin.exe  -i $outputtxt -o $output
