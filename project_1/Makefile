CC = g++
LDFLAGS =
BLDDIR = .
INCDIR = $(BLDDIR)/inc
SRCDIR = $(BLDDIR)/src
OBJDIR = $(BLDDIR)/obj
CFLAGS = -c -g -Wall -I$(INCDIR) -std=c++11
SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC))
EXE = bin/bin

all: clean $(EXE) 
    
$(EXE): $(OBJ) 
	$(CC) $(LDFLAGS) $(OBJDIR)/*.o -o $@ 

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
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
