import os
import random
import sys
import glob

cat = lambda f : [print(l, end='') for l in open(f)]		
rm = lambda path : [os.remove(f) for f in glob.glob(path)]
TMP = os.environ['TMP']

def execute(cmd):
	print(cmd)
	return os.system(cmd)

def compile(compiler, cfiles, target="exe", includes=["."], output_folder="."):
	code = 0
	rand = random.randint(1, 9999999)
	PDB_NAME='%s/ROSE_SYMBOLS_%d.pdb' % (TMP, rand)


	EXTRA_C_FILES = " ".join(["./" + D for D in cfiles])

	CPP_FILE=C_FILES[-1]
	OUTPUT_FOLDER=sys.argv[2]

	INCLUDES=" ".join(["/I " + I for I in includes])

	# Faster builds: https://devblogs.microsoft.com/cppblog/recommendations-to-speed-c-builds-in-visual-studio/
	print(f'compiling {CPP_FILE}')
	#CL /nologo /MP /O1 /std:c++17 /wd"4530" /LD /MD /I third_party/maths /I ../../include /Fe%OUTPUT_FOLDER% %CPP_FILE% source\roseimpl.cpp
	#TODO: check target == exe or dll
	error = execute(f'{compiler} /nologo /MP /std:c++17 /wd"4530" /Zi {INCLUDES} /Fe{OUTPUT_FOLDER} {EXTRA_C_FILES} /link /incremental /PDB:"{PDB_NAME}" > {TMP}/clout.txt')

	if error:
		code = 1
		print("~~~~~~~~~~~")
		print("~~ ERROR ~~")
		print("~~~~~~~~~~~")
		cat(TMP + "/clout.txt")
		print("")
	else:
		print(f"                             ~~ OK ~~")

	rm('*.obj')
	rm('*.idb')
	rm('*.pdb')
	rm(OUTPUT_FOLDER + '/*.lib')
	rm(OUTPUT_FOLDER + '/*.exp')
	rm(OUTPUT_FOLDER + '/*.ilk')

	return code

if __name__ == "__main__":

	INCLUDE_ARRAY = [
		".", "../include"
	]

	C_FILES = [
		"game.cpp"
	]

	compile("CL", includes=INCLUDE_ARRAY, cfiles=C_FILES)	
