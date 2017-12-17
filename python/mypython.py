# Assignment: Program Py
# By Johnny Po

import random
import string
from random import randint

myStr = ""
myFile = ""

for i in range(0, 3): 
	myFile = "file"
	myStr = ""
	
	for j in range(0, 10):
	   randStr = random.choice(string.ascii_lowercase)
	   myStr += randStr
	myStr += '\n'

	myFile += str(i+1)

	f = open(myFile, 'w+')
	f.write(myStr);
	print(myStr, end = "")

firstNum = randint(1, 42)
secNum = randint(1, 42)

print(firstNum)
print(secNum)
print(firstNum * secNum)

