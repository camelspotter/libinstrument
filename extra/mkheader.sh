#!/bin/bash

echo -e '#ifndef _INSTRUMENT' > .build/instrument.hpp
echo -e '#define _INSTRUMENT 1\n' >> .build/instrument.hpp

$(foreach d, $(DOPTS), $(ECHO) '#define $(d)' >> .build/instrument.hpp;)

echo -e '\n#include "instrument/config.hpp"' >> .build/instrument.hpp

# Include library module header files
$(foreach m, $(MODS),																											\
	echo -n '#include "instrument/$(m).hpp"' >> .build/instrument.hpp;								\

echo -n '\n#endif\r\n' >> .build/instrument.hpp
	