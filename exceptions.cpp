// 2. Since some veh hooks are not detectable; they are migrated their anticheats to check exception codes. Some sections banning 0xc00001d, 0x80000001 and more.
// You can check it quickly with a breakpoint and writing that address.
// (https://docs.microsoft.com/en-us/win...texceptioncode)

// Bypassing:

auto CALLBACK VectoredExceptionHandler(_EXCEPTION_POINTERS* ep) -> LONG
{
    if (ep->ExceptionRecord->ExceptionCode == 0xc00001d || ep->ExceptionRecord->ExceptionCode == 0x80000001 ) {
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    else {
        return EXCEPTION_CONTINUE_SEARCH;
    }
}
