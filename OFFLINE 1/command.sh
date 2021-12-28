#!/bin/bash

h=$(pwd)


if [ -d $1 ]
then 
	directory=$1
elif [ -d $2 ]
then 
	directory=$2
else
	directory=.
fi

	
if [[ -f $1 && -n $1 ]]; then
	file=$1
elif [[ -f $2  && -n $2 ]]; then 
	file=$2
else 
	echo "you have to give a input file as a command line argument to use this script efficiently"
	echo "./command.sh working_directory_name input.txt"
	
fi

if [ -z $file ]
then
	echo "give the file name: "
	read f
	file=$f
fi

sed -i 's/\r//g' $file	

#commented here

i=0

while read line
do
	echo "line no $((i + 1)): $line"
	ARRAY[$i]=$line
	((i =  i + 1))
	
done < $file

#echo ${ARRAY[*]}

#find output_dir  -not \( -name "*.html" -o -name "*.pdf" \)


temp=temp12_12_12.txt
temp2=temp22_22_22.txt

find $directory -type f -name "*.*" > $temp

find $directory -type f ! -name "*.*" > $temp2

for i in "${ARRAY[@]}"
do

    sed -i "/.*\.$i/d" $temp
done

sed -i "/temp12_12_12.txt/d" $temp


#sed 's/.*\///' to get the file name from path

# /.*\.txt

# sed 's/.*\.txt/bla/' for file extension

# sed "s/.*\.//" temp.txt to find out extensions only



p="${h}/${directory}/.."
out=$p/output_directory


rm -r $out
mkdir -p $out


total=0
while read line
do
	

	
	IN=$line
	fileexten=(${IN//./ })
	l=${#fileexten[@]}
	((l=l-1))
	#echo ${fileexten[$l]}
	dir=$out/${fileexten[$l]}
	#echo $dir
	if [ -d $dir ]
	then 
		cp "$line" $dir
		echo $line >> $dir/dest_${fileexten[$l]}.txt
		((total = total + 1))
	else
		mkdir -p $dir
		cp "$line" $dir
		
		echo $line > $dir/dest_${fileexten[$l]}.txt
		((total = total + 1))
		
	fi
	
done < $temp


while read line
do

	dir=$out/others
	#echo $dir
	if [ -d $dir ]
	then 
		
		cp "$line" $dir
		echo $line >> $dir/dest_others.txt
		((total = total + 1))
	else
		mkdir -p $dir
		cp "$line" $dir
		echo $line > $dir/dest_others.txt
		((total = total + 1))
		
	fi
	
done < $temp2

rm $temp
rm $temp2


directories=$(find $out -type d)

alltogether=$(find $directory -type f|wc -l)
((alltogether = alltogether - total))
echo -e 'file_type\tno_of_files' > $temp 

f=0


for i in $directories
do
	if((f == 0))
	then 
		
		((f=1))
	else
		fileexten=(${i///// })
		l=${#fileexten[@]}
		((l=l-1))
		#totalofthis=$(cat $out/${fileexten[l]}/dest_${fileexten[l]}.txt|wc -l)
		totalofthis=$(find $out/${fileexten[l]} -type f|wc -l)
		((totalofthis=totalofthis-1))
		echo -e "${fileexten[l]}\t$totalofthis" >> $temp
	fi
done



echo -e "ignored\t$alltogether" >> $temp
cat $temp > $p/out_myinput.csv
rm $temp



	