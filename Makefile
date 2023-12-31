PLATFORM = linux64
INC      = /home/juliana/gurobi1002/linux64/include
CPPLIB   = -L/home/juliana/gurobi1002/linux64/lib/ -lgurobi_c++ -lgurobi100

all: modelo

run: run_modelo

modelo: /home/juliana/Documents/lab/gurobi/vrprd.cpp
	g++ -m64 -g /home/juliana/Documents/lab/gurobi/vrprd.cpp -o modelo -I$(INC) $(CPPLIB) -lpthread -lm

run_modelo: modelo
	./modelo
