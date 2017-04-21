echo -n "" > gluten.h
echo "#ifndef GLUTEN_AMALGAMATION_H" >> gluten.h
echo "#define GLUTEN_AMALGAMATION_H" >> gluten.h
cat src/gluten/config.h >> gluten.h
echo "#define AMALGAMATION" >> gluten.h
cat src/palloc/palloc.h >> gluten.h
cat src/palloc/vector.h >> gluten.h
cat src/lodepng/lodepng.h >> gluten.h
cat src/gluten/Widget.h >> gluten.h
cat src/gluten/Position.h >> gluten.h
cat src/gluten/Anchor.h >> gluten.h
cat src/gluten/Button.h >> gluten.h
cat src/gluten/Label.h >> gluten.h
cat src/gluten/Container.h >> gluten.h
cat src/gluten/Form.h >> gluten.h
cat src/gluten/gluten.h >> gluten.h
echo "#endif" >> gluten.h

echo -n "" > gluten.c
echo "#include \"gluten.h\"" >> gluten.c
cat src/gluten/Container.c >> gluten.c
cat src/gluten/Widget.c >> gluten.c
cat src/gluten/Button.c >> gluten.c
cat src/gluten/Label.c >> gluten.c
cat src/gluten/Anchor.c >> gluten.c
cat src/gluten/Form.c >> gluten.c
cat src/gluten/Position.c >> gluten.c
cat src/gluten/gluten.c >> gluten.c
cat src/lodepng/lodepng.c >> gluten.c
cat src/palloc/palloc.c >> gluten.c
cat src/palloc/vector.c >> gluten.c

mkdir -p bin

CFLAGS="-g -Wall -pedantic -I/usr/local/include"
LDFLAGS="-L/usr/local/lib -lSDL"

cc -o bin/login_example_amalgamation $CFLAGS -I. *.c src/login_example/*.c $LDFLAGS
cc -o bin/login_example $CFLAGS -Isrc/palloc -Isrc/gluten *.c src/login_example/*.c $LDFLAGS

