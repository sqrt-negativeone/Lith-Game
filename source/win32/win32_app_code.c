
typedef struct W32_AppCode W32_AppCode;
struct W32_AppCode
{
    ApplicationPermanentLoadCallback  *PermanentLoad;
    ApplicationHotLoadCallback        *HotLoad;
    ApplicationHotUnloadCallback      *HotUnload;
    ApplicationUpdateCallback         *UpdateAndRender;
    
    HMODULE dll;
    FILETIME last_dll_write_time;
};

internal void
W32_AppCodeSetStubs(W32_AppCode *app_code)
{
    app_code->PermanentLoad = ApplicationPermanentLoadStub;
    app_code->HotLoad = ApplicationHotLoadStub;
    app_code->HotUnload = ApplicationHotUnloadStub;
    app_code->UpdateAndRender = ApplicationUpdateStub;
    
}

internal b32
W32_AppCodeLoad(W32_AppCode *app_code)
{
    b32 result = 1;
    if (!CopyFile(global_app_dll_path, global_temp_app_dll_path, FALSE))
    {
        result = 0;
        DWORD err = GetLastError();
        goto end;
    }
    app_code->dll = LoadLibraryA(global_temp_app_dll_path);
    
    app_code->last_dll_write_time = W32_GetLastWriteTime(global_app_dll_path);
    
    if(!app_code->dll)
    {
        result = 0;
        goto end;
    }
    
    app_code->PermanentLoad    = (ApplicationPermanentLoadCallback *)GetProcAddress(app_code->dll, "PermanentLoad");
    app_code->HotLoad          = (ApplicationHotLoadCallback *)GetProcAddress(app_code->dll, "HotLoad");
    app_code->HotUnload        = (ApplicationHotUnloadCallback *)GetProcAddress(app_code->dll, "HotUnload");
    app_code->UpdateAndRender           = (ApplicationUpdateCallback *)GetProcAddress(app_code->dll, "UpdateAndRender");
    
    
    if(!app_code->PermanentLoad || !app_code->HotLoad || !app_code->HotUnload || !app_code->UpdateAndRender)
    {
        W32_AppCodeSetStubs(app_code);
        result = 0;
        goto end;
    }
    
    end:;
    return result;
}

internal void
W32_AppCodeUnload(W32_AppCode *app_code)
{
    if(app_code->dll)
    {
        FreeLibrary(app_code->dll);
    }
    app_code->dll = 0;
    W32_AppCodeSetStubs(app_code);
}

internal void
W32_AppCodeUpdate(W32_AppCode *app_code)
{
    FILETIME last_write_time = W32_GetLastWriteTime(global_app_dll_path);
    if(CompareFileTime(&last_write_time, &app_code->last_dll_write_time))
    {
        app_code->HotUnload();
        W32_AppCodeUnload(app_code);
        W32_AppCodeLoad(app_code);
        app_code->HotLoad(&global_os);
    }
}
