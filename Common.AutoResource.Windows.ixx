#if USE_CPP_MODULES
    module;
#endif

#include "precomp.h"

#if USE_CPP_MODULES
    import Common.AutoResource;
    export module Common.AutoResource.Windows;
    export
    {
        #include "Common.AutoResource.Windows.h"
    }
#else
    #include "Common.AutoResource.h"
    #include "Common.AutoResource.Windows.h"
#endif
