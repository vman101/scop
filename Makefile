CC      := clang
CFLAGS  := -Wall -Wextra -std=c23 -MMD -MP -Iglfw/include
LDFLAGS :=
LDLIBS  := -lvulkan -lm -ldl -lpthread

GLFW_DIR := external/glfw
GLFW_BUILD := $(GLFW_DIR)/build
GLFW_LIB := $(GLFW_BUILD)/src/libglfw3.a

LIBS := $(GLFW_LIB)

UNAME := $(shell uname)

TARGET := scop
SRCDIR := src/
SRC    := $(addprefix $(SRCDIR), main.c)
OBJDIR := obj/
OBJ    := $(SRC:$(SRCDIR)%.c=$(OBJDIR)%.o)
DEP    := $(OBJ:.o=.d)


all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJ) $(LIBS)
	$(CC) $(OBJ) $(LIBS) $(LDFLAGS) $(LDLIBS) -o $@

$(OBJDIR)%.o: $(SRCDIR)%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBS): $(GLFW_LIB)

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
