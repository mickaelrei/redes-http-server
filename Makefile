BIN := ./bin

all: $(BIN) $(BIN)/server $(BIN)/main

$(BIN)/server: server.cpp server.hpp
	$(CXX) -c server.cpp -o $(BIN)/server.o

$(BIN)/main: main.cpp $(BIN)/server.o
	$(CXX) main.cpp $(BIN)/server.o -o $(BIN)/main

$(BIN):
	@if [ ! -d $(BIN) ]; then mkdir $(BIN); fi

clean:
	@if [ -d $(BIN) ]; then rm -rf $(BIN); fi