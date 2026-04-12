# source ~/personal/utils/emsdk/emsdk_env.sh
# cd /home/mo/personal/learn_colors
# make clean && make web && mv out/* ~/personal/mohammed-ibrahim/public/raylib/learn_colors/
# make desktop && out/learn_colors.out


# Notes:
# The default WASM stack size of 64KB. If you get an error about "out of memory", increase the TOTAL_MEMORY or allocate on the heap.

# Do not need -I. or -L.

# -L. Tells the linker: "Look for libraries in the current directory." It doesn't link a file; it just adds a search path.

# The -L flag must be followed by a directory, not a filename.
# LIBS = -L $(RAYLIB_SRC) -lraylib.web or

# When you provide a direct path to a .a file like this, you are telling the compiler: "Take this specific file and link it directly."
# LIBS = $(RAYLIB_SRC)/libraylib.web


# Change this to your project name
PROJECT_NAME = learn_colors

# Doesn't change
RAYLIB_SRC = /home/mo/personal/utils/raylib/src
RAYLIB_EXAMPLES = /home/mo/personal/utils/raylib/examples
# OUTDIR = /home/mo/personal/mohammed-ibrahim/public/raylib/$(PROJECT_NAME)
OUTDIR = out


CC = emcc
CFLAGS = -Os -Wall -Wno-missing-braces -Wunused-result -std=c99 -D_DEFAULT_SOURCE -DPLATFORM_WEB
INCLUDES = -I. -I $(RAYLIB_SRC) -I $(RAYLIB_EXAMPLES)/others
LIBS = $(RAYLIB_SRC)/libraylib.web.a
LDFLAGS = \
	-s USE_GLFW=3 \
	-s TOTAL_MEMORY=67108864 \
	-s FORCE_FILESYSTEM=1 \
	-s 'EXPORTED_FUNCTIONS=["_free","_malloc","_main"]' \
	-s EXPORTED_RUNTIME_METHODS=ccall \
	--shell-file $(RAYLIB_SRC)/minshell.html \
	--preload-file resources/


all: desktop web

web: $(OUTDIR)/$(PROJECT_NAME).html

$(OUTDIR)/$(PROJECT_NAME).html: $(PROJECT_NAME).c
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $(LDFLAGS) -o $@ $<

desktop: $(PROJECT_NAME).c
	cc $(PROJECT_NAME).c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lcjson -Wall -Wextra -std=c99 -pedantic -g -o out/$(PROJECT_NAME).out

clean:
	rm -f $(OUTDIR)/*
