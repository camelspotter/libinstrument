#!/bin/bash

DOPTS=_REENTRANT
DOPTS+=WITH_COLOR_ASSERTIONS
DOPTS+=WITH_COLOR_TERM
DOPTS+=WITH_DEBUG
DOPTS+=WITH_FILTER
DOPTS+=WITH_HIGHLIGHT
DOPTS+=WITH_PLUGIN
DOPTS+=WITH_STREAM
DOPTS+=WITH_UNRESOLVED
		
echo -e '#ifndef _INSTRUMENT' > .build/instrument.hpp
echo -e '#define _INSTRUMENT 1\n' >> .build/instrument.hpp

for(def in $(DOPTS))
	echo -e '#define $(def)' >> .build/instrument.hpp

echo -e '\n#include "instrument/config.hpp"' >> .build/instrument.hpp

#$(foreach m, $(MODS),
#	echo -n '#include "instrument/$(m).hpp"' >> .build/instrument.hpp;	

echo -n '\n#endif\r\n' >> .build/instrument.hpp
	