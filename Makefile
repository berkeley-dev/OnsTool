CXXFLAGS = -std=c++11 -Wall -Werror -Wextra

OnsTool: main.o
	g++ -o OnsTool main.o
