LIB =parser/lib
SRC =parser
TESTS =tests
CC=gcc

ODIR=obj

_LDEPS = models.h utils.h key.h value.h tokenizer.h
LDEPS = $(patsubst %,$(LIB)/%,$(_LDEPS))

_LOBJ = key.o value.o tokenizer.o 
LOBJ = $(patsubst %,$(ODIR)/%,$(_LOBJ))

_SDEPS = parse_keys.h parse_values.h parse_utils.h
SDEPS = $(patsubst %,$(SRC)/%,$(_SDEPS))

_SOBJ = parse_keys.o parse_values.o parse_utils.o
SOBJ = $(patsubst %,$(ODIR)/%,$(_SOBJ))

all: main test

$(ODIR)/%.o: $(LIB)/%.c $(LDEPS)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $<

$(ODIR)/%.o: $(SRC)/%.c $(SDEPS) $(LOBJ)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $<

$(ODIR)/tomlib.o: tomlib.c tomlib.h $(LOBJ) $(SOBJ)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $<

main: main.c $(ODIR)/tomlib.o $(LOBJ) $(SOBJ)
	$(CC) -o $@ $^

test: $(TESTS)/test.c $(ODIR)/tomlib.o $(LOBJ) $(SOBJ)
	$(CC) -o $(TESTS)/$@ $^

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o main $(TESTS)/test
	rm -rf $(ODIR)
