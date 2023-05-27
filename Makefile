install_dependencies:
	sudo apt-get install -y python3-dev
	sudo apt-get install -y libfuse3-dev
	sudo apt-get install python3-pyotp
	sudo apt install libpam-google-authenticator
	sudo apt install python3-qrcode 

PYTHON_FLAGS = $(shell python3-config --embed --cflags --ldflags)
FUSE_FLAGS = $(shell pkg-config fuse3 --cflags --libs)

build: 
	gcc -Wall hello.c $(FUSE_FLAGS) $(PYTHON_FLAGS) -o hello

#run: build
#	./hello /test2

clean:
	rm hello
