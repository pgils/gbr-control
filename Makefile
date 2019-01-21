CXX		= g++
RM		= rm -f
MD		= mkdir -p
CPPFLAGS= -Iinclude -DNDEBUG
LDLIBS	= -ltinyxml2 \
		  -lsqlite3  \
		  -lpthread

ODIR	= obj
_OBJ	= gbrcontrol.o			\
		  gbrsocketlistener.o	\
		  gbrxml.o				\
		  gbrdatabasehandler.o
OBJ		= $(patsubst %,$(ODIR)/%,$(_OBJ))

.PHONY: $(ODIR) clean distclean

all: $(ODIR) gbrcontrol

$(ODIR):
	$(MD) $(ODIR)

obj/%.o: src/%.cpp
	$(CXX) -c -o $@ $< $(CPPFLAGS)

gbrcontrol: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDLIBS)

clean:
	$(RM) -r $(ODIR)

distclean: clean
	$(RM) gbrcontrol

install: all
	install -m 755 gbrcontrol /usr/local/sbin/
	install -m 644 extra/gbr-control.service /etc/systemd/system/
	install -m 755 extra/gbr-commission /usr/local/sbin/
	install -m 644 extra/gbr-commission.service /etc/systemd/system/
	install -m 644 extra/gbr-commission.timer /etc/systemd/system/
