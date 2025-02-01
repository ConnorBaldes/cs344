To compile my program in os1 type:

    gcc --std=gnu99 -o movies_by_year ./main.c


* There is an assumption that I made when righting my program that is when in the directions it says "Here is 
  a sample file whose format corresponds to the format of the CSV file your program will be tested" that the naming
  of the files will also follow that format and have names starting with "movies_sample_". When searching for the 
  largest and smallest file my program searches for files starting with that string this is to prevent it from accidentally
  trying to read the executable which would cause a seg fault. If the files are named differently you can still run my program
  by just changing the variable "PREFIX" to whatever the file names start with. This issue does not apply to option three 
  where you type your own file name in. 

* I believe everything in my program is working correctly and meets all the requirements of the 
  assignment. I'd like to note that this assignments is definaetly not my prettiest or most efficent 
  work (many of my functions probably should have been split into smaller sections) but in my defense
  I wrote a large portion of this program while I had the flu so hopefully its ok:)