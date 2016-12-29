all: vapnik

vapnik: *.cpp *.hpp
	g++ *.cpp -o vapnik -O0 -g -fpermissive
