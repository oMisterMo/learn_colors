# source ~/personal/utils/emsdk/emsdk_env.sh

# Change this to your project name
PROJECT_NAME = learn_colors

# Doesn't change
RAYLIB_SRC = /home/mo/personal/utils/raylib/src
RAYLIB_EXAMPLES = /home/mo/personal/utils/raylib/examples
# OUTDIR = ../../public/raylib/$(PROJECT_NAME)
OUTDIR = out


CC = emcc
CFLAGS = -Os -Wall -Wno-missing-braces -Wunused-result -std=c99 -D_DEFAULT_SOURCE -DPLATFORM_WEB
INCLUDES = -I. -I $(RAYLIB_SRC) -I $(RAYLIB_EXAMPLES)/others
LIBS = -L. $(RAYLIB_SRC)/libraylib.web.a
LDFLAGS = \
	-s USE_GLFW=3 \
	-s TOTAL_MEMORY=67108864 \
	-s FORCE_FILESYSTEM=1 \
	-s 'EXPORTED_FUNCTIONS=["_free","_malloc","_main"]' \
	-s EXPORTED_RUNTIME_METHODS=ccall \
	--shell-file $(RAYLIB_SRC)/minshell.html \
	--preload-file resources/


all: $(OUTDIR)/$(PROJECT_NAME).html

$(OUTDIR)/$(PROJECT_NAME).html: $(PROJECT_NAME).c
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $(LDFLAGS) -o $@ $<

desktop: $(PROJECT_NAME).c
	cc $(PROJECT_NAME).c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lcjson -Wall -Wextra -std=c99 -pedantic -g -o out/$(PROJECT_NAME).out && ./out/$(PROJECT_NAME).out

clean:
	rm -f $(OUTDIR)/*
