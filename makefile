CC = g++
CFLAGS = -std=c++11 -Wall -I "C:/curl/curl-8.1.2_2-win64-mingw/include"
LDFLAGS = -L "C:/curl/curl-8.1.2_2-win64-mingw/lib" -lcurl

TARGET = main
SRCS = main.cpp

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

.PHONY: clean
clean:
	del $(TARGET).exe