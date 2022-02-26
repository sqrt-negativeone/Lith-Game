// NOTE(fakhri): windows shenanigans
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <windowsx.h>
#define CINTERFACE
#include <xinput.h>
#include <objbase.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>

#undef DeleteFile

extern "C"
{
    _declspec(dllexport) DWORD NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// NOTE(rjf): OpenGL
#include "glad/glad.h" 
#include "glad/glad.c"
#include "GL/wglext.h"

// NOTE(rjf): CRT
#include <stdio.h>

// NOTE(rjf): Headers
#include "program_options.h"
#include "language_layer.h"
#include "network_shared/network_utilities.h"

#include "asserts.h"
#include "maths.h"
#include "memory.h"
#include "strings.h"
#include "network_message.h"
#include "app.h"
#include "os.h"
#include "win32_timer.h"
#include "win32_network_thread.h"

#include "language_layer.c"
#include "network_shared/network_utilities.cpp"
#include "network_shared/host_info.cpp"
#include "asserts.cpp"
#include "memory.c"
#include "strings.c"
#include "win32_game_server.cpp"
#include "os.c"
#include "shader.cpp"
#include "win32_network_thread.cpp"

// NOTE(rjf): Globals
global char global_executable_path[256];
global char global_executable_directory[256];
global char global_working_directory[256];
global char global_app_dll_path[256];
global char global_temp_app_dll_path[256];
global OS_State global_os;
global HDC global_device_context;
global HINSTANCE global_instance_handle;
global W32_Timer global_win32_timer = {0};
#define W32_MAX_GAMEPADS 16
typedef struct W32_GamepadInput W32_GamepadInput;
struct W32_GamepadInput
{
    b32 connected;
    v2 joystick_1;
    v2 joystick_2;
    f32 trigger_left;
    f32 trigger_right;
    i32 button_states[GamepadButton_Max];
};
W32_GamepadInput global_gamepads[W32_MAX_GAMEPADS];

// NOTE(fakhri): directory watcher
struct Directory_Watcher
{
    HANDLE dir_handle;
    OVERLAPPED *overlapped;
#define WATCHER_BUFFER_SIZE 1024
    char buffer[WATCHER_BUFFER_SIZE];
    DWORD bytes_filled;
};

// NOTE(fakhri): global shaders directory watcher
global Directory_Watcher global_shaders_watcher;

// NOTE(rjf): Implementations
#include "win32_utilities.c"
#include "win32_timer.c"
#include "win32_file_io.c"
#include "win32_app_code.c"
#include "win32_xinput.c"
#include "win32_wasapi.c"
#include "win32_opengl.c"

//~

typedef enum W32_CursorStyle
{
    W32_CursorStyle_Normal,
    W32_CursorStyle_HorizontalResize,
    W32_CursorStyle_VerticalResize,
    W32_CursorStyle_IBar,
}
W32_CursorStyle;

global W32_CursorStyle global_cursor_style;

internal v2
W32_GetMousePosition(HWND window)
{
    v2 result = {0};
    POINT mouse;
    GetCursorPos(&mouse);
    ScreenToClient(window, &mouse);
    result.x = (f32)(mouse.x);
    result.y = (f32)(mouse.y);
    return result;
}

internal LRESULT
W32_WindowProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0;
    local_persist b32 mouse_hover_active_because_windows_makes_me_cry = 0;
    KeyModifiers modifiers = 0;
    if(GetKeyState(VK_CONTROL) & 0x8000)
    {
        modifiers |= KeyModifier_Ctrl;
    }
    if(GetKeyState(VK_SHIFT) & 0x8000)
    {
        modifiers |= KeyModifier_Shift;
    }
    if(GetKeyState(VK_MENU) & 0x8000)
    {
        modifiers |= KeyModifier_Alt;
    }
    switch(message)
    {
        case WM_QUIT:
        case WM_DESTROY:
        case WM_CLOSE:
        {
            global_os.quit = 1;
        } break;
        case WM_LBUTTONDOWN:
        {
            global_os.controller.left_mouse.pressed = 1;
            OS_PushEvent(OS_MousePressEvent(MouseButton_Left, global_os.mouse_position));
            result = DefWindowProc(window_handle, message, w_param, l_param);
        } break;
        case WM_LBUTTONUP:
        {
            global_os.controller.left_mouse.released = 1;
            OS_PushEvent(OS_MouseReleaseEvent(MouseButton_Left, global_os.mouse_position));
            result = DefWindowProc(window_handle, message, w_param, l_param);
        } break;
        case WM_RBUTTONDOWN:
        {
            global_os.controller.right_mouse.pressed = 1;
            OS_PushEvent(OS_MousePressEvent(MouseButton_Right, global_os.mouse_position));
            result = DefWindowProc(window_handle, message, w_param, l_param);
        } break;
        case WM_RBUTTONUP:
        {
            global_os.controller.right_mouse.released = 1;
            OS_PushEvent(OS_MouseReleaseEvent(MouseButton_Right, global_os.mouse_position));
            result = DefWindowProc(window_handle, message, w_param, l_param);
        } break;
        case WM_MOUSEMOVE:
        {
            i16 x_position = LOWORD(l_param);
            i16 y_position = HIWORD(l_param);
            v2 last_mouse = global_os.mouse_position;
            global_os.mouse_position = W32_GetMousePosition(window_handle);
            result = DefWindowProc(window_handle, message, w_param, l_param);
            OS_PushEvent(OS_MouseMoveEvent(global_os.mouse_position,
                                           v2{global_os.mouse_position.x - last_mouse.x,
                                               global_os.mouse_position.y - last_mouse.y}));
            if(mouse_hover_active_because_windows_makes_me_cry == 0)
            {
                mouse_hover_active_because_windows_makes_me_cry = 1;
                TRACKMOUSEEVENT track_mouse_event = {0};
                {
                    track_mouse_event.cbSize = sizeof(track_mouse_event);
                    track_mouse_event.dwFlags = TME_LEAVE;
                    track_mouse_event.hwndTrack = window_handle;
                    track_mouse_event.dwHoverTime = HOVER_DEFAULT;
                }
                TrackMouseEvent(&track_mouse_event);
            }
        } break;
        case WM_MOUSELEAVE:
        {
            mouse_hover_active_because_windows_makes_me_cry = 0;
        } break;
        case WM_MOUSEWHEEL:
        {
            i16 wheel_delta = HIWORD(w_param);
            OS_PushEvent(OS_MouseScrollEvent(v2{0, (f32)wheel_delta}, modifiers));
        } break;
        case WM_MOUSEHWHEEL:
        {
            i16 wheel_delta = HIWORD(w_param);
            OS_PushEvent(OS_MouseScrollEvent(v2{(f32)wheel_delta, 0}, modifiers));
        } break;
        case WM_SETCURSOR:
        {
#if 1
            if (LOWORD(l_param) == HTCLIENT)
            {
                SetCursor(NULL);
                result = TRUE;
            }
#else
            if(global_os.mouse_position.x >= 1 && global_os.mouse_position.x <= global_os.window_size.x-1 &&
               global_os.mouse_position.y >= 1 && global_os.mouse_position.y <= global_os.window_size.y-1 && mouse_hover_active_because_windows_makes_me_cry)
            {
                switch(global_cursor_style)
                {
                    case W32_CursorStyle_HorizontalResize:
                    {
                        SetCursor(LoadCursorA(0, IDC_SIZEWE));
                        break;
                    }
                    case W32_CursorStyle_VerticalResize:
                    {
                        SetCursor(LoadCursorA(0, IDC_SIZENS));
                        break;
                    }
                    case W32_CursorStyle_IBar:
                    {
                        SetCursor(LoadCursorA(0, IDC_IBEAM));
                        break;
                    }
                    case W32_CursorStyle_Normal:
                    {
                        SetCursor(LoadCursorA(0, IDC_ARROW));
                        break;
                    }
                    default: break;
                }
            }
#endif
            else
            {
                result = DefWindowProc(window_handle, message, w_param, l_param);
            }
            
        } break;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            u64 vkey_code = w_param;
            i8 was_down = !!(l_param & (1 << 30));
            i8 is_down =   !(l_param & (1 << 31));
            u64 key_input = 0;
            if((vkey_code >= 'A' && vkey_code <= 'Z') ||
               (vkey_code >= '0' && vkey_code <= '9'))
            {
                // NOTE(rjf): Letter/number buttons
                key_input = (vkey_code >= 'A' && vkey_code <= 'Z') ? Key_A + (vkey_code-'A') : Key_0 + (vkey_code-'0');
            }
            else
            {
                switch (vkey_code)
                {
                    case VK_ESCAPE:
                    {
                        key_input = Key_Esc;
                        os->controller.escape_key.pressed = is_down;
                        os->controller.escape_key.released = was_down;
                    } break;
                    case VK_OEM_3:
                    {
                        key_input = Key_GraveAccent;
                    } break;
                    case VK_OEM_MINUS:
                    {
                        key_input = Key_Minus;
                    } break;
                    case VK_OEM_PLUS:
                    {
                        key_input = Key_Equal;
                    } break;
                    case VK_BACK:
                    {
                        key_input = Key_Backspace;
                    } break;
                    case VK_TAB:
                    {
                        key_input = Key_Tab;
                    } break;
                    case VK_SPACE:
                    {
                        key_input = Key_Space;
                    } break;
                    case VK_RETURN:
                    {
                        key_input = Key_Enter;
                        os->controller.confirm.pressed = is_down;
                        os->controller.confirm.released = was_down;
                    } break;
                    case VK_CONTROL:
                    {
                        key_input = Key_Ctrl;
                        modifiers &= ~KeyModifier_Ctrl;
                    } break;
                    case VK_SHIFT:
                    {
                        key_input = Key_Shift;
                        modifiers &= ~KeyModifier_Shift;
                    } break;
                    case VK_MENU:
                    {
                        key_input = Key_Alt;
                        modifiers &= ~KeyModifier_Alt;
                    } break;
                    case VK_UP:
                    {
                        key_input = Key_Up;
                        os->controller.move_up.pressed = is_down;
                        os->controller.move_up.released = was_down;
                    } break;
                    case VK_LEFT:
                    {
                        key_input = Key_Left;
                        os->controller.move_left.pressed = is_down;
                        os->controller.move_left.released = was_down;
                    } break;
                    case VK_DOWN:
                    {
                        key_input = Key_Down;
                        os->controller.move_down.pressed = is_down;
                        os->controller.move_down.released = was_down;
                    } break;
                    case VK_RIGHT:
                    {
                        key_input = Key_Right;
                        os->controller.move_right.pressed = is_down;
                        os->controller.move_right.released = was_down;
                    } break;
                    case VK_DELETE:
                    {
                        key_input = Key_Delete;
                    } break;
                    case VK_PRIOR:
                    {
                        key_input = Key_PageUp;
                    } break;
                    case VK_NEXT:
                    {
                        key_input = Key_PageDown;
                    } break;
                    case VK_HOME:
                    {
                        key_input = Key_Home;
                    } break;
                    case VK_END:
                    {
                        key_input = Key_End;
                    } break;
                    case VK_OEM_2:
                    {
                        key_input = Key_ForwardSlash;
                    } break;
                    case VK_OEM_PERIOD:
                    {
                        key_input = Key_Period;
                    } break;
                    case VK_OEM_COMMA:
                    {
                        key_input = Key_Comma;
                    } break;
                    case VK_OEM_7:
                    {
                        key_input = Key_Quote;
                    } break;
                    case VK_OEM_4:
                    {
                        key_input = Key_LeftBracket;
                    } break;
                    case VK_OEM_6:
                    {
                        key_input = Key_RightBracket;
                    } break;
                    case VK_F1:
                    {
                        os->controller.toggle_fullscreen.pressed = is_down;
                        os->controller.toggle_fullscreen.released = was_down;
                    } break;
                    default:
                    {
                        if(vkey_code >= VK_F1 && vkey_code <= VK_F12)
                        {
                            key_input = Key_F1 + vkey_code - VK_F1;
                        }
                    } break;
                }
            }
            
            if(is_down)
            {
                OS_PushEvent(OS_KeyPressEvent((Key)key_input, modifiers));
            }
            else
            {
                OS_PushEvent(OS_KeyReleaseEvent((Key)key_input, modifiers));
            }
            result = DefWindowProc(window_handle, message, w_param, l_param);
        } break;
        case WM_CHAR:
        {
            u64 char_input = w_param;
            if(char_input >= 32 && char_input != VK_RETURN && char_input != VK_ESCAPE &&
               char_input != 127)
            {
                OS_PushEvent(OS_CharacterInputEvent(char_input));
            }
        } break;
        default:
        {
            result = DefWindowProc(window_handle, message, w_param, l_param);
        }
    }
    return result;
}

internal f32
W32_GetTime(void)
{
    W32_Timer *timer = &global_win32_timer;
    LARGE_INTEGER current_time;
    QueryPerformanceCounter(&current_time);
    return global_os.current_time + (f32)(current_time.QuadPart - timer->begin_frame.QuadPart) / (f32)timer->counts_per_second.QuadPart;
}

internal u64
W32_GetCycles(void)
{
    u64 result = __rdtsc();
    return result;
}

internal void
W32_ResetCursor(void)
{
    global_cursor_style = W32_CursorStyle_Normal;
}

internal void
W32_SetCursorToHorizontalResize(void)
{
    global_cursor_style = W32_CursorStyle_HorizontalResize;
}

internal void
W32_SetCursorToVerticalResize(void)
{
    global_cursor_style = W32_CursorStyle_VerticalResize;
}

int
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR lp_cmd_line, int n_show_cmd)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) 
    {
        LogError("Couldn't init socket");
        PostQuitMessage(1);
    }
    
    u32 concurrent_threads_count = 1;
    
    global_iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
                                                0, 0,
                                                concurrent_threads_count);
    
    HANDLE server_thread_handle  = CreateThread(0, 0, ServerMain, 0, 0, 0);
    // TODO(fakhri): wait for the server thread to create a socket and give us info about our address?
    
    HANDLE network_thread_handle = CreateThread(0, 0, NetworkMain, 0, 0, 0);
    
    // NOTE(fakhri): we won't be using talking about these threads again
    CloseHandle(network_thread_handle);
    CloseHandle(server_thread_handle);
    
    global_instance_handle = instance;
    
    W32_TimerInit(&global_win32_timer);
    W32_AppCode win32_game_code = {0};
    W32_SoundOutput win32_sound_output = {0};
    
    // NOTE(rjf): Calculate executable name and path to DLL
    {
        DWORD size_of_executable_path =
            GetModuleFileNameA(0, global_executable_path, sizeof(global_executable_path));
        
        // NOTE(rjf): Calculate executable directory
        {
            MemoryCopy(global_executable_directory, global_executable_path, size_of_executable_path);
            char *one_past_last_slash = global_executable_directory;
            for(i32 i = 0; global_executable_directory[i]; ++i)
            {
                if(global_executable_directory[i] == '\\')
                {
                    one_past_last_slash = global_executable_directory + i + 1;
                }
            }
            *one_past_last_slash = 0;
        }
        
        // NOTE(rjf): Create DLL filenames
        {
            wsprintf(global_app_dll_path, "%s%s_code.dll", global_executable_directory, PROGRAM_FILENAME);
            wsprintf(global_temp_app_dll_path, "%stemp_%s.dll", global_executable_directory, PROGRAM_FILENAME);
        }
        
        GetCurrentDirectory(sizeof(global_working_directory), global_working_directory);
    }
    
    WNDCLASS window_class = {0};
    {
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc = W32_WindowProc;
        window_class.hInstance = instance;
        window_class.lpszClassName = "ApplicationWindowClass";
        window_class.hCursor = LoadCursor(0, IDC_ARROW);
    }
    
    if(!RegisterClass(&window_class))
    {
        // NOTE(rjf): ERROR: Window class registration failure
        W32_OutputError("Fatal Error", "Window class registration failure.");
        goto quit;
    }
    
    HWND window_handle = CreateWindow("ApplicationWindowClass", WINDOW_TITLE,
                                      WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                      DEFAULT_WINDOW_WIDTH,
                                      DEFAULT_WINDOW_HEIGHT,
                                      0, 0, instance, 0);
    
    if(!window_handle)
    {
        // NOTE(rjf): ERROR: Window creation failure
        W32_OutputError("Fatal Error", "Window creation failure.");
        goto quit;
    }
    
    // NOTE(rjf): Load application code
    W32_AppCode win32_app_code = {0};
    {
        if(!W32_AppCodeLoad(&win32_app_code))
        {
            // NOTE(rjf): ERROR: Application code load failure
            W32_OutputError("Fatal Error", "Application code load failure.");
            goto quit;
        }
    }
    
    // NOTE(rjf): Sound initialization
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
        os = &global_os;
        
        global_os.executable_folder_absolute_path = String8FromCString(global_executable_directory);
        global_os.executable_absolute_path = String8FromCString(global_executable_path);
        global_os.working_directory_path = String8FromCString(global_working_directory);
        
        global_os.quit                      = 0;
        global_os.vsync                     = 1;
        global_os.fullscreen                = 0;
        global_os.window_size.x             = DEFAULT_WINDOW_WIDTH;
        global_os.window_size.y             = DEFAULT_WINDOW_HEIGHT;
        global_os.current_time              = 0.f;
        // TODO(fakhri): we can just target 30fps because we don't need 60fps for a cards game
        // and use the extra time for server processing and stuff
        
#if 1
        global_os.target_frames_per_second  = refresh_rate;
#else
        global_os.target_frames_per_second  = 30;
#endif
        
        global_os.sample_out = (f32 *)W32_HeapAlloc(win32_sound_output.samples_per_second * sizeof(f32) * 2);
        global_os.samples_per_second = win32_sound_output.samples_per_second;
        
        global_os.Reserve                          = W32_Reserve;
        global_os.Release                          = W32_Release;
        global_os.Commit                           = W32_Commit;
        global_os.Decommit                         = W32_Decommit;
        global_os.OutputError                      = W32_OutputError;
        global_os.SaveToFile                       = W32_SaveToFile;
        global_os.AppendToFile                     = W32_AppendToFile;
        global_os.LoadEntireFile                   = W32_LoadEntireFile;
        global_os.LoadEntireFileAndNullTerminate   = W32_LoadEntireFileAndNullTerminate;
        global_os.DeleteFile                       = W32_DeleteFile;
        global_os.MakeDirectory                    = W32_MakeDirectory;
        global_os.DoesFileExist                    = W32_DoesFileExist;
        global_os.DoesDirectoryExist               = W32_DoesDirectoryExist;
        global_os.CopyFile                         = W32_CopyFile;
        global_os.ListDirectory                    = W32_DirectoryListLoad;
        global_os.GetTime                          = W32_GetTime;
        global_os.GetCycles                        = W32_GetCycles;
        global_os.ResetCursor                      = W32_ResetCursor;
        global_os.SetCursorToHorizontalResize      = W32_SetCursorToHorizontalResize;
        global_os.SetCursorToVerticalResize        = W32_SetCursorToVerticalResize;
        global_os.RefreshScreen                    = W32_OpenGLRefreshScreen;
        global_os.PushNetworkMessage               = W32_PushNetworkMessageToServer;
        global_os.GetNextNetworkMessageIfAvailable = W32_GetNextNetworkMessageIfAvailable;
        
        global_os.permanent_arena = M_ArenaInitialize();
        global_os.frame_arena = M_ArenaInitialize();
        
        // NOTE(fakhri): allocate memory for game state
        global_os.game_state = (Game_State *)M_ArenaPushZero(&global_os.permanent_arena, sizeof(Game_State));
        
    }
    
    // NOTE(rjf): OpenGL initialization
    {
        global_device_context = GetDC(window_handle);
        if(!W32_InitOpenGL(&global_device_context, global_instance_handle))
        {
            W32_OutputError("Fatal Error", "OpenGL initialization failure.");
            goto quit;
        }
        else
        {
            gladLoadGL();
        }
    }
    
    W32_LoadXInput();
    
    win32_app_code.PermanentLoad(&global_os);
    win32_app_code.HotLoad(&global_os);
    
    ShowWindow(window_handle, n_show_cmd);
    UpdateWindow(window_handle);
    
    // NOTE(fakhri): setup directory watcher to see if any shader changed
    W32_BeginWatchDirectory( &global_shaders_watcher, &global_os.permanent_arena, S8Lit("shaders/"));
    
    glEnable(GL_CULL_FACE); 
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    while(!global_os.quit)
    {
        W32_TimerBeginFrame(&global_win32_timer);
        M_ArenaClear(&os->frame_arena);
        
        // NOTE(rjf): Update Windows events
        {
            MSG message;
            while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }
        
        // NOTE(rjf): Update window size
        {
            iv2 old_window_size = global_os.window_size;
            RECT client_rect;
            GetClientRect(window_handle, &client_rect);
            global_os.window_size.x = client_rect.right - client_rect.left;
            global_os.window_size.y = client_rect.bottom - client_rect.top;
        }
        
        // NOTE(rjf): Update input data (post-event)
        OS_BeginFrame();
        {
            POINT mouse;
            GetCursorPos(&mouse);
            ScreenToClient(window_handle, &mouse);
            W32_UpdateXInput();
            global_os.pump_events = 0;
        }
        
        
        // NOTE(rjf): Find how much sound to write and where
        if(win32_sound_output.initialized)
        {
            global_os.sample_count_to_output = 0;
            UINT32 sound_padding_size;
            if(SUCCEEDED(win32_sound_output.audio_client->lpVtbl->GetCurrentPadding(win32_sound_output.audio_client, &sound_padding_size)))
            {
                global_os.samples_per_second = win32_sound_output.samples_per_second;
                global_os.sample_count_to_output = (u32)(win32_sound_output.latency_frame_count - sound_padding_size);
                if(global_os.sample_count_to_output > win32_sound_output.latency_frame_count)
                {
                    global_os.sample_count_to_output = win32_sound_output.latency_frame_count;
                }
            }
            
            for(u32 i = 0; i < win32_sound_output.buffer_frame_count; ++i)
            {
                global_os.sample_out[i] = 0;
            }
        }
        
        W32_AppCodeUpdate(&win32_app_code);
        
        // NOTE(fakhri): check if any shader file changed and hotreload it if so
        {
            s8 changed_shader_name = W32_CheckDirectoryChanges(&global_os.frame_arena, &global_shaders_watcher);
            if (changed_shader_name.size)
            {
                s8 shader_path = StringConcatenate(&global_os.frame_arena, S8Lit("shaders/"), changed_shader_name);
                
                ShaderFileHotreload(&global_os.frame_arena, shader_path);
            }
        }
        
        // NOTE(rjf): Call into the app layer to update
        {
            b32 last_fullscreen = global_os.fullscreen;
            
            win32_app_code.UpdateAndRender();
            
            
            
            // NOTE(rjf): Update fullscreen if necessary
            if(last_fullscreen != global_os.fullscreen)
            {
                W32_ToggleFullscreen(window_handle);
            }
            
            // NOTE(rjf): Fill sound buffer with game sound
            if(win32_sound_output.initialized)
            {
                W32_FillSoundBuffer(global_os.sample_count_to_output, global_os.sample_out, &win32_sound_output);
            }
        }
        
        // NOTE(rjf): Post-update platform data update
        {
            OS_EndFrame();
        }
        
        W32_OpenGLRefreshScreen();
        W32_TimerEndFrame(&global_win32_timer, 1000.0 * (1.0 / (f64)global_os.target_frames_per_second));
    }
    
    W32_AppCodeUnload(&win32_app_code);
    W32_CleanUpOpenGL(&global_device_context);
    
    quit:;
    
    return 0;
}
