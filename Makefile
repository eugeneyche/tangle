CC		:= g++
CFLAGS	:= -O2 --std=c++11

SRC_DIR := src
INC_DIR := include
BUILD_DIR := .build

TARGET	:= tangle
LIBS	:= -lGL -lGLEW -lSDL2

SOURCES := $(shell find $(SRC_DIR) -name '*.cpp' -type 'f')
HEADERS := $(shell find $(INC_DIR) -name '*.hpp' -type 'f')
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS) 
