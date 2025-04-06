CXX        = g++
CFLAGS     = -std=c++20
SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system

TEST 			   = 300
OPTIMIZATION_LEVEL = -O0
MULTITHREADING 	  ?=

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
ARRAY_OUTPUT       = $(BUILD_DIR)arrayOptimized/arrayOptimized
AVX_OUTPUT   	   = $(BUILD_DIR)avxOptimized/avxOptimized
TEST_DEF_OUTPUT    = $(BUILD_DIR)defaultTest/defaultTest
TEST_ARR_OUTPUT    = $(BUILD_DIR)arrayOptimizedTest/arrayOptimizedTest
TEST_AVX_OUTPUT    = $(BUILD_DIR)avxOptimizedTest/avxOptimizedTest
GPU_OUTPUT         = $(BUILD_DIR)gpu/gpu

DEFAULT_SRC        = mandelbrot.cpp      colorTheme.cpp $(COLOR_PRINT_SRC)
ARRAY_SRC          = arrayMandelbrot.cpp colorTheme.cpp $(COLOR_PRINT_SRC)
AVX_SRC   		   = avxMandelbrot.cpp   colorTheme.cpp $(COLOR_PRINT_SRC)
TEST_DEF_SRC       = mandelbrot.cpp      colorTheme.cpp $(COLOR_PRINT_SRC)
TEST_ARR_SRC       = arrayMandelbrot.cpp colorTheme.cpp $(COLOR_PRINT_SRC)
TEST_AVX_SRC       = avxMandelbrot.cpp   colorTheme.cpp $(COLOR_PRINT_SRC)
GPU_SRC            = gpuMandelbrot.cpp                  $(COLOR_PRINT_SRC)

DEFAULT_OBJ        = $(addprefix $(BUILD_DIR)default/, 		 	  $(DEFAULT_SRC:.cpp=.o))
ARRAY_OBJ		   = $(addprefix $(BUILD_DIR)arrayOptimized/,	  $(ARRAY_SRC:.cpp=.o))
AVX_OBJ		       = $(addprefix $(BUILD_DIR)avxOptimized/,  	  $(AVX_SRC:.cpp=.o))
TEST_DEF_OBJ       = $(addprefix $(BUILD_DIR)defaultTest/,   	  $(TEST_DEF_SRC:.cpp=.o))
TEST_ARR_OBJ       = $(addprefix $(BUILD_DIR)arrayOptimizedTest/, $(TEST_ARR_SRC:.cpp=.o))
TEST_AVX_OBJ       = $(addprefix $(BUILD_DIR)avxOptimizedTest/,   $(TEST_AVX_SRC:.cpp=.o))
GPU_OBJ            = $(addprefix $(BUILD_DIR)gpu/,			 	  $(GPU_SRC:.cpp=.o))

DEFAULT_FLAGS      = $(OPTIMIZATION_LEVEL) -D $(MULTITHREADING) -D TEST=$(TEST) 
ARRAY_FLAGS        = $(OPTIMIZATION_LEVEL) -D $(MULTITHREADING) -D TEST=$(TEST) -march=native -mavx2
AVX_FLAGS 		   = $(OPTIMIZATION_LEVEL) -D $(MULTITHREADING) -D TEST=$(TEST) -march=native -mavx2
TEST_DEF_FLAGS     = $(OPTIMIZATION_LEVEL) -D $(MULTITHREADING) -D TEST=$(TEST) -D ON_TEST_
TEST_ARR_FLAGS     = $(OPTIMIZATION_LEVEL) -D $(MULTITHREADING) -D TEST=$(TEST) -march=native -mavx2 -D ON_TEST_
TEST_AVX_FLAGS     = $(OPTIMIZATION_LEVEL) -D $(MULTITHREADING) -D TEST=$(TEST) -march=native -mavx2 -D ON_TEST_
GPU_FLAGS          = $(OPTIMIZATION_LEVEL) -D $(MULTITHREADING) -D TEST=$(TEST) -D ON_GPU_

vpath %.cpp $(SRC_DIR) $(CUSTOM_WARNING_DIR)$(COLOR_PRINT_DIR)

$(BUILD_DIR)default/ $(BUILD_DIR)arrayOptimized/ $(BUILD_DIR)avxOptimized/ $(BUILD_DIR)defaultTest/ $(BUILD_DIR)arrayOptimizedTest/ $(BUILD_DIR)avxOptimizedTest/ $(BUILD_DIR)gpu/:
	@mkdir -p $@

$(BUILD_DIR)default/%.o: %.cpp | $(BUILD_DIR)default/
	$(CXX) $(INCLUDE_FLAGS) $(DEFAULT_FLAGS) -c $< -o $@

$(BUILD_DIR)arrayOptimized/%.o: %.cpp | $(BUILD_DIR)arrayOptimized/
	$(CXX) $(INCLUDE_FLAGS) $(ARRAY_FLAGS) -c $< -o $@

$(BUILD_DIR)avxOptimized/%.o: %.cpp | $(BUILD_DIR)avxOptimized/
	$(CXX) $(INCLUDE_FLAGS) $(AVX_FLAGS) -c $< -o $@

$(BUILD_DIR)defaultTest/%.o: %.cpp | $(BUILD_DIR)defaultTest/
	$(CXX) $(INCLUDE_FLAGS) $(TEST_DEF_FLAGS) -c $< -o $@

$(BUILD_DIR)arrayOptimizedTest/%.o: %.cpp | $(BUILD_DIR)arrayOptimizedTest/
	$(CXX) $(INCLUDE_FLAGS) $(TEST_ARR_FLAGS) -c $< -o $@

$(BUILD_DIR)avxOptimizedTest/%.o: %.cpp | $(BUILD_DIR)avxOptimizedTest/
	$(CXX) $(INCLUDE_FLAGS) $(TEST_AVX_FLAGS) -c $< -o $@

$(BUILD_DIR)gpu/%.o: %.cpp | $(BUILD_DIR)gpu/
	$(CXX) $(INCLUDE_FLAGS) $(GPU_FLAGS) -c $< -o $@

$(DEFAULT_OUTPUT): $(DEFAULT_OBJ)
	$(CXX) $^ $(CFLAGS) $(SFML_FLAGS) -o $@

$(ARRAY_OUTPUT): $(ARRAY_OBJ)
	$(CXX) $^ $(CFLAGS) $(SFML_FLAGS) -o $@

$(AVX_OUTPUT): $(AVX_OBJ)
	$(CXX) $^ $(CFLAGS) $(SFML_FLAGS) -o $@

$(TEST_DEF_OUTPUT): $(TEST_DEF_OBJ)
	$(CXX) $^ $(CFLAGS) $(SFML_FLAGS) -o $@

$(TEST_ARR_OUTPUT): $(TEST_ARR_OBJ)
	$(CXX) $^ $(CFLAGS) $(SFML_FLAGS) -o $@

$(TEST_AVX_OUTPUT): $(TEST_AVX_OBJ)
	$(CXX) $^ $(CFLAGS) $(SFML_FLAGS) -o $@

$(GPU_OUTPUT): $(GPU_OBJ)
	$(CXX) $^ $(SFML_FLAGS) -o $@

ALL_OUTPUTS = $(DEFAULT_OUTPUT) $(ARRAY_OUTPUT) $(AVX_OUTPUT) $(TEST_DEF_OUTPUT) $(TEST_ARR_OUTPUT) $(TEST_AVX_OUTPUT) $(GPU_OUTPUT)

.PHONY: all
all: $(ALL_OUTPUTS)

.PHONY: default
default: $(DEFAULT_OUTPUT)

.PHONY: arrayOptimized
arrayOptimized: $(ARRAY_OUTPUT)

.PHONY: avxOptimized
avxOptimized: $(AVX_OUTPUT)

.PHONY: defaultTest
defaultTest: $(TEST_DEF_OUTPUT)

.PHONY: arrayOptimizedTest
arrayOptimizedTest: $(TEST_ARR_OUTPUT)

.PHONY: avxOptimizedTest
avxOptimizedTest: $(TEST_AVX_OUTPUT)

.PHONY: gpu
gpu: $(GPU_OUTPUT)

clean:
	@rm -rf $(BUILD_DIR)