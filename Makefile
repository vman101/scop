GLFW_DIR       := external/glfw
GLFW_BUILD     := $(GLFW_DIR)/build
GLFW_LIB       := $(GLFW_BUILD)/src/libglfw3.a

CC             := clang
CFLAGS         := -Wall -Wextra -std=c99 -MMD -MP -Isrc -I$(GLFW_DIR)/include -Iexternal -Ilib -g -D_DEFAULT_SOURCE
LDFLAGS        :=
LDLIBS         := -lvulkan -lm -ldl -lpthread
SAN_FLAGS      := -fsanitize=address,undefined -fno-omit-frame-pointer

LIBS           := $(GLFW_LIB)

UNAME          := $(shell uname)

LIB_DIR        := lib/
TARGET         := scop
SRCDIR         := src/
UTILS          := da.c utils.c sv.c
VK             := validation_layers.c logical_device.c \
					swapchain.c physical_device.c memory.c \
					render_pass.c shader.c command.c \
					sync.c framebuffer.c buffer.c
MESH           := mesh.c
MODEL          := model.c
MATERIAL       := material.c
GEOMETRY       := geometry.c
LOADER         := loader.c

PLATFORM_GLFW  := platform_glfw.c
PLATFORM_LINUX := platform_linux.c

ASSET          := asset_mtl.c asset_obj.c asset_debug.c asset_util.c asset_ppm.c

GFX_VULKAN     := gfx_vulkan.c $(addprefix gfx_vulkan_, \
			  	buffer.c swapchain.c pipeline.c \
				memory.c platform.c draw.c \
				descriptors.c texture.c)

LIB_SRC        := $(addprefix $(LIB_DIR)utils/, $(UTILS)) \
					$(addprefix $(LIB_DIR)asset/, $(ASSET)) \
					$(addprefix $(LIB_DIR)mesh/, $(MESH)) \
					$(addprefix $(LIB_DIR)model/, $(MODEL)) \
					$(addprefix $(LIB_DIR)material/, $(MATERIAL)) \
					$(addprefix $(LIB_DIR)geometry/, $(GEOMETRY)) \
					$(addprefix $(LIB_DIR)loader/, $(LOADER)) \
					$(addprefix $(LIB_DIR)platform/glfw/, $(PLATFORM_GLFW)) \
					$(addprefix $(LIB_DIR)platform/linux/, $(PLATFORM_LINUX)) \
					$(addprefix $(LIB_DIR)gfx/vulkan/, $(GFX_VULKAN)) \
					$(addprefix $(LIB_DIR)gfx/vulkan/vk/, $(VK))

APP_SRC        := $(addprefix $(SRCDIR), main.c scop.c)
ifeq ($(TARGET), scop-san)
ASAN_SRC       := lib/utils/san_nodlclose.c
endif

OBJDIR         := obj/
LIB_OBJ        := $(LIB_SRC:%.c=$(OBJDIR)%.o)
APP_OBJ        := $(APP_SRC:%.c=$(OBJDIR)%.o)
LIB            := $(OBJDIR)libscop.a
DEP            := $(LIB_OBJ:.o=.d) $(APP_OBJ:.o=.d)


SHADER_DIR     := shaders/
SHADER_SRC     := $(wildcard $(SHADER_DIR)*.vert $(SHADER_DIR)*.frag $(SHADER_DIR)*.comp)
SPVDIR         := obj/$(SHADER_DIR)
SHADER_BIN     := $(SHADER_SRC:$(SHADER_DIR)%=$(SPVDIR)%.spv)

all: $(TARGET)

$(TARGET): $(APP_OBJ) $(LIB) $(SHADER_BIN) $(GLFW_LIB)
	$(CC) $(CFLAGS) $(ASAN_SRC) $(APP_OBJ) $(LIB) $(LIBS) $(LDLIBS) $(LDFLAGS) -o $@

run: $(TARGET)
	./$(TARGET) $(ARGS)

val: all
	valgrind --show-leak-kinds=all ./$(TARGET)

lib: $(LIB)

$(LIB): $(LIB_OBJ)
	ar rcs $@ $^

$(SPVDIR)%.spv: $(SHADER_DIR)%
	@mkdir -p $(@D)
	glslc $< -o $@

$(OBJDIR)%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(GLFW_LIB):
	cmake -S $(GLFW_DIR) -B $(GLFW_BUILD) -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
	cmake --build $(GLFW_BUILD) -j$(shell nproc)

san:
	$(MAKE) TARGET=$(TARGET)-san OBJDIR=obj-san/ \
		CFLAGS="$(CFLAGS) $(SAN_FLAGS)" LDFLAGS="$(LDFLAGS) $(SAN_FLAGS)"

run-san: san
	LSAN_OPTIONS=suppressions=lsan.supp ASAN_OPTIONS=detect_leaks=1,fast_unwind_on_malloc=0,max_alloc_fill_size=0 ./$(TARGET)-san $(ARGS)

clean:
	rm -rf obj/ obj-san/

fclean: clean
	rm -rf $(TARGET) $(TARGET)-san $(GLFW_BUILD)

re: fclean all

-include $(DEP)

.PHONY: all run val san run-san clean fclean re
