#include <string.h>
#include "opengl.h"

bool init_opengl()
{
	glewInit();
	return true;
}
