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

while getopts ":i:h" OPTION
do
	case $OPTION in
	i) #"this is the input information"		
        bootimageaddr=$2
        bootimagemajorversion=$3
        bootimageminorversion=$4
        appimageaddr=$5
        appimagemajorversion=$6
        appimageminorversion=$7        
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
outputtmp=apptmp.bin


bootload=../../Output/Staging/Lib/ar8020_boot.bin
upgrade=../../Output/Staging/Lib/ar8020_upgrade.bin
cpu0=../../Output/Staging/Lib/ar8020_cpu0.bin
cpu1=../../Output/Staging/Lib/ar8020_cpu1.bin
cpu2=../../Output/Staging/Lib/ar8020_cpu2.bin

echo "Making the image package, please wait ..."

imagedate=`date "+%Y%m%d%H%M%S"`

#get the length of bootload/cpu0cpu1/cpu2.txt
bootloadlength=`stat --format=%s $bootload`
upgradelength=`stat --format=%s $upgrade`
cpu0length=`stat --format=%s $cpu0`
cpu1length=`stat --format=%s $cpu1`
cpu2length=`stat --format=%s $cpu2`

cat $bootload > $outputtxt
#add "0" to the 8K offset
zerolengthboot=$((8192 - $bootloadlength))
dd if=/dev/zero of=zero.image bs=$zerolengthboot count=1
cat zero.image >> $outputtxt

#add upgrade.bin
#add YMD
echo -n -e \\x0 > $outputboottxt
echo -n -e \\x0 >> $outputtxt
for i in {0..6}
do
        shiftlen=$[ i * 2 + 1]
        tmp=`expr substr "$imagedate" $shiftlen 2`
        echo -n -e \\x$tmp >> $outputtxt
        echo -n -e \\x$tmp >> $outputboottxt
done
#locad addr
for i in {0..3}
do
        shiftlen=$[ i * 2 + 1]
        tmp=`expr substr "$bootimageaddr" $shiftlen 2`
        echo -n -e \\x$tmp >> $outputtxt
        echo -n -e \\x$tmp >> $outputboottxt
done
#bootimageversion
echo -n -e \\x$bootimagemajorversion >> $outputboottxt
echo -n -e \\x$bootimageminorversion >> $outputboottxt
echo -n -e \\x$bootimagemajorversion >> $outputtxt
echo -n -e \\x$bootimageminorversion >> $outputtxt

upgradelengthhead=$((34+$upgradelength))
#echo $upgradelengthhead
#image length
for i in {0..3}
do
        shiftlen=$[ i * 8 ]
        tmp=`echo $upgradelengthhead $shiftlen | awk '{print rshift($1,$2)}'`
        tmp=`echo $tmp | awk '{print and($1,255)}'`
        tmphex=$(dec2hex $tmp)
        echo -n -e \\x$tmphex >> $outputtxt
        echo -n -e \\x$tmphex >> $outputboottxt
        #echo -n -e \\x$tmp >> $outtst
done

md5=`md5sum $upgrade | cut -d ' ' -f 1`
#echo `md5sum $upgrade | cut -d ' ' -f 1`
for i in {0..15}
do
        shiftlen=$[ i * 2 + 1]
        tmp=`expr substr "$md5" $shiftlen 2`
        echo -n -e \\x$tmp >> $outputboottxt
        echo -n -e \\x$tmp >> $outputtxt
done
cat $upgrade >> $outputtxt
cat $upgrade >> $outputboottxt
#add "0" to the 128K offset 
zerolength=$((122880 - $upgradelengthhead))
dd if=/dev/zero of=zero.image bs=$zerolength count=1
cat zero.image >> $outputtxt

#add app.bin
echo -n -e \\x0 > $outputapptxt
echo -n -e \\x0 >> $outputtxt
for i in {0..6}
do
        shiftlen=$[ i * 2 + 1]
        tmp=`expr substr "$imagedate" $shiftlen 2`
        echo -n -e \\x$tmp >> $outputtxt
        echo -n -e \\x$tmp >> $outputapptxt
done
#locad addr
for i in {0..3}
do
        shiftlen=$[ i * 2 + 1]
        tmp=`expr substr "$appimageaddr" $shiftlen 2`
        echo -n -e \\x$tmp >> $outputtxt
        echo -n -e \\x$tmp >> $outputapptxt
done
#bootimageversion
echo -n -e \\x$appimagemajorversion >> $outputapptxt
echo -n -e \\x$appimageminorversion >> $outputapptxt
echo -n -e \\x$bootimagemajorversion >> $outputtxt
echo -n -e \\x$bootimageminorversion >> $outputtxt

applengthhead=$((46+$cpu0length+$cpu1length+$cpu2length))
#echo $applengthhead
#image length
for i in {0..3}
do
        shiftlen=$[ i * 8 ]
        tmp=`echo $applengthhead $shiftlen | awk '{print rshift($1,$2)}'`
        tmp=`echo $tmp | awk '{print and($1,255)}'`
        tmphex=$(dec2hex $tmp)
        echo -n -e \\x$tmphex >> $outputtxt
        echo -n -e \\x$tmphex >> $outputapptxt
done

#add size of cpu0 to ar8020.bin
for i in {0..3}
do
        shiftlen=$[ i * 8 ]
        tmp=`echo $cpu0length $shiftlen | awk '{print rshift($1,$2)}'`
        tmp=`echo $tmp | awk '{print and($1,255)}'`
        tmphex=$(dec2hex $tmp)
        echo -n -e \\x$tmphex >> $outputtmp
done
#add cpu0.bin to ar8020.bin
cat $cpu0 >> $outputtmp
#add size of cpu1 to ar8020.bin
for i in {0..3}
do
        shiftlen=$[ i * 8 ]
        tmp=`echo $cpu1length $shiftlen | awk '{print rshift($1,$2)}'`
        tmp=`echo $tmp | awk '{print and($1,255)}'`
        tmphex=$(dec2hex $tmp)
        echo -n -e \\x$tmphex >> $outputtmp
done
#add cpu1.bin to ar8020.bin
cat $cpu1 >> $outputtmp
#add size of cpu2 to ar8020.bin
for i in {0..3}
do
        shiftlen=$[ i * 8 ]
        tmp=`echo $cpu2length $shiftlen | awk '{print rshift($1,$2)}'`
        tmp=`echo $tmp | awk '{print and($1,255)}'`
        tmphex=$(dec2hex $tmp)
        echo -n -e \\x$tmphex >> $outputtmp
done
#add cpu2.bin to ar8020.bin
cat $cpu2 >> $outputtmp

md5=`md5sum $outputtmp | cut -d ' ' -f 1`
#echo `md5sum $outputtmp | cut -d ' ' -f 1`
for i in {0..15}
do
        shiftlen=$[ i * 2 + 1]
        tmp=`expr substr "$md5" $shiftlen 2`
        echo -n -e \\x$tmp >> $outputapptxt
        echo -n -e \\x$tmp >> $outputtxt
done
cat $outputtmp >> $outputapptxt
cat $outputtmp >> $outputtxt

rm zero.image
rm $outputtmp
