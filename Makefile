SRCS = $(wildcard *.cpp) $(wildcard adapt/*.cpp)
OBJDIR=bin
EXE = $(OBJDIR)/acached
LIBS := -levent -ljemalloc -lpthread
CLEANCC=-Wno-write-strings -Wno-format -Wno-unused -Wno-unused-variable
CFLAGS:=-g3 -O0 -Wall -fmessage-length=0 $(CLEANCC)  
CPPFLAGS = -I/usr/local/include -Iadapt
LDFLAGS:=-levent
CC = g++ 
OBJS = $(addprefix $(OBJDIR)/,  $(patsubst %.cpp,%.o,$(SRCS)))

all: 	$(EXE)

$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: %.cpp
	$(CC) $(CPPFLAGS) $(CFLAGS)  -c -o $@  $<

clean: 
	rm -f $(OBJS) $(EXE)

#g++ -I/usr/local/include -I"/home/arun/git/adapt" -O0 -g3 -Wall -std=gnu99 -pthread -MT  -MD -MP -MF -Wno-format -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
