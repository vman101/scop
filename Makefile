GLFW_DIR := external/glfw
GLFW_BUILD := $(GLFW_DIR)/build
GLFW_LIB := $(GLFW_BUILD)/src/libglfw3.a

CC      := clang
CFLAGS  := -Wall -Wextra -std=c23 -MMD -MP -Isrc -I$(GLFW_DIR)/include -g
LDFLAGS :=
LDLIBS  := -lvulkan -lm -ldl -lpthread
SAN_FLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer

LIBS := $(GLFW_LIB)

UNAME := $(shell uname)

TARGET := scop
SRCDIR := src/
CORE := vector.c io.c utils.c
RENDERER := validation_layers.c renderer.c \
			logical_device.c physical_device.c surface.c \
			swapchain.c graphics_pipeline.c \
			render_pass.c shader.c command.c draw.c \
			sync.c framebuffer.c

SRC    := 	$(addprefix $(SRCDIR), main.c) \
			$(addprefix $(SRCDIR)renderer/, $(RENDERER)) \
			$(addprefix $(SRCDIR)core/, $(CORE))

OBJDIR := obj/
OBJ    := $(SRC:$(SRCDIR)%.c=$(OBJDIR)%.o)
DEP    := $(OBJ:.o=.d)

SHADER_DIR := shaders/
SHADER_SRC := $(wildcard $(SHADER_DIR)*.vert $(SHADER_DIR)*.frag $(SHADER_DIR)*.comp)
SPVDIR := obj/$(SHADER_DIR)
SHADER_BIN := $(SHADER_SRC:$(SHADER_DIR)%=$(SPVDIR)%.spv)

all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

val: all
	valgrind --show-leak-kinds=all ./$(TARGET)

$(SPVDIR)%.spv: $(SHADER_DIR)%
	@mkdir -p $(@D)
	glslc $< -o $@

$(OBJDIR)%.o: $(SRCDIR)%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ) $(LIBS) $(SHADER_BIN)
	$(CC) $(OBJ) $(LIBS) $(LDFLAGS) $(LDLIBS) -o $@

$(GLFW_LIB):
	cmake -S $(GLFW_DIR) -B $(GLFW_BUILD) -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
	cmake --build $(GLFW_BUILD) -j$(shell nproc)

san:
	$(MAKE) TARGET=$(TARGET)-san OBJDIR=obj-san/ \
		CFLAGS="$(CFLAGS) $(SAN_FLAGS)" LDFLAGS="$(LDFLAGS) $(SAN_FLAGS)"

run-san: san
	LSAN_OPTIONS=suppressions=lsan.supp ASAN_OPTIONS=detect_leaks=1 ./$(TARGET)-san

clean:
	rm -rf obj/ obj-san/

fclean: clean
	rm -f $(TARGET) $(TARGET)-san

re: fclean all

-include $(DEP)

.PHONY: all run val san run-san clean fclean re
