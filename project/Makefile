
CXX = g++
CXXFLAGS = -std=c++17 -I/opt/homebrew/include -I.
LDFLAGS = -L/opt/homebrew/lib -lglfw -lglew -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

TARGET = main
SRC = main.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
