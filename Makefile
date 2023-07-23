PLATFORM = linux64
INC      = /home/juliana/gurobi1002/linux64/include
CPP      = g++
CPPLIB   = -L/home/juliana/gurobi1002/linux64/lib/ -lgurobi_c++ -lgurobi100
CXXFLAGS = -Wall -std=c++11

TARGET = programa

# Lista de arquivos fonte (.cpp)
SOURCES = instance.cpp vrprd.cpp

# Lista de arquivos objetos (.o) gerados a partir dos arquivos fonte
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

# Regra para compilar cada arquivo fonte (.cpp) em um arquivo objeto (.o)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
