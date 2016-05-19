PROGRAM := fpga_tester_linux1

okFP_SDK ?= ./

CPPFLAGS := -I$(okFP_SDK)
LDFLAGS := -L$(okFP_SDK)
LIBS := -ldl

okFP_LIBS := -lokFrontPanel

OBJECTS := \
	fpga_tester_linux1.o

.SUFFIXES: .o .cpp

all: $(PROGRAM)

$(OBJECTS): %.o:%.cpp
	$(CXX) $(CPPFLAGS) $(okFP_CXXFLAGS) $(CXXFLAGS) -o $@ -c $<

$(PROGRAM): $(OBJECTS)
	$(CXX) $(okFP_LDFLAGS) $(LDFLAGS) -o $(PROGRAM) $(OBJECTS) $(okFP_LIBS) $(LIBS)

clean:
	rm -f *.o $(PROGRAM)