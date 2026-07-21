#ifndef CL_TEST_UTILS_SCOPED_ENVELOPE_H
#define CL_TEST_UTILS_SCOPED_ENVELOPE_H
#include "datatype/datatype.h"

DataEnvelope* s_envelope_init(DataEnvelope* data);
void s_envelope_free_all();

#endif
