all:
	g++ -c -fPIC MS3DFile.cpp -o ms3dfile.o -O3 -lGL -lGLU -lglut -ljpeg -lpng
	g++ -c -fPIC optimizations.cpp -o optimizations.o -O3
	g++ -c -fPIC Shader.cpp -o Shader.o -O3
	g++ -c -fPIC MS3DFileIO.cpp -o MS3DFileIO.o -O3
	g++ -c -fPIC Textures.cpp -o textures.o -O3 -lGL -lGLU -lglut -ljpeg -lpng
	gcc -O3 -shared -Wl,-soname,libms3d.so.1 -o libms3d.so.1.0.1 ms3dfile.o textures.o optimizations.o Shader.o MS3DFileIO.o

optimizer:
	g++ -g -o optimizer optimizer.cpp -lms3d -lGL -lGLU -lglut -ljpeg -lpng

debug:
	g++ -g -c -fPIC MS3DFile.cpp -o ms3dfile.o -lGL -lGLU -lglut -ljpeg -lpng
	g++ -g -c -fPIC Textures.cpp -o textures.o -lGL -lGLU -lglut -ljpeg -lpng
	g++ -g -c -fPIC optimizations.cpp -o optimizations.o 
	g++ -g -c -fPIC Shader.cpp -o Shader.o -O3
	gcc -g -shared -Wl,-soname,libms3d.so.1 -o libms3d.so.1.0.1 ms3dfile.o textures.o optimizations.o Shader.o

install:
	cp libms3d.so.1.0.1 /usr/lib
	ln -s /usr/lib/libms3d.so.1.0.1 /usr/lib/libms3d.so
	ln -s /usr/lib/libms3d.so.1.0.1 /usr/lib/libms3d.so.1

reinstall:
	cp libms3d.so.1.0.1 /usr/lib
	rm /usr/lib/libms3d.so
	ln -s /usr/lib/libms3d.so.1.0.1 /usr/lib/libms3d.so
	rm /usr/lib/libms3d.so.1
	ln -s /usr/lib/libms3d.so.1.0.1 /usr/lib/libms3d.so.1

clean:
	rm textures.o
	rm ms3dfile.o
	rm optimizations.o
	rm Shader.o
	rm libms3d.so.1.0.1
	rm optimizer
