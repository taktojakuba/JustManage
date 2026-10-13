flags = -march=native -flto -DNDEBUG
build: 
	g++ -o jm main.cpp $(flags)
