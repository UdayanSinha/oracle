CC := ${CC}
CFLAGS := ${CFLAGS}
LDFLAGS := ${LDFLAGS}

# source files
MAIN_SRC := main.c
FUNC_SRC := example-func.c

# object files
MAIN_OBJ := main.o
FUNC_OBJ := example-func.o

TARGET := example-make

all: ${TARGET}

${TARGET}: ${MAIN_OBJ} ${FUNC_OBJ}
	${CC} ${CFLAGS} ${LDFLAGS} $^ -o $@

${MAIN_OBJ}: ${MAIN_SRC}
	${CC} ${CFLAGS} -c $< -o $@

${FUNC_OBJ}: ${FUNC_SRC}
	${CC} ${CFLAGS} -c $< -o $@

install:
	install -d ${DESTDIR}${BINDIR}
	install -m 0755 ${TARGET} ${DESTDIR}${BINDIR}

clean:
	rm -f ${MAIN_OBJ} ${FUNC_OBJ} ${TARGET}
