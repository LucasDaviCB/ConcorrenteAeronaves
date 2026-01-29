CC = gcc
CFLAGS = -Wall -Wextra -pthread -O2 -g
INCDIR = include
SRCDIR = src
OBJ = $(SRCDIR)/main.o $(SRCDIR)/aeronave.o $(SRCDIR)/controle.o $(SRCDIR)/setor.o

all: atc

$(SRCDIR)/main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(SRCDIR)/aeronave.o: $(SRCDIR)/aeronave.c $(INCDIR)/aeronave.h $(INCDIR)/controle.h
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(SRCDIR)/controle.o: $(SRCDIR)/controle.c $(INCDIR)/controle.h $(INCDIR)/setor.h $(INCDIR)/aeronave.h
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(SRCDIR)/setor.o: $(SRCDIR)/setor.c $(INCDIR)/setor.h
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

atc: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

clean:
	rm -f $(SRCDIR)/*.o atc
