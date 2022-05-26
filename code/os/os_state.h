/* date = March 26th 2022 6:38 pm */

#ifndef OS_STATE_H
#define OS_STATE_H

struct OS_Time
{
    f32 game_time;
    f32 wall_time;
    f32 game_dt_for_frame;
    f32 wall_dt_for_frame;
};

struct OS_State
{
    // NOTE(fakhri): Memory
    M_Arena *permanent_arena;
    M_Arena *frame_arenas[2];
    
    // NOTE(fakhri): Options
    volatile b32 quit;
    b32 vsync;
    b32 fullscreen;
    v2i32 window_size;
    u32 frame_index;
    
    f32 target_frames_per_second;
    OS_Time time;
    
    // NOTE(fakhri): Event List and Input Data
    v2 mouse_position;
    OS_EventList events;
    
    // NOTE(fakhri): Audio Output Data
    f32 *sample_out;
    u32 sample_count_to_output;
    u32 samples_per_second;
    
    // NOTE(fakhri): Platform exposed API
    OS_Release_Fucntion                      *Release;
    OS_Reserve_Function                      *Reserve;
    OS_Commit_Function                       *Commit;
    OS_Decommit_Function                     *Decommit;
    OS_PageSize_Function                     *PageSize;
    OS_OutputError_Function                  *OutputError;
    OS_SaveToFile_Function                   *SaveToFile;
    OS_AppendToFile_Function                 *AppendToFile;
    OS_LoadEntireFile_Function               *LoadEntireFile;
    OS_DeleteFile_Function                   *DeleteFile;
    OS_MakeDirectory_Function                *MakeDirectory;
    OS_IsHostMessageQueueEmpty_Function      *IsHostMessageQueueEmpty;
    OS_BeginHostMessageQueueRead_Function    *BeginHostMessageQueueRead;
    OS_EndHostMessageQueueRead_Function      *EndHostMessageQueueRead;
    OS_BeginPlayerMessageQueueWrite_Function *BeginPlayerMessageQueueWrite;
    OS_EndPlayerMessageQueueWrite_Function   *EndPlayerMessageQueueWrite;
};

global OS_State *os;


//~ NOTE(fakhri): app layer interface

// NOTE(fakhri): permanant load, called once at the start of the program
#define APP_PermanantLoad(name) void name(OS_State *_os, Game_State *game_state)
typedef APP_PermanantLoad(APP_PermanentLoad_Function);
internal APP_PermanantLoad(APP_PermanentLoadStub) {}

// NOTE(fakhri): hot load, called each time we hot reload
#define APP_HotLoad(name) void name(OS_State *_os)
typedef APP_HotLoad(APP_HotLoad_Function);
internal APP_HotLoad(APP_HotLoadStub) {}

// NOTE(fakhri): called when a shader needs hot reloading
#define APP_HotLoadShader(name) void name(M_Arena *arena, String8 shader_name, Game_State *game_state)
typedef APP_HotLoadShader(APP_HotLoadShader_Function);
internal APP_HotLoadShader(APP_HotLoadShaderStub) {}

// NOTE(fakhri): called when we hot unload
#define APP_HotUnload(name) void name(void)
typedef APP_HotUnload(APP_HotUnload_Function);
internal APP_HotUnload(APP_HotUnloadStub) {}

// NOTE(fakhri): called each frame to do the update and the rendering
#define APP_UpdateAndRender(name) void name(Game_State *game_state, f32 dt)
typedef APP_UpdateAndRender(APP_UpdateAndRender_Function);
internal APP_UpdateAndRender(APP_UpdateAndRenderStub) {}


#endif //OS_STATE_H
