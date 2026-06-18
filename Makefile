CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall

SRCS := $(shell find . -name "*.cpp")
BINS := $(patsubst ./%.cpp,bin/%,$(SRCS))

.PHONY: all clean run-all list

all: $(BINS)

bin/%: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $< -o $@

# Build and run every demo in sequence, printing a separator between each.
run-all: all
	@for b in $(BINS); do \
		echo "=================================================="; \
		echo "Running: $$b"; \
		echo "=================================================="; \
		./$$b; \
		echo ""; \
	done

list:
	@echo "$(SRCS)" | tr ' ' '\n'

clean:
	rm -rf bin
