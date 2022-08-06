
typedef struct W32_AppCode W32_AppCode;
struct W32_AppCode
{
    APP_PermanentLoad_Function   *PermanentLoad;
    APP_HotUnload_Function       *HotUnload;
    APP_UpdateAndRender_Function *UpdateAndRender;
    APP_HotLoad_Function         *HotLoad;
    APP_HotLoadShader_Function   *HotLoadShader;
    
    HMODULE dll;
    FILETIME last_dll_write_time;
};

internal void
W32_AppCodeSetStubs(W32_AppCode *app_code)
{
    app_code->PermanentLoad   = APP_PermanentLoadStub;
    app_code->HotUnload       = APP_HotUnloadStub;
    app_code->UpdateAndRender = APP_UpdateAndRenderStub;
    app_code->HotLoad         = APP_HotLoadStub;
    app_code->HotLoadShader   = APP_HotLoadShaderStub;
}

internal b32
W32_AppCodeLoad(W32_AppCode *app_code)
{
    b32 result = 1;
    if (!CopyFile(w32_app_dll_path, w32_temp_app_dll_path, FALSE))
    {
        result = 0;
        GetLastError();
        goto end;
    }
    app_code->dll = LoadLibraryA(w32_temp_app_dll_path);
    
    app_code->last_dll_write_time = W32_GetLastWriteTime(w32_app_dll_path);
    
    if(!app_code->dll)
    {
        result = 0;
        goto end;
    }
    
    app_code->PermanentLoad    = (APP_PermanentLoad_Function *)  GetProcAddress(app_code->dll, "PermanentLoad");
    app_code->HotUnload        = (APP_HotUnload_Function *)      GetProcAddress(app_code->dll, "HotUnload");
    app_code->UpdateAndRender  = (APP_UpdateAndRender_Function *)GetProcAddress(app_code->dll, "UpdateAndRender");
    app_code->HotLoad          = (APP_HotLoad_Function *)        GetProcAddress(app_code->dll, "HotLoad");
    app_code->HotLoadShader    = (APP_HotLoadShader_Function *)  GetProcAddress(app_code->dll, "HotLoadShader");
    
    
    if(!app_code->PermanentLoad || !app_code->HotUnload || !app_code->HotLoadShader || !app_code->HotLoad || !app_code->UpdateAndRender)
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
    FILETIME last_write_time = W32_GetLastWriteTime(w32_app_dll_path);
    if(CompareFileTime(&last_write_time, &app_code->last_dll_write_time))
    {
        app_code->HotUnload();
        W32_AppCodeUnload(app_code);
        W32_AppCodeLoad(app_code);
        app_code->HotLoad(os);
    }
}
