#if USE_CPP_MODULES
    module;
#endif

#include "precomp.h"

#if USE_CPP_MODULES
    export module Common.AutoResource;
    export
    {
        #include "Common.AutoResource.h"
    }
#else
    #include "Common.AutoResource.h"
#endif
