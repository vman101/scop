GLFW_DIR := external/glfw
GLFW_BUILD := $(GLFW_DIR)/build
GLFW_LIB := $(GLFW_BUILD)/src/libglfw3.a

CC      := clang
CFLAGS  := -Wall -Wextra -std=c23 -MMD -MP -I$(GLFW_DIR)/include -g
LDFLAGS :=
LDLIBS  := -lvulkan -lm -ldl -lpthread

LIBS := $(GLFW_LIB)

UNAME := $(shell uname)

TARGET := scop
SRCDIR := src/
SRC    := $(addprefix $(SRCDIR), main.c \
			vector.c io.c validation_layers.c app.c \
			logical_device.c physical_device.c surface.c \
			swapchain.c graphics_pipeline.c utils.c)
OBJDIR := obj/
OBJ    := $(SRC:$(SRCDIR)%.c=$(OBJDIR)%.o)
DEP    := $(OBJ:.o=.d)

SHADER_DIR := shaders/
SHADER_SRC := $(wildcard $(SHADER_DIR)*.vert $(SHADER_DIR)*.frag $(SHADER_DIR)*.comp)
SHADER_BIN := $(SHADER_SRC:$(SHADER_DIR)%=$(OBJDIR)%.spv)

$(OBJDIR)%.spv: $(SHADER_DIR)%
	@mkdir -p $(dir $@)
	glslc $< -o $@

all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

val: all
	valgrind --show-leak-kinds=all ./$(TARGET)

$(TARGET): $(OBJ) $(LIBS) $(SHADER_BIN)
	mkdir -p $(@D)
	$(CC) $(OBJ) $(LIBS) $(LDFLAGS) $(LDLIBS) -o $@

$(OBJDIR)%.o: $(SRCDIR)%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)%.spv: $(SHADER_SRC)%
	glslc $< -o $@

$(GLFW_LIB):
	cmake -S $(GLFW_DIR) -B $(GLFW_BUILD) -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
	cmake --build $(GLFW_BUILD) -j$(shell nproc)

clean:
	rm -f $(OBJ) $(DEP)

fclean: clean
	rm -f $(TARGET)

re: fclean all

-include $(DEP)

.PHONY: all clean fclean re
