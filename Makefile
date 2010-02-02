# Makefile for Yuma entire project
#  
#   very top directory

##################### LIBRARIES ########################
#
# DIRS:  
# This is an ordered list
# Put only the directories that build object files in the list

# make all targets
DIRS = libtecla netconf libtoaster
S_DIRS = netconf libtoaster
C_DIRS = libtecla netconf
L_DIRS = netconf

ifdef DESTDIR
JFLAG= 
else
JFLAG = -j3
endif

#################### PLATFORM DEFINITIONS ############

all: yumaall

################################ MAKE RULES #################################
#

clean: yumaclean

superclean: yumasuperclean

install: yumainstall

ifdef CLIENT
yumaall: yuma-client

yumainstall: yuma-client-install
else
ifdef SERVER
yumaall: yuma-server

yumainstall: yuma-server-install
else
ifdef SHLIBS
yumaall: yuma-shlibs

yumainstall: yuma-shlibs-install
else
ifdef DEVELOPER
yumaall: yuma-dev

yumainstall: yuma-dev-install
else
yumaall: yuma-all

yumainstall: yuma-all-install
endif
endif
endif
endif


yuma-shlibs:
	for dir in $(L_DIRS); do\
	  cd $$dir && $(MAKE) $(JFLAG) && cd ..;\
        done

yuma-client:
	cd libtecla && ./configure;
	for dir in $(C_DIRS); do\
	  cd $$dir && $(MAKE) $(JFLAG) && cd ..;\
        done

yuma-server:
	for dir in $(S_DIRS); do\
	  cd $$dir && $(MAKE) $(JFLAG) && cd ..;\
        done


yuma-all:
	cd libtecla && ./configure; 
	for dir in $(DIRS); do\
	  cd $$dir && $(MAKE) $(JFLAG) && cd ..;\
        done


yumaclean:
	for dir in $(DIRS); do\
	  cd $$dir && $(MAKE) clean && cd ..;\
        done

yumasuperclean:
	for dir in $(DIRS); do\
	  cd $$dir && $(MAKE) superclean && cd ..;\
        done

yuma-shlibs-install:
	for dir in $(L_DIRS); do\
          cd $$dir && $(MAKE) install && cd ..;\
        done

yuma-client-install:
	for dir in $(C_DIRS); do\
          cd $$dir && $(MAKE) install && cd ..;\
        done

yuma-server-install:
	for dir in $(S_DIRS); do\
          cd $$dir && $(MAKE) install && cd ..;\
        done


yuma-dev-install:
	for dir in $(DIRS); do\
          cd $$dir && $(MAKE) install && cd ..;\
        done

yuma-all-install:
	for dir in $(DIRS); do\
          cd $$dir && $(MAKE) install && cd ..;\
        done

.PHONY: all clean superclean install \
	yumaall yumaclean yumasuperclean yumainstall \
	yuma-client yuma-server yuma-dev yuma-all \
	yuma-client-install yuma-server-install \
	yuma-dev-install yuma-all-install \
	yuma-shlibs yuma-shlibs-install


# prevent the make program from choking on all the symbols
# that get generated from autogenerated make rules
.NOEXPORT:



