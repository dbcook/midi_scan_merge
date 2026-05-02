#pragma once

// this chained pair of macros will stringify any macro value
// the 2 stage chaining is necessary; it won't work in one go
#define xstr(a) str(a)
#define str(a) #a
