#if USE_CPP_MODULES
    module;
#endif

#include "precomp.h"

#if USE_CPP_MODULES
    export module Common.OptionalValue;
    export
    {
        #include "Common.OptionalValue.h"
    }
#else
    #include "Common.OptionalValue.h"
#endif
