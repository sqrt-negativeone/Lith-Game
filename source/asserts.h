/* date = February 21st 2022 5:43 pm */

#ifndef ASSERTS_H
#define ASSERTS_H

#undef Assert
#define AssertStatement HardAssert
#define Assert HardAssert
#define HardAssert(b) do { if(!(b)) { _AssertFailure(#b, __LINE__, __FILE__, 1); } } while(0)
#define SoftAssert(b) do { if(!(b)) { _AssertFailure(#b, __LINE__, __FILE__, 0); } } while(0)


#endif //ASSERTS_H
