
PWD := $(shell pwd)
CXX := g++
LINKER := g++
INCDIRS := -I. -I$(PWD)/lib -I$(PWD)/lib/protobuf-2.6.1/src 
LIBDIRS := $(PWD)/lib/protobuf-2.6.1/src/.libs 
LIBS := -l protobuf 
CXXFLAGS := -std=c++11 -pthread -g #-Wall -Wextra 
BUILD_DIR=build
BIN := $(BUILD_DIR)/bin

PROTO := $(shell bash -c "pwd")/lib/protobuf-2.6.1/src/protoc
MODEL_DIR := model
MODEL_BUILD_DIR := build
BIN := $(BUILD_DIR)/bin
MODELS := $(shell bash -c "cd $(MODEL_DIR) && ls *.proto")
MODELS_SRCFILES := $(patsubst %.proto, %.pb.cc, $(MODELS))
MODELS_OBJFILES := $(addprefix $(MODEL_DIR)/$(MODEL_BUILD_DIR)/, $(patsubst %.cc,%.o,$(MODELS_SRCFILES)))
INCDIRS += -I$(MODEL_DIR)/$(MODEL_BUILD_DIR)

SERVER_DIR := server
SERVER_SRCFILES := $(wildcard $(SERVER_DIR)/*.cpp) 
SERVER_OBJFILES := $(addprefix $(BIN)/, $(patsubst %.cpp,%.o,$(SERVER_SRCFILES)))
SERVER_BUILD := $(BUILD_DIR)/server

CLIENT_DIR := client
QMAKE := qmake

LIB_DIR := lib
LIB_SRCFILES := $(wildcard $(LIB_DIR)/*.cpp) 
LIB_OBJFILES := $(addprefix $(BIN)/, $(patsubst %.cpp,%.o,$(LIB_SRCFILES)))

.PHONY: init_dir clean client model 

all: init_dir model $(SERVER_BUILD) 

run:
	@export LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:$(LIBDIRS) && ./build/server

run_client: client
	@export LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:$(LIBDIRS) && ./build/client

init_dir: 
	@mkdir -p $(BIN)/$(SERVER_DIR) $(BIN)/$(MODEL_DIR) $(MODEL_DIR)/$(MODEL_BUILD_DIR) $(BIN)/$(LIB_DIR) 

model: $(addprefix, $(MODEL_DIR)/, $(MODELS))
	@cd $(MODEL_DIR) && \
	for model in $(MODELS); do echo proto compiling $$model; \
	$(PROTO) -I=. --cpp_out=./$(MODEL_BUILD_DIR) $$model; done  && \
	cd $(MODEL_BUILD_DIR) && \
	for modelcc in $(MODELS_SRCFILES); do echo compiling $$modelcc; $(CXX) $(CXXFLAGS) $(INCDIRS) -c $$modelcc ; done

$(SERVER_BUILD): $(LIB_OBJFILES) $(MODELS_OBJFILES) $(SERVER_OBJFILES) 
	$(LINKER) $^ -L $(LIBDIRS) $(LIBS) -o $@


$(BIN)/$(SERVER_DIR)/%.o: $(SERVER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCDIRS) -c $< -o $@

$(BIN)/$(LIB_DIR)/%.o: $(LIB_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCDIRS) -c $< -o $@

client:
	@cd $(CLIENT_DIR) && $(QMAKE) && $(MAKE) 

clean:
	rm -rf $(BUILD_DIR) $(MODEL_DIR)/$(MODEL_BUILD_DIR)/*
