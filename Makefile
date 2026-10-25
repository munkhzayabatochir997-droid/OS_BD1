CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -pthread
BIN = build/sleeping_ta
CHK = build/check_log

all: $(BIN) $(CHK)

$(BIN): src/sleeping_ta.c
	mkdir -p build
	$(CC) $(CFLAGS) -o $@ $<

$(CHK): tests/check_log.c
	mkdir -p build
	$(CC) -Wall -Wextra -O2 -o $@ $<

test: all
	bash tests/run_tests.sh

stress: all
	bash tests/stress.sh 30

tsan: $(CHK)
	$(CC) -Wall -g -O1 -fsanitize=thread -pthread -o build/sleeping_ta_tsan src/sleeping_ta.c
	./build/sleeping_ta_tsan -c 2 -n 6 -p 0.1 -d 0.05 -r 3 -q -o build/tsan.csv && $(CHK) build/tsan.csv

test-ui:
	node tests/test_ui_model.js

clean:
	rm -rf build

.PHONY: all test stress tsan test-ui clean
