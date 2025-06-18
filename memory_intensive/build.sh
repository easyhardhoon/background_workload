g++ -std=c++11 -c -o dynamic_workload.o dynamic_workload.cc
g++ -std=c++11 -c -o main.o main.cpp
g++ -o dynamic_workload main.o dynamic_workload.o -lEGL -lGLESv2 -lpthread



