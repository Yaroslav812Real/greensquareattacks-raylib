all:
	g++ main.cpp button.cpp -o main.exe -I include/ -L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows
	./main.exe