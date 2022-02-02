// Copyright Andreas Franck - All rights reserved

#include "kiss_fftr_double_redefines.h"

// As the real-to-complex transform uses the complex transform, we need to include the new names as well.
#include "kiss_fft_double_redefines.h"

#include "tools/kiss_fftr.c"
