CXX = g++
CXXFLAGS = -std=c++11 -I/opt/homebrew/include -I./src
LDFLAGS = -L/opt/homebrew/lib -lglfw -lglew -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

TARGET = main
SRC = src/main.cpp src/camera.cpp src/shader.cpp src/lighting.cpp src/material.cpp src/bus_model.cpp src/viewport.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
