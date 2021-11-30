#!/bin/bash



#commented here 



if [ -d $1 ]
then 
	directory=$1
elif [ -d $2 ]
then 
	directory=$2
else
	directory=.
fi

ls $directory



	
if [[ -f $1 && -n $1 ]]; then
	file=$1
elif [[ -f $2  && -n $2 ]]; then 
	file=$2
else 
	echo "you have to give a input file as a second command line argument to use this script efficiently"
	
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
: '
for i in "${ARRAY[@]}"
do
    # access each element 
    # as $i
    echo $i
done

echo ${#ARRAY[@]}
	
n="-name"
'
#find output_dir  -not \( -name "*.html" -o -name "*.pdf" \)


: '
if [ -n ${ARRAY[0]} ]; then
	fname="${n} \"*.${ARRAY[0]}\""
fi

echo $fname



echo $fname

fname="find . \( ${fname} \)"
echo $fname

"${fname}"


len=${#ARRAY[@]}

for((i=1;i<$len;i++))
do
	fname="$fname -o -name \"*.${ARRAY[$i]}\""
done

echo $fname

find $directory  -not \( $fname \)	


'

temp=temp.txt
temp2=temp2.txt

find $directory -type f -name "*.*" > $temp

find $directory -type f ! -name "*.*" > $temp2

for i in "${ARRAY[@]}"
do
    # access each element 
    # as $i
    #echo $i
    sed -i "/.*\.$i/d" $temp
done


#sed 's/.*\///' to get the file name from path

# /.*\.txt

# sed 's/.*\.txt/bla/' for file extension

# sed "s/.*\.//" temp.txt to find out extensions only

out=output_directory

mkdir -p $out


total=0
while read line
do
	#echo "line no $((i + 1)): $line"
	
	IN=$line
	fileexten=(${IN//./ })
	l=${#fileexten[@]}
	((l=l-1))
	#echo ${fileexten[$l]}
	dir=$out/${fileexten[$l]}
	#echo $dir
	if [ -d $dir ]
	then 
		
		cp $line $dir
		echo $line >> $dir/dest_${fileexten[$l]}.txt
		((total = total + 1))
	else
		mkdir $dir
		cp $line $dir
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
		
		cp $line $dir
		echo $line >> $dir/dest_${fileexten[$l]}.txt
		((total = total + 1))
	else
		mkdir $dir
		cp $line $dir
		echo $line > $dir/dest_${fileexten[$l]}.txt
		((total = total + 1))
		
	fi
	
done < $temp2


rm $temp
rm $temp2

directories=$(find $out -type d)

f=0

for i in $directories
do
	if((f == 0))
	then 
		
		((f=1))
	else
		fileexten=(${i///// })
		totalofthis=$(find $out/${fileexten[1]} -type f|wc -l)
		((totalofthis = totalofthis - 1))
		echo ${fileexten[1]},$totalofthis >> $temp
	fi
done


alltogether=$(find $directory -type f|wc -l)
((alltogether = alltogether - total))
echo ignored,$alltogether >> $temp

cat $temp > out_myinput.csv
rm $temp








	
