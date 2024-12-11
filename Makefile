BIN := ./bin
INCLUDE := ./include
SRC := ./src

all: $(BIN) $(BIN)/http_util.o $(BIN)/server.o $(BIN)/proxy_server.o $(BIN)/main

$(BIN)/http_util.o: $(SRC)/http_util.cpp $(INCLUDE)/http_util.hpp
	$(CXX) -I$(INCLUDE) -c $(SRC)/http_util.cpp -o $(BIN)/http_util.o

$(BIN)/server.o: $(SRC)/server.cpp $(INCLUDE)/server.hpp
	$(CXX) -I$(INCLUDE) -c $(SRC)/server.cpp -o $(BIN)/server.o

$(BIN)/proxy_server.o: $(SRC)/proxy_server.cpp $(INCLUDE)/proxy_server.hpp
	$(CXX) -I$(INCLUDE) -c $(SRC)/proxy_server.cpp -o $(BIN)/proxy_server.o

$(BIN)/main: main.cpp $(BIN)/server.o
	$(CXX) -I$(INCLUDE) main.cpp $(BIN)/server.o $(BIN)/proxy_server.o $(BIN)/http_util.o -o $(BIN)/main

$(BIN):
	@if [ ! -d $(BIN) ]; then mkdir $(BIN); fi

clean:
	@if [ -d $(BIN) ]; then rm -rf $(BIN); fi