#!/bin/bash
# Johnny Po
# Assignment 1: stats.sh

# Variables for temporary filenames
tmpStdinFile=$(mktemp)
tmpFile=$(mktemp)

# General Variables 
average=0
median=0

# To easily manipulate file
inputFile=${args[1]}

# Error messages
inputError() {
	echo "Incorrect number of arguments, or wrong arguments. "
	echo "Usage: stats { -r | -c } [filename]"
	echo " "
	exit 1
}

readError() {
	echo "Cannot read file. "
	exit 1
}


# Cleaning up
cleanUp() {
	rm -f tmpFile
	rm -f tmpStdinFile
	echo " "
	echo "Cleanup completed. Program terminated. "
}


# Trap command to catch signals
trap "cleanUp $#; echo -e 'Signal detected, exiting...'; exit 1" INT HUP TERM



##############################
# Start of program
###############################
# Check number of command line arguments 
if [[ "$#" -lt 1 || "$#" -gt 2 ]]
then
   inputError   
fi


# Check if first argument is either -r or -c
if ! [[ $1 == -c* ]] && ! [[ $1 == -r* ]]
then
   inputError
fi

# Check if file is readable 
if test -f $inputFile 
then
   echo "File is readable. Continuing..."
  
else
   readError
fi

# Read each line and append into tmpStdinFile
while read line
do
   echo "$line" >> $tmpStdinFile
done < "${2:-/dev/stdin}"

# Put the read data into inputFile
inputFile=$tmpStdinFile


# If user chooses "-r", calculate for rows
if [[ $1 == -r* ]]
then
   echo "Average Median "  
  
   # Just to make sure all vars are set to 0 
   rowCount=0
   rowColumn=0
   rowTotal=0

   while read line
   do 
      # Count number of characters in each row
      rowCount=($(echo $line | wc -w))
      
      # Moves each character in the row to its own line for easier counting
      rowColumn=$(echo $line | tr "\t" "\n")

      # Count the sum of all the elements in row
      rowTotal=0
      for i in $rowColumn
      do
	 rowTotal=$(expr $i + $rowTotal)
      done	 
      
      # Calculate and print out the Average for row
      average=$(expr $rowTotal + $(expr $rowCount / 2))

      # Make sure there is no Divide by 0
      if [[ $rowCount == 0 ]] 
      then 
	 average=""

      else
	 average=$(expr $average / $rowCount)
      fi
      

      # Calculate and print out the Median for the rows using Piping
      median=$(echo "$line" | tr '\t' '\n' | sort -n | tr '\n' '\t' | cut -f $(expr $(expr $rowCount / 2) + 1))


      # Push average and median into tmpFile
      echo -e " $average	$median " >> $tmpFile
	
   done < $inputFile

   while read line
   do
      echo "$line"
   done < $tmpFile



elif [[ $1 == -c* ]]
then
   echo "Average Median "
   
   # Just to make sure all vars are set to 0
   columnCount=0
   
   # Calculate the number of columns 
   line=$(head -n 1 $inputFile)
   for i in $line
   do
      columnCount=$(expr $columnCount + 1)
   done

   echo -e "columnCount: " $columnCount

fi   

# Call the function to clean everything up
cleanUp

