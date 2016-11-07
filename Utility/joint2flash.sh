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
                bootload=$2
		cpu0=$3
		cpu1=$4
		cpu2=$5
	;;
	o) #"this is the output information"
		output=$7
	;;
	h) #"this is the help information"
                helptext
		exit
	;;
	esac
done

outputtxt=flash.txt
output=$7


#delete the line address of flash txt
sed -i 's/@.\{9\} \{3\}//g' $bootload
sed -i 's/@.\{9\} \{3\}//g' $cpu0
sed -i 's/@.\{9\} \{3\}//g' $cpu1
sed -i 's/@.\{9\} \{3\}//g' $cpu2

echo "Making the image package, please wait ..."

#get the length of bootload/cpu0cpu1/cpu2.txt
bootloadlength=`cat $bootload | wc -l`
cpu0length=`cat $cpu0 | wc -l`
cpu1length=`cat $cpu1 | wc -l`
cpu2length=`cat $cpu2 | wc -l`

#echo "cpu0length $cpu0length"
#echo "cpu1length $cpu1length"
#echo "cpu2length $cpu2length"

cat $bootload > $outputtxt
#add "0" to the 64K offset
zerolength=$((65536-$bootloadlength))
#echo "zerolength is $zerolength"
for ((i=0; i<zerolength; i++));
do
echo '0' >> $outputtxt
done

#add size of cpu1 to flash.image
for i in {0..3}
do
        shiftlen=$[ i * 8 ]
        tmp=`echo $cpu0length $shiftlen | awk '{print rshift($1,$2)}'`
        tmp=`echo $tmp | awk '{print and($1,255)}'`
        tmphex=$(dec2hex $tmp)
        echo $tmphex >> $outputtxt
done

cat $cpu0 >> $outputtxt

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
