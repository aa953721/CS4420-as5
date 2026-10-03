CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
TARGET = schedule
SOURCE = schedule.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET)

test1:
	./$(TARGET) testcase1.txt FCFS
	./$(TARGET) testcase1.txt SJF
	./$(TARGET) testcase1.txt RR 5

test2:
	./$(TARGET) testcase2.txt FCFS
	./$(TARGET) testcase2.txt SJF
	./$(TARGET) testcase2.txt RR 2

test3:
	./$(TARGET) testcase3.txt FCFS
	./$(TARGET) testcase3.txt SJF
	./$(TARGET) testcase3.txt RR 3

test: test1 test2 test3

clean:
	rm -f $(TARGET)