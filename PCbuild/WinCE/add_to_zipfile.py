#
#	A simple Python program that allows us to add files to a zip file
#
import sys, os, zipfile

#
#	Make sure we have enough program arguments args
#
if len(sys.argv) < 4:
   sys.stderr.write("usage: add_to_zipfile zipfile base-directory file1 [file2] [file3] ...\n")
   sys.exit()
#
#	Get the zip file name and the other file names
#
Zip_File_Name = sys.argv[1]
Base_Directory = sys.argv[2]
Files = sys.argv[3:]
#
#	Attempt to open an existing zip file
#
try:
  Zip_File = zipfile.ZipFile(Zip_File_Name, "a" , zipfile.ZIP_DEFLATED)
except:
  #
  #	Not there: Create a new one
  #
  Zip_File = zipfile.ZipFile(Zip_File_Name, "w" , zipfile.ZIP_DEFLATED)
#
#	Add all the files
#
for File_Name in Files:
	#
	#	Save where we are and go to the base directory
	#
	Saved_Directory = os.getcwd()
	os.chdir(Base_Directory)
	#
	#	Add the file
	#
	Zip_File.write(File_Name[len(Base_Directory):])
	#
	#	Go back to the original directory
	#
	os.chdir(Saved_Directory)
#
#	Done
#
Zip_File.close()

