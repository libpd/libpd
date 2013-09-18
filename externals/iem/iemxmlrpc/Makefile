# xmlrpc - XMLRPC external for PD
# 
# Makefile for gcc @ linux
#
# usage:
# to build run "make -f Makefile.linux"
# to install (as root), do "make -f Makefile.linux install"
#
#* Author: Thomas Grill t.grill [at] gmx.net
#
# License LGPL see LICENSE.txt
# IEM - Institute of Electronic Music and Acoustics, Graz
# Inffeldgasse 10/3, 8010 Graz, Austria
# http://iem.at 
#************************************************************/


# Configuration: ##########################


# where are the PD header files?
# leave it blank if it is a system directory (like /usr/local/include), 
# Åsince gcc 3.2 complains about it
PDPATH=

# where should flext libraries be built?
TARGDIR=./pd-linux

# where are the XMLRPC++ header files?
XMLRPCPATH=xmlrpc++
XMLRPCINC=$(XMLRPCPATH)/src/
XMLRPCLIB=libXmlRpc.a

# where should the external be installed?
# (leave blank to omit installation)
INSTPATH=/usr/local/lib/pd/extra

# additional compiler flags
# (check if they fit for your system!)
# UFLAGS=-mcpu=pentiumpro  # gcc 2.95
UFLAGS=-mcpu=pentium3 -msse  # gcc 3.2

###########################################

# compiler+linker stuff	
INCLUDES=$(PDPATH) $(XMLRPCINC)
LIBPATH= ${XMLRPCPATH}

FLAGS=-DPD ${U_FLAGS}
CFLAGS=-O6 
#CFLAGS=-g
LIBS=m util XmlRpc

# ---------------------------------------------
# the rest can stay untouched
# ----------------------------------------------
NAME=iemxmlrpc

# all the source files from the package
DIR=.
SRCS=main.cpp

MAKEFILE=Makefile

TARGET=$(TARGDIR)/$(NAME).pd_linux

# default target
all: $(XMLRPCPATH)/$(XMLRPCLIB) $(TARGDIR) $(TARGET) 

$(XMLRPCPATH)/$(XMLRPCLIB): $(XMLRPCPATH)
	make -C $(XMLRPCPATH)

$(patsubst %,$(DIR)/%,$(SRCS)): $(patsubst %,$(DIR)/%,$(HDRS)) $(MAKEFILE)
	touch $@

$(TARGDIR):
	mkdir -p $(TARGDIR)

$(TARGDIR)/%.o : $(DIR)/%.cpp
	$(CXX) -c $(CFLAGS) $(FLAGS) $(patsubst %,-I%,$(INCLUDES)) $< -o $@

$(TARGET) : $(patsubst %.cpp,$(TARGDIR)/%.o,$(SRCS))
	$(CXX) -shared $^ $(patsubst %,-L%,$(LIBPATH)) $(patsubst %,-l%,$(LIBS)) -o $@ 
	chmod 755 $@

$(INSTPATH):
	mkdir $(INSTPATH)

install:: $(INSTPATH)

install:: $(TARGET)
	cp $^ $(INSTPATH)
	chown root.root  $(patsubst %,$(INSTPATH)/%,$(notdir $^))
	chmod 755 $(patsubst %,$(INSTPATH)/%,$(notdir $^))

.PHONY: clean
clean:
	make -C $(XMLRPCPATH) clean
	rm -f $(TARGDIR)/*.o $(TARGET)
	-rm -f *~
