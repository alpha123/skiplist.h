CFLAGS=-DDEBUG -g

SL_HEADER=skiplist.h
SRCS=test/test_skiplist.c test/ptest.c
OBJS=$(SRCS:.c=.o)
TEST_OUT=test_skiplist

all: build test

build: $(SL_HEADER) $(SRCS) $(TEST_OUT)

.PHONY: test
test:
	./$(TEST_OUT)


DOC_DEFS=-DSKIPLIST_KEY='void *' -DSKIPLIST_VALUE='void *'

.PHONY: doc
doc:
	cldoc generate $(DOC_DEFS) -- --output doc/ $(SL_HEADER)

$(TEST_OUT): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm test/*.o $(TEST_OUT)
