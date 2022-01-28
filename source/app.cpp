#include "glad/glad.h" 
#include "glad/glad.c"
#include "GL/wglext.h"

#include "language_layer.h"
#include "maths.h"
#include "memory.h"
#include "strings.h"
#include "perlin.h"
#include "app.h"
#include "os.h"

#include "language_layer.c"
#include "memory.c"
#include "strings.c"
#include "perlin.c"
#include "os.c"
#include "shader.cpp"

// TODO(fakhri): i don't really like the way we currently do input hanlding probably we need something better
internal
void handle_events(Game_State *game_state)
{
    OS_Event *event = 0;
    
    while(OS_GetNextEvent(&event))
    {
        OS_EatEvent(event);
    }
}

extern "C"
{
    APP_PERMANENT_LOAD
    {
        os = os_;
        gladLoadGL();
    }
    
    APP_HOT_LOAD
    {
        os = os_;
        gladLoadGL();
    }
    
    APP_HOT_UNLOAD {
    }
    
    APP_UPDATE
    {
        handle_events(game_state);
        
        glClearColor(0.f, .8f, .8f, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        
        os->RefreshScreen();
    }
}