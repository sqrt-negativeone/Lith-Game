
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
