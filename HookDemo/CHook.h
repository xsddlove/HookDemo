#pragma once
#include <windows.h>

void SetSehHook();

//�쳣������
LONG NTAPI ExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo);


