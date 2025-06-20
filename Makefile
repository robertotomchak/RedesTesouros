# PROGRAMA
    PROG = main
    OBJS = main.o mensagem.o gerenciador.o servidor.o cliente.o caca_tesouro.o utils.o

# Compilador
    CC   = gcc

# Flag de bitfield é por causa de uma chatisse com bitfield na struct do protocolo
    CFLAGS = -Wall -Werror -Wno-packed-bitfield-compat

# Lista de arquivos para distribuição. Acrescentar mais arquivos se necessário.
DISTFILES = *.c *.h LEIAME* Makefile
DISTDIR = `basename ${PWD}`

.PHONY: all clean purge dist

%.o: %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $< -g

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *~ *.bak

purge:  clean
	@rm -f $(PROG) $(OBJS) core a.out $(DISTDIR) $(DISTDIR).tgz *.txt *.jpg *.mp4

dist: purge
	@ln -s . $(DISTDIR)
	@tar -cvf $(DISTDIR).tgz $(addprefix ./$(DISTDIR)/, $(DISTFILES))
	@rm -f $(DISTDIR)
