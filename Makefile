CC = clang
CPPFLAGS = -MMD -MP
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g -O0

TARGET = build/maze
SOURCES = main.c config.c maze.c generator.c robot.c solver.c render.c
CORE = maze.c generator.c robot.c solver.c
OBJECTS = $(patsubst %.c,build/%.o,$(SOURCES))

.PHONY: all run draw animate check-graphics test sanitize package clean
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

build/%.o: %.c Makefile | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

run: $(TARGET)
	./$(TARGET) $(ARGS)

check-graphics:
	@test -f graphics.h -a -f graphics.c || { echo 'Add the Moodle graphics.h and graphics.c files to this directory first.' >&2; exit 1; }

draw: check-graphics build/maze-draw

build/maze-draw: $(SOURCES) $(wildcard *.h) graphics.c Makefile | build
	$(CC) $(CFLAGS) -DMAZE_DRAWAPP $(SOURCES) graphics.c $(LDFLAGS) $(LDLIBS) -o build/maze-draw

animate: draw
	./run.sh $(ARGS)

build/test-core: tests/test_core.c $(CORE) $(wildcard *.h) | build
	$(CC) $(CFLAGS) -Werror -I. tests/test_core.c $(CORE) -o $@

test: $(TARGET) build/test-core
	./build/test-core mazes/*.txt
	python3 tests/test_cli.py ./$(TARGET)
	python3 tests/check_solver_interface.py
	@if test -f graphics.c -a -f graphics.h; then \
		python3 tests/test_drawapp.py && \
		$(MAKE) --no-print-directory draw && \
		python3 tests/test_launcher.py; \
	fi

sanitize: | build
	$(CC) $(CFLAGS) -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -I. tests/test_core.c $(CORE) -o build/test-sanitize
	./build/test-sanitize mazes/*.txt
	$(CC) $(CFLAGS) -Werror -fsanitize=address,undefined -fno-omit-frame-pointer $(SOURCES) -o build/maze-sanitize
	python3 tests/test_cli.py ./build/maze-sanitize
	@if test -f graphics.c -a -f graphics.h; then \
		CC='$(CC) -fsanitize=address,undefined -fno-omit-frame-pointer' python3 tests/test_drawapp.py; \
	fi

package:
	python3 tools/package.py
	python3 tests/test_package.py

clean:
	rm -f $(TARGET) $(OBJECTS) $(OBJECTS:.o=.d) build/maze-draw build/test-core build/test-sanitize build/maze-sanitize

-include $(OBJECTS:.o=.d)
