#include <windows.h>
#include"m_Dialog.h"
void SetSehHook();

//异常处理函数
LONG NTAPI ExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo);


