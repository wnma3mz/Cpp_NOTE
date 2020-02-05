OBJS   = output
SRCS   = main.cpp
CC     = g++
CFLAGS = -std=c++11 -g -Wall -pthread
RM     = rm
$(OBJS):$(SRCS)
	$(CC) $(CFLAGS) -o $(OBJS) $(SRCS)

clean:
	$(RM) $(OBJS)