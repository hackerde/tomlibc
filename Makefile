LIB =lib
SRC =src
TESTS =tests
CC=gcc

ODIR=obj

_LDEPS = models.h utils.h key.h value.h tokenizer.h
LDEPS = $(patsubst %,$(LIB)/%,$(_LDEPS))

_LOBJ = key.o value.o tokenizer.o 
LOBJ = $(patsubst %,$(ODIR)/%,$(_LOBJ))

_SDEPS = keys.h values.h utils.h toml.h
SDEPS = $(patsubst %,$(SRC)/%,$(_SDEPS))

_SOBJ = keys.o values.o utils.o
SOBJ = $(patsubst %,$(ODIR)/%,$(_SOBJ))

$(ODIR)/%.o: $(LIB)/%.c $(LDEPS)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $<

$(ODIR)/%.o: $(SRC)/%.c $(SDEPS) $(LOBJ)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $<

main: main.c $(LOBJ) $(SOBJ)
	$(CC) -o $@ $^

test: $(TESTS)/test.c $(LOBJ) $(SOBJ)
	$(CC) -o $(TESTS)/$@ $^

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o main $(TESTS)/test
	rm -rf $(ODIR)
