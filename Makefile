vpath %.c ../src

CC = gcc
CFLAGS = -c -Wall -std=gnu99
LDFLAGS =

DSTDIR := /usr/local
DISTDIR := dist
OBJDIR := obj
SRCDIR := src

SRC = power-profile-change.c
OBJ = $(patsubst %.c,$(OBJDIR)/%.o,$(SRC)) 

PACKAGE_DIR := "/dev/shm/checkinstall_install" 

TARGET = bin/power-profile-change

# Maybe later
#CFLAGS += `pkg-config --cflags appindicator3-0.1`
#LDFLAGS += `pkg-config --libs appindicator3-0.1`
TOPDIR=$(shell pwd)

all: $(TARGET)

packages-intended: 
	mkdir -p "$(PACKAGE_DIR)" 
	mount --bind "$(TOPDIR)"  "$(PACKAGE_DIR)" 

	mkdir -p $(DISTDIR)
# Have to type 'GPL' to be marked as open-source, for some reason
	(    \
		cd /dev/shm/checkinstall_install && \
		checkinstall -D \
		-d2 \
		--install=no \
		--pkgname 'power-profile-change' \
		--pkglicense 'MIT' \
		--pkggroup '' \
		--pkgsource 'https://github.com/brunoais/tuxedo-control-center-profile-changer' \
		--pkgversion '0.5.0' \
		--pakdir $(DISTDIR) \
		--maintainer 'brunoais' \
		--requires 'tuxedo-control-center' \
		--nodoc \
		--bk \
		make install  \
	)
	
	umount "$(PACKAGE_DIR)" 
	rmdir "$(PACKAGE_DIR)"

packages:

		checkinstall -D \
		-d2 \
		--install=no \
		--pkgname 'power-profile-change' \
		--pkglicense 'GPL' \
		--pkgversion '0.5.0' \
		--pakdir $(DISTDIR) \
		--maintainer 'brunoais' \
		--bk \
		make install  


install: $(TARGET)
	@sudo chown -R root ${DSTDIR}/bin/
	@echo Install to ${DSTDIR}/bin/
	@sudo install -m 4750 -g adm $(TARGET) ${DSTDIR}/bin/

test: $(TARGET)
	@sudo chown root $(TARGET)
	@sudo chgrp adm  $(TARGET)
	@sudo chmod 4750 $(TARGET)

$(TARGET): $(OBJ) Makefile
	@mkdir -p bin
	@echo linking $(TARGET) from $(OBJ)
	@$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS) -lm
	chmod +x $(TARGET)

clean:
	rm $(OBJ) $(TARGET)

$(OBJDIR)/%.o : $(SRCDIR)/%.c Makefile
	@echo compiling $< 
	@mkdir -p obj
	@$(CC) $(CFLAGS) -c $< -o $@

#$(OBJECTS): | obj

#obj:
#	@mkdir -p $@
