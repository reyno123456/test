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
		bootload_origin=$2
		bootload=$3
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

outputtxt=flash.txt
outputboottxt=flash_boot.txt
outputapptxt=flash_app.txt
outputboot=$8
outputapp=$9
output=$10

#delete the line address of flash txt
sed -i 's/@.\{9\} \{3\}//g' $bootload_origin
sed -i 's/@.\{9\} \{3\}//g' $bootload
sed -i 's/@.\{9\} \{3\}//g' $cpu0
sed -i 's/@.\{9\} \{3\}//g' $cpu1
sed -i 's/@.\{9\} \{3\}//g' $cpu2

echo "Making the image package, please wait ..."

#get the length of bootload/cpu0cpu1/cpu2.txt
bootloadoriginlength=`cat $bootload_origin | wc -l`
bootloadlength=`cat $bootload | wc -l`
cpu0length=`cat $cpu0 | wc -l`
cpu1length=`cat $cpu1 | wc -l`
cpu2length=`cat $cpu2 | wc -l`

#echo "cpu0length $cpu0length"
#echo "cpu1length $cpu1length"
#echo "cpu2length $cpu2length"

cat $bootload_origin > $outputtxt
#add "0" to the 8K offset

zerolengthorigin=$((8192 - $bootloadoriginlength))
for ((j=0; j<zerolengthorigin; j++));
do
echo '0' >> $outputtxt
done

cat $bootload >> $outputtxt
cat $bootload >  $outputboottxt

#add "0" to the 120K offset
zerolength=$((122880 - $bootloadlength))
for ((i=0; i<zerolength; i++));
do
echo '0' >> $outputtxt
done

echo '34' >> $outputboottxt
echo '45' >> $outputboottxt
echo '67' >> $outputboottxt
zerolengthboot=$((61437 - $bootloadlength))
for ((i=0; i<zerolengthboot; i++));
do
echo '0' >> $outputboottxt
done


echo '65' >> $outputtxt
echo '65' >  $outputapptxt

echo '82' >> $outputtxt
echo '82' >> $outputapptxt

echo '84' >> $outputtxt
echo '84' >> $outputapptxt

echo '79' >> $outputtxt
echo '79' >> $outputapptxt

echo '83' >> $outputtxt
echo '83' >> $outputapptxt

echo '89' >> $outputtxt
echo '89' >> $outputapptxt

echo '78' >> $outputtxt
echo '78' >> $outputapptxt

echo '83' >> $outputtxt
echo '83' >> $outputapptxt

#add size of cpu1 to flash.image
for i in {0..3}
do
        shiftlen=$[ i * 8 ]
        tmp=`echo $cpu0length $shiftlen | awk '{print rshift($1,$2)}'`
        tmp=`echo $tmp | awk '{print and($1,255)}'`
        tmphex=$(dec2hex $tmp)
        echo $tmphex >> $outputtxt
        echo $tmphex >> $outputapptxt
done

cat $cpu0 >> $outputtxt
cat $cpu0 >> $outputapptxt
#add size of cpu1 to flash.image
for i in {0..3}
do
	shiftlen=$[ i * 8 ]
	tmp=`echo $cpu1length $shiftlen | awk '{print rshift($1,$2)}'`
	tmp=`echo $tmp | awk '{print and($1,255)}'`
	tmphex=$(dec2hex $tmp)
	echo $tmphex >> $outputtxt
	echo $tmphex >> $outputapptxt
done

cat $cpu1 >> $outputtxt
cat $cpu1 >> $outputapptxt
#add size of cpu2 to flash.image
for i in {0..3}
do
	shiftlen=$[ i * 8 ]
	tmp=`echo $cpu2length $shiftlen | awk '{print rshift($1,$2)}'`
	tmp=`echo $tmp | awk '{print and($1,255)}'`
	tmphex=$(dec2hex $tmp)
	echo $tmphex >> $outputtxt
	echo $tmphex >> $outputapptxt
done
cat $cpu2 >> $outputtxt
cat $cpu2 >> $outputapptxt

echo '34' >> $outputtxt
echo '34' >> $outputapptxt

echo '45' >> $outputtxt
echo '45' >> $outputapptxt

echo '67' >> $outputtxt
echo '67' >> $outputapptxt
#transfer the ascii format to hexadecimal
../../Utility/txt2bin.exe  -i $outputboottxt -o $outputboot
../../Utility/txt2bin.exe  -i $outputapptxt -o $outputapp
../../Utility/txt2bin.exe  -i $outputtxt -o ar8020.bin
rm ../../Application/AR8020Verification/flash*
