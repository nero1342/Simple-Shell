# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall

# the build target executable:
TARGET = myshell
run:
	./$(TARGET)
all: $(TARGET)
	@echo  Simple compiler named myshell has been compiled

$(TARGET): $(TARGET).c 
	$(CC) $(CFLAGS) -o $(TARGET) builtin_func.c  $(TARGET).c 

clean:
	-rm $(TARGET)
