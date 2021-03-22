CC = gcc
CCXX = g++

LDFLAGS = -lm -lwiringPi -lpthread -lcrypt -lrt -lncurses	
BLDDIR = .

INCDIR = $(BLDDIR)/inc
SRCDIR = $(BLDDIR)/src
OBJDIR = $(BLDDIR)/obj

CXXFLAGS = -c -g -Wall -I$(INCDIR) -std=c++11
CFLAGS = -c -g -Wall -I$(INCDIR) -std=c99

CSRC = $(wildcard $(SRCDIR)/*.c)
CXXSRC = $(wildcard $(SRCDIR)/*.cpp)

CXXOBJ = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(CXXSRC))
COBJ = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(CSRC))

EXE = bin/bin

all: 
	-make clean
	make $(EXE)

$(EXE): $(CXXOBJ) $(COBJ)
	$(CCXX) $(OBJDIR)/*.o -o $@  $(LDFLAGS)

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CCXX) $(CXXFLAGS) $< -o $@

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@


clean:
	-rm -f $(OBJDIR)/*.o $(EXE)

rasp:
	make CC=arm-linux-gnueabihf-gcc CCXX=arm-linux-gnueabihf-g++

clean-remote:
	ssh guilhermeandrade@3.tcp.ngrok.io -p 23900 rm -rf /home/guilhermeandrade/build/
	-ssh guilhermeandrade@3.tcp.ngrok.io -p 23900 mkdir -p /home/guilhermeandrade/build/proj1



send:
	tar -cvzf /tmp/proj1.tar.gz .
	scp -P 23900 /tmp/proj1.tar.gz  guilhermeandrade@3.tcp.ngrok.io:/home/guilhermeandrade/build/ptoj1.tar.gz
	ssh guilhermeandrade@3.tcp.ngrok.io -p 23900 "tar -xvzf /home/guilhermeandrade/build/ptoj1.tar.gz -C /home/guilhermeandrade/build/proj1"

remote-build:
	make clean
	make clean-remote
	make send
	ssh guilhermeandrade@3.tcp.ngrok.io -p 23900 "cd /home/guilhermeandrade/build/proj1; make"

run:
	./$(EXE)