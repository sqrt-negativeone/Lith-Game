
global HDC   w32_device_context;
global HGLRC w32_gl_ctx;

PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
PFNWGLMAKECONTEXTCURRENTARBPROC wglMakeContextCurrentARB;
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

global int w32_pf_attribs_i[] = 
{
    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
    WGL_COLOR_BITS_ARB, 24,
    WGL_DEPTH_BITS_ARB, 24,
    WGL_STENCIL_BITS_ARB, 8,
    WGL_SAMPLE_BUFFERS_ARB, 1,
    WGL_SAMPLES_ARB, 4,
    0
};

global int w32_pixel_format = 0;
global PIXELFORMATDESCRIPTOR w32_pfd = {sizeof(w32_pfd)};

internal VoidFunction *
W32_GetOpenGLProcAddress(char *name)
{
    VoidFunction *p = (VoidFunction *)wglGetProcAddress(name);
    if(!p || p == (VoidFunction*)0x1 || p == (VoidFunction*)0x2 || p == (VoidFunction*)0x3 || p == (VoidFunction*)-1)
    {
        p = 0;
    }
    return p;
}

internal void
W32_LoadWGLFunctions()
{
    wglChoosePixelFormatARB    = (PFNWGLCHOOSEPIXELFORMATARBPROC)    W32_GetOpenGLProcAddress("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) W32_GetOpenGLProcAddress("wglCreateContextAttribsARB");
    wglMakeContextCurrentARB   = (PFNWGLMAKECONTEXTCURRENTARBPROC)   W32_GetOpenGLProcAddress("wglMakeContextCurrentARB");
    wglSwapIntervalEXT         = (PFNWGLSWAPINTERVALEXTPROC)         W32_GetOpenGLProcAddress("wglSwapIntervalEXT");
}

internal void
W32_InitOpenGL(HINSTANCE h_instance)
{
    //- NOTE(fakhri): make global invisible window
    HWND dummy_hwnd = CreateWindowW(L"STATIC",
                                    L"",
                                    WS_OVERLAPPEDWINDOW,
                                    CW_USEDEFAULT, CW_USEDEFAULT,
                                    100, 100,
                                    0, 0,
                                    h_instance,
                                    0);
    HDC dummy_hdc = GetDC(dummy_hwnd);
    
    //- NOTE(fakhri): make dummy context
    HGLRC gl_dummy_render_context = 0;
    int dummy_pixel_format = 0;
    {
        PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof(PIXELFORMATDESCRIPTOR),
            1,
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
            PFD_TYPE_RGBA,
            32,
            0, 0, 0, 0, 0, 0,
            0,
            0,
            0,
            0, 0, 0, 0,
            24,
            8,
            0,
            PFD_MAIN_PLANE,
            0,
            0, 0, 0
        };
        dummy_pixel_format = ChoosePixelFormat(dummy_hdc, &pfd);
        if(dummy_pixel_format == 0)
        {
            goto end;
        }
        SetPixelFormat(dummy_hdc, dummy_pixel_format, &pfd);
        gl_dummy_render_context = wglCreateContext(dummy_hdc);
        wglMakeCurrent(dummy_hdc, gl_dummy_render_context);
        W32_LoadWGLFunctions();
        wglMakeCurrent(0, 0);
        wglDeleteContext(gl_dummy_render_context);
    }
    
    //- NOTE(fakhri): setup real pixel format
    {
        UINT num_formats = 0;
        wglChoosePixelFormatARB(w32_device_context, w32_pf_attribs_i, 0, 1, &w32_pixel_format, &num_formats);
        DescribePixelFormat(w32_device_context, w32_pixel_format, sizeof(w32_pfd), &w32_pfd);
        SetPixelFormat(w32_device_context, w32_pixel_format, &w32_pfd);
    }
    
    //- NOTE(fakhri): initialize real context
    {
        const int context_attribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 6,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_DEBUG_BIT_ARB,
            0
        };
        w32_gl_ctx = wglCreateContextAttribsARB(w32_device_context, 0, context_attribs);
        if(w32_gl_ctx)
        {
            BOOL make_curr_good = wglMakeCurrent(w32_device_context, w32_gl_ctx);
            wglSwapIntervalEXT(1);
            
            SetPixelFormat(w32_device_context, w32_pixel_format, &w32_pfd);
        }
    }
    
    end:;
    ReleaseDC(dummy_hwnd, dummy_hdc);
    DestroyWindow(dummy_hwnd);
}

internal void
W32_CleanUpOpenGL(HDC *device_context)
{
    wglMakeCurrent(*device_context, 0);
    wglDeleteContext(w32_gl_ctx);
}

internal void
W32_OpenGLRefreshScreen(void)
{
    SwapBuffers(w32_device_context);
}