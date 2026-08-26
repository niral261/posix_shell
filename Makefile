
# The name of exe
TARGET_EXEC := shell

# Directories
BUILD_DIR := ./build
SRC_DIRS := ./src

# Compiler Settings
CXX = g++
CXXLAGS = -Wall -Wextra -g 

# Find all .cpp files under src
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')

# Prepare BUILD_DIR and appends .o to every src
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# String substitution for dependency file
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_DIRS += ./include

# Add -I prefix for GCC
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD AND -MP flags generate the .d dependency files
CPPFLAGS := $(INC_FLAGS) -MMD -MP

# The final build step.
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean	

clean:
	rm -rf $(BUILD_DIR) 

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)