#include <windows.h>
#include"m_Dialog.h"
void SetSehHook();

//�쳣������
LONG NTAPI ExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo);


