all:
	g++ -Ofast bchess.cpp -o bchess 
	x86_64-w64-mingw32-g++ -Ofast bchess.cpp -o Knightfall.exe

debug:
	g++ -Ofast -g bchess.cpp -o bchess 
	x86_64-w64-mingw32-g++ -g bchess.cpp -o Knightfall.exe 