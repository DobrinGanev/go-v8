#--------Vars-------------
BASEDIR = .
SRCDIR = $(BASEDIR)/src
BINDIR = $(BASEDIR)/bin
OBJDIR = $(BASEDIR)/obj

INCLUDES = -I$(BASEDIR)/includes

#-------OS specific-------
ifeq ($(GOOS),windows)
CXXFLAGS = -shared $(INCLUDES)
LDFLAGS = -lv8 -lstdc++ -lws2_32 -lwinmm
OUTLIB = $(BINDIR)/v8wrap.dll
else
CXXFLAGS = -fPIC -shared $(INCLUDES)
LDFLAGS = -lv8
OUTLIB = $(BINDIR)/libv8wrap.so
endif

# Compiler and options
CXX = g++
#CXXFLAGS = $(INCLUDES)
#LDFLAGS =
#OUTLIB = $(BINDIR)/hoverbot

#----Source files---------
SOURCES = \
	$(SRCDIR)/v8wrap.cc \
	$(SRCDIR)/v8wrap_context.cc \
	$(SRCDIR)/json_util.cc

# Set the build destination to be different than the source
OBJECTS = $(patsubst $(SRCDIR)/%.cc,$(OBJDIR)/%.o,$(SOURCES))

#-----Commands------------
all: checkdirs $(SOURCES) $(OUTLIB)

test: checkdirs $(SOURCES) $(OUTLIB)
	LD_LIBRARY_PATH=$(BINDIR) go test

clean:
	rm -rf $(BINDIR) $(OBJDIR)

rebuild: clean all

#-----Stages--------------
$(BINDIR):
	@mkdir -p $@
$(OBJDIR):
	@mkdir -p $@
checkdirs: $(BINDIR) $(OBJDIR)

$(OUTLIB): $(OBJECTS) 
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)
	go build -x .

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS) 
