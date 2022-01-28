
void
_AssertFailure(char *expression, int line, char *file, int crash)
{
    LogError("[Assertion Failure] Assertion of %s at %s:%i failed.", expression, file, line);
    if(crash)
    {
        os->OutputError("Assertion Failure", "Assertion of %s at %s:%i failed. Trying to crash...",
                        expression, file, line);
        BreakDebugger();
        *(volatile int *)0 = 0;
    }
}

void
_DebugLog(i32 flags, char *file, int line, char *format, ...)
{
    // NOTE(rjf): Log to stdout
    {
        char *name = "Info";
        if(flags & Log_Error)
        {
            name = "Error";
        }
        else if(flags & Log_Warning)
        {
            name = "Warning";
        }
        fprintf(stdout, "%s (%s:%i) ", name, file, line);
        va_list args;
        va_start(args, format);
        vfprintf(stdout, format, args);
        va_end(args);
        fprintf(stdout, "%s", "\n");
    }
    
    // NOTE(rjf): Log to VS output, etc.
    {
        local_persist char string[4096] = {0};
        va_list args;
        va_start(args, format);
        vsnprintf(string, sizeof(string), format, args);
        va_end(args);
#if BUILD_WIN32
        OutputDebugStringA(string);
        OutputDebugStringA("\n");
#endif
    }
    
}

void
_DebugBreak_Internal_(void)
{
#if _MSC_VER
    __debugbreak();
#else
    *(volatile int *)0 = 0;
#endif
}
