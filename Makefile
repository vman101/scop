CC      := clang
CFLAGS  := -Wall -Wextra -std=c23 -MMD -MP -Iglfw/include
LDFLAGS :=
LDLIBS  := -lvulkan -lm -ldl -lpthread
GLFW_LIB := $(PWD)/glfw/build/src/libglfw3.a

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

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(GLFW_LIB) $(LDFLAGS) $(LDLIBS) -o $@

$(OBJDIR)%.o: $(SRCDIR)%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(DEP)

fclean: clean
	rm -f $(TARGET)

re: fclean all

-include $(DEP)

.PHONY: all clean fclean re
