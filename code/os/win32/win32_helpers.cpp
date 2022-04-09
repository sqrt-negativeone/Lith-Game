
//- NOTE(fakhri): Library Functions
internal HMODULE
W32_LibraryOpen(String8 path)
{
    M_Temp scratch = GetScratch(0, 0);
    String16 path16 = Str16From8(scratch.arena, path);
    HMODULE hmodule = LoadLibraryW((WCHAR*)path16.str);
    ReleaseScratch(scratch);
    return hmodule;
}

internal void
W32_LibraryClose(HMODULE hmodule)
{
    FreeLibrary(hmodule);
}

internal VoidFunction *
W32_LibraryLoadFunction(HMODULE hmodule, String8 name)
{
    M_Temp scratch = GetScratch(0, 0);
    String8 name_copy = PushStr8Copy(scratch.arena, name);
    VoidFunction *result = (VoidFunction *)GetProcAddress(hmodule, (char *)name_copy.str);
    ReleaseScratch(scratch);
    return result;
}

// NOTE(fakhri): fullscreen
internal void
W32_ToggleFullscreen(HWND hwnd)
{
    local_persist WINDOWPLACEMENT last_window_placement = {
        sizeof(last_window_placement)
    };
    
    DWORD window_style = GetWindowLong(hwnd, GWL_STYLE);
    if(window_style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO monitor_info = { sizeof(monitor_info) };
        if(GetWindowPlacement(hwnd, &last_window_placement) &&
           GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY),
                          &monitor_info))
        {
            
            SetWindowLong(hwnd, GWL_STYLE,
                          window_style & ~WS_OVERLAPPEDWINDOW);
            
            SetWindowPos(hwnd, HWND_TOP,
                         monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right -
                         monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom -
                         monitor_info.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(hwnd, GWL_STYLE,
                      window_style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hwnd, &last_window_placement);
        SetWindowPos(hwnd, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}


internal OS_Modifiers
W32_GetModifiers(void)
{
    OS_Modifiers modifiers = 0;
    if(GetKeyState(VK_CONTROL) & 0x8000)
    {
        modifiers |= OS_Modifier_Ctrl;
    }
    if(GetKeyState(VK_SHIFT) & 0x8000)
    {
        modifiers |= OS_Modifier_Shift;
    }
    if(GetKeyState(VK_MENU) & 0x8000)
    {
        modifiers |= OS_Modifier_Alt;
    }
    return modifiers;
}

//- NOTE(fakhri): file helpers
internal FILETIME
W32_GetLastWriteTime(char *filename)
{
    FILETIME last_write_time = {0};
    WIN32_FIND_DATA find_data;
    HANDLE find_handle = FindFirstFileA(filename, &find_data);
    if(find_handle != INVALID_HANDLE_VALUE)
    {
        FindClose(find_handle);
        last_write_time = find_data.ftLastWriteTime;
    }
    return last_write_time;
}

internal void
W32_ReadWholeBlock(HANDLE file, void *data, U64 data_len)
{
    U8 *ptr = (U8*)data;
    U8 *opl = ptr + data_len;
    for (;;){
        U64 unread = (U64)(opl - ptr);
        DWORD to_read = (DWORD)(ClampTop(unread, U32Max));
        DWORD did_read = 0;
        if (!ReadFile(file, ptr, to_read, &did_read, 0))
        {
            break;
        }
        ptr += did_read;
        if (ptr >= opl)
        {
            break;
        }
    }
}

internal void
W32_WriteWholeBlock(HANDLE file, String8List data)
{
    for(String8Node *node = data.first; node != 0; node = node->next)
    {
        U8 *ptr = node->string.str;
        U8 *opl = ptr + node->string.size;
        for(;;)
        {
            U64 unwritten = (U64)(opl - ptr);
            DWORD to_write = (DWORD)(ClampTop(unwritten, U32Max));
            DWORD did_write = 0;
            if(!WriteFile(file, ptr, to_write, &did_write, 0))
            {
                goto fail_out;
            }
            ptr += did_write;
            if(ptr >= opl)
            {
                break;
            }
        }
    }
    fail_out:;
}
