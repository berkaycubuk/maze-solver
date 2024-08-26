build:
	eval cc main.c $(shell pkg-config --libs --cflags raylib) -o MazeSolver

run: build
	./MazeSolver
