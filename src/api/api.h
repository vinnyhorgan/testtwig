#ifndef API_H
#define API_H

#include "lib/minilua/minilua.h"

#define API_TYPE_IMAGE "Image"

void api_load_libs(lua_State *L);

#endif
