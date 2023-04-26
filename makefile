all:
	gcc -oFast bchess.cpp -o bchess
	x86_64-w64-mingw32-gcc -oFast bchess.cpp -o bchess.exe

debug:
	gcc -oFast bchess.cpp -o bchess
	x86_64-w64-mingw32-gcc bchess.cpp -o bchess.exe