#ifndef STDBOOL_H

#define STDBOOL_H

// Visual studio will complain if we try to do a typedef - it still sees bool as a builtin type
// The C++ standard says sizeof(bool) is implementation defined, Microsoft's implementation
// is 1 byte (see http://msdn.microsoft.com/en-us/library/tf4dy80a.aspx)
#define bool char

#define true ((bool)1)
#define false ((bool)0)

#endif