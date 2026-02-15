
CXX = g++
OBJCXX = clang++
CXXFLAGS = -std=c++17 -I/opt/homebrew/include -I.
OBJCXXFLAGS = -std=c++17 -I. -fobjc-arc
LDFLAGS = -L/opt/homebrew/lib -lglfw -lglew -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -framework AVFoundation -framework Foundation

TARGET = main
SRC = main.cpp
OBJC_SRC = audio.mm

OBJS = main.o audio.o

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

audio.o: audio.mm
	$(OBJCXX) $(OBJCXXFLAGS) -c audio.mm -o audio.o

clean:
	rm -f $(TARGET) $(OBJS)
