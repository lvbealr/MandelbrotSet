CXX        = g++
CFLAGS     = -std=c++20
SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system

BUILD_DIR    = build/
SRC_DIR      = src/
INCLUDE_DIR  = include/

CUSTOM_WARNING_DIR = customWarning/
COLOR_PRINT_DIR    = colorPrint/

CUSTOM_WARNING_INCLUDE = $(CUSTOM_WARNING_DIR)
COLOR_PRINT_INCLUDE    = $(CUSTOM_WARNING_DIR)$(COLOR_PRINT_DIR)
COLOR_PRINT_SRC        = colorPrint.cpp
SUBMODULES             = $(CUSTOM_WARNING_DIR)$(COLOR_PRINT_DIR)$(COLOR_PRINT_SRC)

INCLUDE_FLAGS = -I $(CUSTOM_WARNING_INCLUDE) -I $(COLOR_PRINT_INCLUDE) -I $(INCLUDE_DIR)

DEFAULT_OUTPUT     = $(BUILD_DIR)default/default
OPTIMIZED_OUTPUT   = $(BUILD_DIR)optimized/optimized
TEST_DEF_OUTPUT    = $(BUILD_DIR)defaultTest/defaultTest
TEST_OPT_OUTPUT    = $(BUILD_DIR)optimizedTest/optimizedTest
GPU_OUTPUT         = $(BUILD_DIR)gpu/gpu

DEFAULT_SRC        = mandelbrot.cpp    colorTheme.cpp $(COLOR_PRINT_SRC)
OPTIMIZATION_SRC   = avxMandelbrot.cpp colorTheme.cpp $(COLOR_PRINT_SRC)
TEST_DEF_SRC       = mandelbrot.cpp    colorTheme.cpp $(COLOR_PRINT_SRC)
TEST_OPT_SRC       = avxMandelbrot.cpp colorTheme.cpp $(COLOR_PRINT_SRC)
GPU_SRC            = gpuMandelbrot.cpp                $(COLOR_PRINT_SRC)

DEFAULT_OBJ        = $(addprefix $(BUILD_DIR)default/, 		 $(DEFAULT_SRC:.cpp=.o))
OPTIMIZED_OBJ      = $(addprefix $(BUILD_DIR)optimized/, 	 $(OPTIMIZATION_SRC:.cpp=.o))
TEST_DEF_OBJ       = $(addprefix $(BUILD_DIR)defaultTest/,   $(TEST_DEF_SRC:.cpp=.o))
TEST_OPT_OBJ       = $(addprefix $(BUILD_DIR)optimizedTest/, $(TEST_OPT_SRC:.cpp=.o))
GPU_OBJ            = $(addprefix $(BUILD_DIR)gpu/,			 $(GPU_SRC:.cpp=.o))

DEFAULT_FLAGS      = -O0
OPTIMIZATION_FLAGS = -march=native -O3 -mavx2
TEST_DEF_FLAGS     = -D ON_TEST_ -O0
TEST_OPT_FLAGS     = -D ON_TEST_ -O3 -march=native -mavx2
GPU_FLAGS          = -D ON_GPU_

vpath %.cpp $(SRC_DIR) $(CUSTOM_WARNING_DIR)$(COLOR_PRINT_DIR)

$(BUILD_DIR)default/ $(BUILD_DIR)optimized/ $(BUILD_DIR)defaultTest/ $(BUILD_DIR)optimizedTest/ $(BUILD_DIR)gpu/:
	@mkdir -p $@

$(BUILD_DIR)default/%.o: %.cpp | $(BUILD_DIR)default/
	$(CXX) $(INCLUDE_FLAGS) $(DEFAULT_FLAGS) -c $< -o $@

$(BUILD_DIR)optimized/%.o: %.cpp | $(BUILD_DIR)optimized/
	$(CXX) $(INCLUDE_FLAGS) $(OPTIMIZATION_FLAGS) -c $< -o $@

$(BUILD_DIR)defaultTest/%.o: %.cpp | $(BUILD_DIR)defaultTest/
	$(CXX) $(INCLUDE_FLAGS) $(TEST_DEF_FLAGS) -c $< -o $@

$(BUILD_DIR)optimizedTest/%.o: %.cpp | $(BUILD_DIR)optimizedTest/
	$(CXX) $(INCLUDE_FLAGS) $(TEST_OPT_FLAGS) -c $< -o $@

$(BUILD_DIR)gpu/%.o: %.cpp | $(BUILD_DIR)gpu/
	$(CXX) $(INCLUDE_FLAGS) $(GPU_FLAGS) -c $< -o $@

$(DEFAULT_OUTPUT): $(DEFAULT_OBJ)
	$(CXX) $^ $(CFLAGS) $(SFML_FLAGS) -o $@

$(OPTIMIZED_OUTPUT): $(OPTIMIZED_OBJ)
	$(CXX) $^ $(CFLAGS) $(SFML_FLAGS) -o $@

$(TEST_DEF_OUTPUT): $(TEST_DEF_OBJ)
	$(CXX) $^ $(CFLAGS) $(SFML_FLAGS) -o $@

$(TEST_OPT_OUTPUT): $(TEST_OPT_OBJ)
	$(CXX) $^ $(CFLAGS) $(SFML_FLAGS) -o $@

$(GPU_OUTPUT): $(GPU_OBJ)
	$(CXX) $^ $(SFML_FLAGS) -o $@

ALL_OUTPUTS = $(DEFAULT_OUTPUT) $(OPTIMIZED_OUTPUT) $(TEST_DEF_OUTPUT) $(TEST_OPT_OUTPUT) $(GPU_OUTPUT)

.PHONY: all
all: $(ALL_OUTPUTS)

.PHONY: default
default: $(DEFAULT_OUTPUT)

.PHONY: optimized
optimized: $(OPTIMIZED_OUTPUT)

.PHONY: defaultTest
defaultTest: $(TEST_DEF_OUTPUT)

.PHONY: optimizedTest
optimizedTest: $(TEST_OPT_OUTPUT)

.PHONY: gpu
gpu: $(GPU_OUTPUT)

clean:
	@rm -rf $(BUILD_DIR)