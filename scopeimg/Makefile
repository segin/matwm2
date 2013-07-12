all: capture plt2ps plt2ppm

OBJS_PS = hpgl.o postscript.o io.o
OBJS_PPM = hpgl.o bitmap.o io.o

capture: capture.o

plt2ps: $(OBJS_PS)
	$(CC) $(LDFLAGS) $(OBJS_PS) -o $@

bitmap.o:
	$(CC) -c $(CFLAGS) `pkg-config --cflags freetype2` bitmap.c -o $@

plt2ppm: $(OBJS_PPM)
	$(CC) $(LDFLAGS) `pkg-config --libs freetype2` $(OBJS_PPM) -o $@

clean:
	rm -rf *.o capture plt2ps
