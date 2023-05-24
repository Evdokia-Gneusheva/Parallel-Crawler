CC = g++
CFLAGS = -std=c++11 -Wall

TARGET = main
SRCS = main.cpp

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

.PHONY: clean
clean:
	del $(TARGET).exe