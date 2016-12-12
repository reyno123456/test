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


if [ $# -lt 3 ]; then
	echo "***Too few parameters***"
	helptext
	exit
fi

while getopts ":i:o:h" OPTION
do
	case $OPTION in
	i) #"this is the input information"
		bootload=$2
		upgrade=$3
		cpu0=$4
		cpu1=$5
		cpu2=$6
	;;
	o) #"this is the output information"
		outputboot=$8
        outputapp=$9		
        output=$10
	;;
	h) #"this is the help information"
                helptext
		exit
	;;
	esac
done

outputtxt=ar8020.bin
outputboottxt=boot.bin
outputapptxt=app.bin
outputboot=$8
outputapp=$9
output=$10

echo "Making the image package, please wait ..."

#get the length of bootload/cpu0cpu1/cpu2.txt
bootloadlength=`stat --format=%s $bootload`
upgradelength=`stat --format=%s $upgrade`
cpu0length=`stat --format=%s $cpu0`
cpu1length=`stat --format=%s $cpu1`
cpu2length=`stat --format=%s $cpu2`



#add boot.bin
cat $bootload > $outputtxt
#add "0" to the 8K offset
zerolengthboot=$((8192 - $bootloadlength))
dd if=/dev/zero of=zero.image bs=$zerolengthboot count=1
cat zero.image >> $outputtxt
#add upgrade.bin
cat $upgrade >> $outputtxt
#add "0" to the 128K offset 
zerolength=$((122880 - $upgradelength))
dd if=/dev/zero of=zero.image bs=$zerolength count=1
cat zero.image >> $outputtxt

echo -n -e \\x65 >> $outputtxt
echo -n -e \\x82 >> $outputtxt
echo -n -e \\x84 >> $outputtxt
echo -n -e \\x79 >> $outputtxt
echo -n -e \\x83 >> $outputtxt
echo -n -e \\x89 >> $outputtxt
echo -n -e \\x78 >> $outputtxt
echo -n -e \\x83 >> $outputtxt

echo -n -e \\x65 > $outputapptxt
echo -n -e \\x82 >> $outputapptxt
echo -n -e \\x84 >> $outputapptxt
echo -n -e \\x79 >> $outputapptxt
echo -n -e \\x83 >> $outputapptxt
echo -n -e \\x89 >> $outputapptxt
echo -n -e \\x78 >> $outputapptxt
echo -n -e \\x83 >> $outputapptxt
#add size of cpu0 to ar8020.bin
for i in {0..3}
do
        shiftlen=$[ i * 8 ]
        tmp=`echo $cpu0length $shiftlen | awk '{print rshift($1,$2)}'`
        tmp=`echo $tmp | awk '{print and($1,255)}'`
        tmphex=$(dec2hex $tmp)
        echo -n -e \\x$tmphex >> $outputtxt
        echo -n -e \\x$tmphex >> $outputapptxt
done
#add cpu0.bin to ar8020.bin
cat $cpu0 >> $outputtxt
cat $cpu0 >> $outputapptxt
#add size of cpu1 to ar8020.bin
for i in {0..3}
do
        shiftlen=$[ i * 8 ]
        tmp=`echo $cpu1length $shiftlen | awk '{print rshift($1,$2)}'`
        tmp=`echo $tmp | awk '{print and($1,255)}'`
        tmphex=$(dec2hex $tmp)
        echo -n -e \\x$tmphex >> $outputtxt
        echo -n -e \\x$tmphex >> $outputapptxt
done
#add cpu1.bin to ar8020.bin
cat $cpu1 >> $outputtxt
cat $cpu1 >> $outputapptxt
#add size of cpu2 to ar8020.bin
for i in {0..3}
do
        shiftlen=$[ i * 8 ]
        tmp=`echo $cpu2length $shiftlen | awk '{print rshift($1,$2)}'`
        tmp=`echo $tmp | awk '{print and($1,255)}'`
        tmphex=$(dec2hex $tmp)
        echo -n -e \\x$tmphex >> $outputtxt
        echo -n -e \\x$tmphex >> $outputapptxt
done
#add cpu2.bin to ar8020.bin
cat $cpu2 >> $outputtxt
cat $cpu2 >> $outputapptxt


echo -n -e \\x34 >> $outputapptxt
echo -n -e \\x45 >> $outputapptxt
echo -n -e \\x67 >> $outputapptxt

cat $upgrade >> $outputboottxt
echo -n -e \\x34 >> $outputboottxt
echo -n -e \\x45 >> $outputboottxt
echo -n -e \\x67 >> $outputboottxt


rm zero.image
