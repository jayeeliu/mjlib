#ifndef __MJCO_H
#define __MJCO_H

#define CoBegin(state) switch (state) { case 0:
#define CoReturnValue(state, x) do { state = __LINE__; return x; case __LINE__:; } while(0)
#define CoReturnVoid(state) do { state = __LINE__; return; case __LINE__:; } while(0)
#define CoFinishValue(state, x) state = 0; return x
#define CoFinishVoid(state) state = 0; return
#define CoEnd }

#endif
