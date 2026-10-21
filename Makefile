CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -pthread
BIN = build/sleeping_ta

all: $(BIN)

$(BIN): src/sleeping_ta.c
	mkdir -p build
	$(CC) $(CFLAGS) -o $@ $<

test: all
	bash tests/run_tests.sh
	node tests/test_ui_model.js

tsan:
	mkdir -p build
	$(CC) -Wall -g -O1 -fsanitize=thread -pthread -o build/sleeping_ta_tsan src/sleeping_ta.c
	./build/sleeping_ta_tsan -c 2 -n 6 -p 0.1 -d 0.05 -r 3 -q -o build/tsan.csv && python3 tests/check_log.py build/tsan.csv

clean:
	rm -rf build

.PHONY: all test tsan clean
