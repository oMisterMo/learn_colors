
# Run on desktop:
# cd ~/personal/learn_colors && make clean && make desktop && out/learn_colors.out

# Deploy to web:
# cd ~/personal/learn_colors && make clean && make web && rm -f ~/personal/mohammed-ibrahim/public/raylib/learn_colors/* && mv out/* ~/personal/mohammed-ibrahim/public/raylib/learn_colors/



# Notes:

# Memory management:
# The default WASM stack size of 64KB. If you get an error about "out of memory", increase the TOTAL_MEMORY or allocate on the heap.
# Can set the initial memory size with: -s INITIAL_MEMORY=X
# assert(typeof Module["TOTAL_MEMORY"] == "undefined", "Module.TOTAL_MEMORY has been renamed Module.INITIAL_MEMORY");
# assert(typeof Module["STACK_SIZE"] == "undefined", "STACK_SIZE can no longer be set at runtime.  Use -sSTACK_SIZE at link time");
# err("Stack overflow detected.  You can try increasing -sSTACK_SIZE (currently set to 65536)");

# Linking:
# Do not need -I. or -L.
# -L. Tells the linker: "Look for libraries in the current directory." It doesn't link a file; it just adds a search path.
# The -L flag must be followed by a directory, not a filename.
# LIBS = -L $(RAYLIB_SRC) -lraylib.web or
# When you provide a direct path to a .a file like this, you are telling the compiler: "Take this specific file and link it directly."
# LIBS = $(RAYLIB_SRC)/libraylib.web
# ld = linker, LDFLAGS = linker flags
# Linker flags:
# -s USE_GLFW=3: Tells the linker to include support for GLFW version
# -s: Strips debugging information from the final executable.
# Wl,-rpath,/path: Sets the directory for shared libraries at runtime.
# -lname: Links a specific library (libname.so or libname.a).
# -L/path/to/library: Tells the linker to look for libraries in a specific directory.
# Compiler flags:
# -Os: Optimize for size.
# -Wall: Enable all compiler warnings.
# -Wno-missing-braces: Suppress warnings about missing braces in initializer lists.
# -Wunused-result: Suppress warnings about unused return values.
# -std=c99: Use the C99 standard for compilation.
# --preload-file: Preloads files into the virtual filesystem of the WebAssembly module.
# --shell-file: Specifies a custom HTML shell file to use as the template for the generated


# Change this to your project name
PROJECT_NAME = learn_colors

# Doesn't change
RAYLIB_SRC = ~/personal/utils/raylib/src
RAYLIB_EXAMPLES = ~/personal/utils/raylib/examples
# OUTDIR = ~/personal/mohammed-ibrahim/public/raylib/$(PROJECT_NAME)
OUTDIR = out


CC = emcc
CFLAGS = -Os -Wall -Wno-missing-braces -Wunused-result -std=c99 \
	-D_DEFAULT_SOURCE \
	-DPLATFORM_WEB \
	--shell-file $(RAYLIB_SRC)/minshell.html \
	--preload-file resources/
INCLUDES = -I. -I $(RAYLIB_SRC) -I $(RAYLIB_EXAMPLES)/others
LIBS = $(RAYLIB_SRC)/libraylib.web.a
LDFLAGS = \
	-s USE_GLFW=3 \
	-s TOTAL_MEMORY=67108864 \
	-s FORCE_FILESYSTEM=1 \
	-s 'EXPORTED_FUNCTIONS=["_free","_malloc","_main"]' \
	-s EXPORTED_RUNTIME_METHODS=ccall \
	--shell-file $(RAYLIB_SRC)/minshell.html


all: desktop web

web: $(OUTDIR)/$(PROJECT_NAME).html

$(OUTDIR)/$(PROJECT_NAME).html: $(PROJECT_NAME).c
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $(LDFLAGS) -o $@ $<

desktop: $(PROJECT_NAME).c
	cc $(PROJECT_NAME).c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lcjson -Wall -Wextra -std=c99 -pedantic -g -o out/$(PROJECT_NAME).out

clean:
	rm -f $(OUTDIR)/*
