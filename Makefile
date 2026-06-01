CC=gcc
BIN=./bin
CFLAGS=-g -Wall -Wextra -Wshadow -Wconversion -Wunreachable-code
LIBS=-lncursesw

PROG=nave

LIST=$(addprefix $(BIN)/, $(PROG))

.PHONY: all
all: $(LIST)

$(BIN)/%: %.c
	$(CC) -o $@ $< $(CFLAGS) $(LIBS)

%: %.c
	$(CC) -o $(BIN)/$@ $< $(CFLAGS) $(LIBS)

test:
	@./test.sh ||:

run: all
	bin/nave

.PHONY: clean
clean:
	rm -f $(LIST)

zip:
	git archive --format zip --output ${USER}-lab04.zip HEAD

html:
	pandoc -o README.html -f gfm README.md
