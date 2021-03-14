CC = gcc
CCXX = g++

LDFLAGS =
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
	$(CCXX) $(LDFLAGS) $(OBJDIR)/*.o -o $@ 

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CCXX) $(CXXFLAGS) $< -o $@

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@


clean:
	-rm -f $(OBJDIR)/*.o $(EXE)

rasp:
	make CC=arm-linux-gnueabihf-g++

send:
	scp -P 23900 ./bin/bin  guilhermeandrade@3.tcp.ngrok.io:/home/guilhermeandrade

remote-test:
	make rasp
	make send
	ssh guilhermeandrade@3.tcp.ngrok.io -p 23900 ./bin

run:
	./$(EXE)