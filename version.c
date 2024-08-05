
#include <stdio.h>
#include "config.h"

static char *EVPath_version = "EVPath Version 4.5.2 -- Mon Aug  5 15:33:46 EDT 2024\n";

#if defined (__INTEL_COMPILER)
//  Allow extern declarations with no prior decl
#  pragma warning (disable: 1418)
#endif
void EVprint_version()
{
    printf("%s",EVPath_version);
}
void EVfprint_version(FILE*out)
{
    fprintf(out, "%s",EVPath_version);
}

