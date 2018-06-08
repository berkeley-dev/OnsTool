CXXFLAGS = -std=c++11 -Wall -Werror -Wextra -Wpedantic

OnsTool: main.o
	g++ -o OnsTool main.o
