#if USE_CPP_MODULES
    module;
#endif

#include "precomp.h"

#if USE_CPP_MODULES
    export module Common.AutoResource.Windows;
    import Common.AutoResource;
    export
    {
        #include "Common.AutoResource.Windows.h"
    }
#else
    #include "Common.AutoResource.h"
    #include "Common.AutoResource.Windows.h"
#endif
