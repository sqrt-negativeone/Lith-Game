
#define PLATFORM_LAYER 1

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define _WINSOCK_DEPRECATED_NO_WARNINGS
////////////////////////////////
//~ NOTE(fakhri): headers

#include "base/base_inc.h"
#include "game/game_ids.h"
#include "network/network_inc.h"
#include "os/os_inc.h"
#include "win32_inc.h"
#include "game/game_inc.h"

////////////////////////////////
//~ NOTE(fakhri): Globals

global OS_State   w32_os;
global W32_Timer  w32_timer;
global Thread_Ctx w32_main_tctx;
global HWND w32_window_handle;
global char w32_executable_path[256];
global char w32_executable_directory[256];
global char w32_working_directory[256];
global char w32_app_dll_path[256];
global char w32_temp_app_dll_path[256];
global OS_WorkQueue w32_work_queue;
global volatile b32 w32_host_running = 0;

////////////////////////////////
//~ NOTE(fakhri): implementations
#include "base/base_inc.cpp"
#include "network/network_inc.cpp"
#include "os/os_inc.cpp"
#include "win32_inc.cpp"

////////////////////////////////
//~ NOTE(fakhri): use high performance GPUs
exported DWORD NvOptimusEnablement = 1;
exported int AmdPowerXpressRequestHighPerformance = 1;

////////////////////////////////
//~ NOTE(fakhri): application options
#define Application_Name             "Lith"
#define W32_GraphicalWindowClassName L"ApplicationWindowClass"
#define W32_GraphicalWindowTitle     L"Lith"
#define W32_GraphicalWindowWidth     1280u
#define W32_GraphicalWindowHeight    720u 

//~

internal LRESULT
W32_WindowProc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0;
    OS_Event *event = 0;
    
    B32 is_release = 0;
    Axis2 scroll_axis = Axis2_Y;
    switch(message)
    {
        default:
        {
            result = DefWindowProcW(hwnd, message, w_param, l_param);
        }break;
        
        case WM_CLOSE:
        {
            w32_os.quit = true;
        }break;
        
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        {
            is_release = true;
        } fallthrough;
        case WM_MBUTTONDOWN:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        {
            
            OS_EventKind kind = is_release ? OS_EventKind_Release : OS_EventKind_Press;
            OS_Key key = OS_Key_MouseLeft;
            switch(message)
            {
                case WM_MBUTTONUP: case WM_MBUTTONDOWN: key = OS_Key_MouseMiddle; break;
                case WM_RBUTTONUP: case WM_RBUTTONDOWN: key = OS_Key_MouseRight; break;
            }
            
            event = OS_MakeEvent(w32_os.permanent_arena, &w32_os.events);
            event->kind = kind;
            event->key = key;
            event->position = w32_os.mouse_position;
        }break;
        
        case WM_MOUSEHWHEEL: scroll_axis = Axis2_X; goto scroll;
        case WM_MOUSEWHEEL:
        scroll:;
        {
            S16 wheel_delta = HIWORD(w_param);
            event = OS_MakeEvent(w32_os.permanent_arena, &w32_os.events);
            event->kind = OS_EventKind_MouseScroll;
            event->scroll[scroll_axis] = (F32)wheel_delta;
        }break;
        
        case WM_SETCURSOR:
        {
            if (LOWORD(l_param) == HTCLIENT)
            {
                SetCursor(NULL);
                result = TRUE;
            }
        }break;
        
        case WM_SYSKEYDOWN: case WM_SYSKEYUP:
        {
            DefWindowProcW(hwnd, message, w_param, l_param);
        }fallthrough;
        case WM_KEYDOWN: case WM_KEYUP:
        {
            //B32 was_down = !!(l_param & (1 << 30));
            B32 is_down =   !(l_param & (1 << 31));
            OS_EventKind kind = is_down ? OS_EventKind_Press : OS_EventKind_Release;
            
            local_persist OS_Key key_table[256] = {0};
            local_persist B32 key_table_initialized = 0;
            if(!key_table_initialized)
            {
                key_table_initialized = 1;
                
                for (U32 i = 'A', j = OS_Key_A; i <= 'Z'; i += 1, j += 1)
                {
                    key_table[i] = (OS_Key)j;
                }
                for (U32 i = '0', j = OS_Key_0; i <= '9'; i += 1, j += 1)
                {
                    key_table[i] = (OS_Key)j;
                }
                for (U32 i = VK_F1, j = OS_Key_F1; i <= VK_F24; i += 1, j += 1)
                {
                    key_table[i] = (OS_Key)j;
                }
                
                key_table[VK_ESCAPE]        = OS_Key_Esc;
                key_table[VK_OEM_3]         = OS_Key_GraveAccent;
                key_table[VK_OEM_MINUS]     = OS_Key_Minus;
                key_table[VK_OEM_PLUS]      = OS_Key_Equal;
                key_table[VK_BACK]          = OS_Key_Backspace;
                key_table[VK_TAB]           = OS_Key_Tab;
                key_table[VK_SPACE]         = OS_Key_Space;
                key_table[VK_RETURN]        = OS_Key_Enter;
                key_table[VK_CONTROL]       = OS_Key_Ctrl;
                key_table[VK_SHIFT]         = OS_Key_Shift;
                key_table[VK_MENU]          = OS_Key_Alt;
                key_table[VK_UP]            = OS_Key_Up;
                key_table[VK_LEFT]          = OS_Key_Left;
                key_table[VK_DOWN]          = OS_Key_Down;
                key_table[VK_RIGHT]         = OS_Key_Right;
                key_table[VK_DELETE]        = OS_Key_Delete;
                key_table[VK_PRIOR]         = OS_Key_PageUp;
                key_table[VK_NEXT]          = OS_Key_PageDown;
                key_table[VK_HOME]          = OS_Key_Home;
                key_table[VK_END]           = OS_Key_End;
                key_table[VK_OEM_2]         = OS_Key_ForwardSlash;
                key_table[VK_OEM_PERIOD]    = OS_Key_Period;
                key_table[VK_OEM_COMMA]     = OS_Key_Comma;
                key_table[VK_OEM_7]         = OS_Key_Quote;
                key_table[VK_OEM_4]         = OS_Key_LeftBracket;
                key_table[VK_OEM_6]         = OS_Key_RightBracket;
            }
            
            OS_Key key = OS_Key_Null;
            if(w_param < ArrayCount(key_table))
            {
                key = key_table[w_param];
            }
            
            event = OS_MakeEvent(w32_os.permanent_arena, &w32_os.events);
            event->kind = kind;
            event->key = key;
        }break;
        
        case WM_MOUSEMOVE:
        {
            os->mouse_position.x = (f32)GET_X_LPARAM(l_param); 
            os->mouse_position.y = (f32)GET_Y_LPARAM(l_param); 
        } break;
        
        case WM_SYSCOMMAND:
        {
            switch (w_param)
            {
                case SC_CLOSE:
                {
                    w32_os.quit = true;
                } break;
                case SC_KEYMENU:
                {}break;
                
                default:
                {
                    result = DefWindowProcW(hwnd, message, w_param, l_param);
                }break;
            }
        }break;
        
        case WM_CHAR: case WM_SYSCHAR:
        {
            u32 char_input = (u32)w_param;
            if (char_input == '\r')
            {
                char_input = '\n';
            }
            if((char_input >= 32 && char_input != 127) || char_input == '\t' || char_input == '\n')
            {
                event = OS_MakeEvent(w32_os.permanent_arena, &w32_os.events);
                event->kind = OS_EventKind_Text;
                event->character = char_input;
            }
        }break;
        
        case WM_DPICHANGED:
        {
            //F32 new_dpi = (F32)w_param;
            result = DefWindowProcW(hwnd, message, w_param, l_param);
        }break;
        
    }
    
    if(event)
    {
        event->modifiers = W32_GetModifiers();
        OS_EventListPushBack(&w32_os.events, event);
    }
    
    return result;
}

internal DWORD WINAPI 
W32_WorkerThreadMain(LPVOID param)
{
    Thread_Ctx WorkerThreadContext = MakeTCTX();
    SetTCTX(&WorkerThreadContext);
    
    OS_WorkQueue *work_queue = (OS_WorkQueue *)param;
    
    for(;;)
    {
        b32 did_work = W32_ProcessOneWorkQueueEntry();
        if (!did_work)
        {
            // NOTE(fakhri): queue is empty
            WaitForSingleObject(work_queue->waiting_worker_threads_semaphore, INFINITE);
        }
    }
}


int
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR lp_cmd_line, int n_show_cmd)
{
    // NOTE(fakhri): setup network
    {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) 
        {
            LogError("Couldn't init socket");
            PostQuitMessage(1);
        }
    }
    
    u32 concurr_threads_count = 1;
    network_thread_iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
                                                        0, 0,
                                                        concurr_threads_count);
    
    
    W32_TimerInit(&w32_timer);
    
    W32_SoundOutput win32_sound_output = {0};
    // NOTE(fakhri): Sound initialization
    {
        win32_sound_output.channels = 2;
        win32_sound_output.samples_per_second = 48000;
        win32_sound_output.latency_frame_count = 48000;
        W32_LoadWASAPI();
        W32_InitWASAPI(&win32_sound_output);
    }
    
    // NOTE(rjf): Find refresh rate
    f32 refresh_rate = 60.f;
    {
        DEVMODEA device_mode = {0};
        if(EnumDisplaySettingsA(0, ENUM_CURRENT_SETTINGS, &device_mode))
        {
            refresh_rate = (float)device_mode.dmDisplayFrequency;
        }
    }
    
    // NOTE(rjf): Initialize platform
    {
        os = &w32_os;
        
        w32_os.quit                      = 0;
        w32_os.vsync                     = 1;
        w32_os.fullscreen                = 0;
        w32_os.window_size.x             = DEFAULT_WINDOW_WIDTH;
        w32_os.window_size.y             = DEFAULT_WINDOW_HEIGHT;
        w32_os.time.game_time            = 0.f;
        
        w32_os.target_frames_per_second  = refresh_rate;
        w32_os.time.game_dt_for_frame = 1.f / os->target_frames_per_second; 
        
        w32_os.samples_per_second = win32_sound_output.samples_per_second;
        w32_os.sample_out         = (f32 *)W32_HeapAlloc(win32_sound_output.samples_per_second * sizeof(f32) * 2);
        
        // NOTE(fakhri): Platform exposed API
        w32_os.Release                      = W32_Release;
        w32_os.Reserve                      = W32_Reserve;
        w32_os.Commit                       = W32_Commit;
        w32_os.Decommit                     = W32_Decommit;
        w32_os.PageSize                     = W32_PageSize;
        w32_os.OutputError                  = W32_OutputError;
        w32_os.SaveToFile                   = W32_SaveToFile;
        w32_os.AppendToFile                 = W32_AppendToFile;
        w32_os.LoadEntireFile               = W32_LoadEntireFile;
        w32_os.DeleteFile                   = W32_DeleteFile;
        w32_os.MakeDirectory                = W32_MakeDirectory;
        w32_os.BeginHostMessageQueueRead    = W32_BeginHostMessageQueueRead;
        w32_os.EndHostMessageQueueRead      = W32_EndHostMessageQueueRead;
        w32_os.BeginPlayerMessageQueueWrite = W32_BeginPlayerMessageQueueWrite;
        w32_os.EndPlayerMessageQueueWrite   = W32_EndPlayerMessageQueueWrite;
        w32_os.IsHostMessageQueueEmpty      = W32_IsHostMessageQueueEmpty;
        w32_os.AcceptSocket                 = W32_AcceptSocket;
        w32_os.CloseSocket                  = W32_CloseSocket;
        w32_os.ConnectToServer              = W32_ConnectToServer;
        w32_os.OpenListenSocket             = W32_OpenListenSocket;
        w32_os.SendBuffer                   = W32_SendBuffer;
        w32_os.ReceiveBuffer                = W32_ReceiveBuffer;
        w32_os.SendString                   = W32_SendString;
        w32_os.ReceiveString                = W32_ReceiveString;
        w32_os.WaitForSemaphore             = W32_WaitForSemaphore;
        w32_os.ReleaseSemaphore             = W32_ReleaseSemaphore;
        w32_os.WaitForMutex                 = W32_WaitForMutex;
        w32_os.CreateMutex                  = W32_CreateMutex;
        w32_os.ReleaseMutex                 = W32_ReleaseMutex;
        w32_os.PopQueueEntry                = W32_PopQueueEntry;
        w32_os.ProcessOneWorkQueueEntry     = W32_ProcessOneWorkQueueEntry;
        w32_os.PushWorkQueueEntrySP         = W32_PushWorkQueueEntrySP;
        w32_os.PushWorkQueueEntry           = W32_PushWorkQueueEntry;
        w32_os.IsWorkQueueEmpty             = W32_IsWorkQueueEmpty;
        w32_os.CopyStringToClipboard        = W32_CopyStringToClipboard;
        w32_os.GetStringFromClipboard       = W32_GetStringFromClipboard;
        
        w32_os.StartGameHost                = W32_StartGameHost;
        w32_os.IsGameHostRunning            = W32_IsGameHostRunning;
        w32_os.StopGameHost                 = W32_StopGameHost;
        
        w32_os.permanent_arena = M_ArenaAllocDefault();
        for (u32 arena_index = 0;
             arena_index < ArrayCount(w32_os.frame_arenas);
             ++arena_index)
        {
            w32_os.frame_arenas[arena_index] = M_ArenaAlloc(Gigabytes(16));
        }
    }
    
    // NOTE(fakhri): worker threads
    {
        u32 initial_count = 0;
        u32 work_threads_count = 8;
        
        w32_work_queue.waiting_worker_threads_semaphore = CreateSemaphoreA(0, initial_count, work_threads_count, 0);
        w32_work_queue.producer_mutex                   = CreateMutexA(0, FALSE, 0);
        
        
        // NOTE(fakhri): start worker threads
        for (u32 thread_index = 0;
             thread_index < work_threads_count;
             ++thread_index)
        {
            Thread_Handle handle = CreateThread(0, 0, W32_WorkerThreadMain, &w32_work_queue, 0, 0);
            CloseHandle(handle);
        }
        Log("Started worker threads");
    }
    
    // NOTE(fakhri): init the main thread context
    {
        w32_main_tctx = MakeTCTX();
        SetTCTX(&w32_main_tctx);
    }
    
    String clip = W32_GetStringFromClipboard(OS_FrameArena());
    
    // NOTE(fakhri): Calculate executable name and path to DLL
    {
        DWORD size_of_executable_path =
            GetModuleFileNameA(0, w32_executable_path, sizeof(w32_executable_path));
        
        // NOTE(fakhri): Calculate executable directory
        {
            MemoryCopy(w32_executable_directory, w32_executable_path, size_of_executable_path);
            char *one_past_last_slash = w32_executable_directory;
            for(i32 i = 0; w32_executable_directory[i]; ++i)
            {
                if(w32_executable_directory[i] == '\\')
                {
                    one_past_last_slash = w32_executable_directory + i + 1;
                }
            }
            *one_past_last_slash = 0;
        }
        
        // NOTE(fakhri): Create DLL filenames
        {
            wsprintf(w32_app_dll_path, "%s%s_code.dll", w32_executable_directory, Application_Name);
            wsprintf(w32_temp_app_dll_path, "%stemp_%s.dll", w32_executable_directory, Application_Name);
        }
        
        GetCurrentDirectory(sizeof(w32_working_directory), w32_working_directory);
    }
    
    // NOTE(fakhri): launch network and host server threads
    {
        // NOTE(fakhri): start the network thread
        Thread_Handle network_thread_handle = CreateThread(0, 0, NetworkMain, 0, 0, 0);
        CloseHandle(network_thread_handle);
    }
    
    //- NOTE(fakhri): initialize window class
    WNDCLASSW window_class = {0};
    {
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc = W32_WindowProc;
        window_class.hInstance = instance;
        window_class.lpszClassName = W32_GraphicalWindowClassName;
        window_class.hCursor = LoadCursor(0, IDC_ARROW);
    }
    if(!RegisterClassW(&window_class))
    {
        W32_OutputError("Fatal Error", "Window class registration failure.");
        goto quit;
    }
    
    //- NOTE(fakhri): create a window
    w32_window_handle = CreateWindowW(W32_GraphicalWindowClassName, W32_GraphicalWindowTitle,
                                      WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                      W32_GraphicalWindowWidth,
                                      W32_GraphicalWindowHeight,
                                      0, 0, instance, 0);
    
    if(!w32_window_handle)
    {
        W32_OutputError("Fatal Error", "Window creation failure.");
        goto quit;
    }
    
    //- NOTE(fakhri): OpenGL initialization
    {
        w32_device_context = GetDC(w32_window_handle);
        W32_InitOpenGL(instance);
        gladLoadGL();
    }
    
    Game_State *game_state;
    W32_AppCode w32_game_code = {};
    //- NOTE(fakhri): Load and init app code
    {
        game_state = (Game_State *)M_ArenaPushAligned(w32_os.permanent_arena, sizeof(Game_State), 16);
        W32_AppCodeLoad(&w32_game_code);
        w32_game_code.PermanentLoad(os, game_state);
    }
    
    os->config = OS_LoadConfigFile(w32_os.permanent_arena);
    
    // NOTE(fakhri): init directory watcher for shader files hotreload
    Directory_Watcher shaders_watcher = {};
    if (!W32_BeginWatchDirectory(w32_os.permanent_arena, &shaders_watcher, Str8Lit("data/shaders/")))
    {
        InvalidCodePath;
    }
    
    ShowWindow(w32_window_handle, n_show_cmd);
    UpdateWindow(w32_window_handle);
    
    while(!w32_os.quit)
    {
        M_ArenaClear(OS_FrameArena());
        W32_TimerBeginFrame(&w32_timer);
        
        // NOTE(fakhri): Windows events loop
        {
            MSG message;
            while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }
        
        // NOTE(fakhri): Update window size
        {
            v2i32 old_window_size = w32_os.window_size;
            RECT client_rect;
            GetClientRect(w32_window_handle, &client_rect);
            w32_os.window_size.x = client_rect.right - client_rect.left;
            w32_os.window_size.y = client_rect.bottom - client_rect.top;
            glViewport(0, 0, w32_os.window_size.width, w32_os.window_size.height);
        }
        
        
        // NOTE(rjf): Find how much sound to write and where
        if(win32_sound_output.initialized)
        {
            w32_os.sample_count_to_output = 0;
            UINT32 sound_padding_size;
            if(SUCCEEDED(win32_sound_output.audio_client->lpVtbl->GetCurrentPadding(win32_sound_output.audio_client, &sound_padding_size)))
            {
                w32_os.samples_per_second = win32_sound_output.samples_per_second;
                w32_os.sample_count_to_output = (u32)(win32_sound_output.latency_frame_count - sound_padding_size);
                if(w32_os.sample_count_to_output > win32_sound_output.latency_frame_count)
                {
                    w32_os.sample_count_to_output = win32_sound_output.latency_frame_count;
                }
            }
            
            for(u32 i = 0; i < win32_sound_output.buffer_frame_count; ++i)
            {
                w32_os.sample_out[i] = 0;
            }
        }
        
        // NOTE(fakhri): hot reload game code if needed
        {
            W32_AppCodeUpdate(&w32_game_code);
        }
        
        // NOTE(fakhri): hotreload shaders if needed
        {
            M_Temp scratch = GetScratch(0, 0);
            String8 changed_shader_name = W32_CheckDirectoryChanges(scratch.arena, &shaders_watcher);
            if (changed_shader_name.size)
            {
                w32_game_code.HotLoadShader(scratch.arena, changed_shader_name, game_state);
            }
            ReleaseScratch(scratch);
        }
        
        // NOTE(rjf): Call into the app layer to update
        {
            b32 last_fullscreen = w32_os.fullscreen;
            w32_game_code.UpdateAndRender(game_state, w32_os.time.game_dt_for_frame);
            // NOTE(fakhri): Update fullscreen if necessary
            if(last_fullscreen != w32_os.fullscreen)
            {
                W32_ToggleFullscreen(w32_window_handle);
            }
            
            // NOTE(rjf): Fill sound buffer with game sound
            if(win32_sound_output.initialized)
            {
                W32_FillSoundBuffer(w32_os.sample_count_to_output, w32_os.sample_out, &win32_sound_output);
            }
        }
        
        OS_EndFrame();
        W32_OpenGLRefreshScreen();
        W32_TimerEndFrame(&w32_timer);
        
    }
    
    quit:;
    return 0;
}
