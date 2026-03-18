TARGET := BrokenPianoSimulator
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj

CXX ?= g++
CC ?= cc

GLFW_CFLAGS := $(shell pkg-config --cflags glfw3 2>/dev/null)
GLFW_LIBS := $(shell pkg-config --libs glfw3 2>/dev/null || echo -lglfw)

CPPFLAGS += -Isrc -Ithirdparty/load_obj -Ithirdparty/frog -isystem include -isystem include/glfw/include $(GLFW_CFLAGS)
CXXFLAGS ?= -std=gnu++17 -O2 -Wall -Wextra
CFLAGS ?= -O2 -Wall -Wextra
LDLIBS += $(GLFW_LIBS) -lGL -ldl -lpthread -lm

CPP_SOURCES := $(wildcard src/*.cpp) thirdparty/load_obj/load_obj.cpp
C_SOURCES := include/glad/glad.c
OBJECTS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(CPP_SOURCES)) \
           $(patsubst %.c,$(OBJ_DIR)/%.o,$(C_SOURCES))

.PHONY: all clean prepare-assets

all: prepare-assets $(TARGET)

prepare-assets:
	@if [ ! -f assets/StBartsTheLess02.obj ] && [ -f assets/stBarts.zip ] && command -v unzip >/dev/null 2>&1; then \
		echo "Extracting assets/StBartsTheLess02.obj from assets/stBarts.zip"; \
		unzip -o -j assets/stBarts.zip StBartsTheLess02.obj -d assets >/dev/null; \
	fi

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDLIBS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
