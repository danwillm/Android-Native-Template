#include <atomic>
#include <stdexcept>
#include <thread>

#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/native_window.h>
#include <jni.h>

#include <sys/system_properties.h>

#include "gles.h"
#include "log.h"

struct AndroidAppState
{
    std::atomic<bool> b_app_running = false;

    ANativeWindow *p_native_window = nullptr;

    int npx_width;
    int npx_height;
};


struct android_app *g_app;
AndroidAppState g_app_state;

void app_handle_cmd( struct android_app *app, int32_t cmd )
{
    switch ( cmd )
    {
        case APP_CMD_START:
        {
            Log( "[AndroidActivity] APP_CMD_START" );
            g_app = app;

            break;
        }

        case APP_CMD_DESTROY:
        {
            Log( "[AndroidActivity] APP_CMD_DESTROY" );
            g_app_state.p_native_window = nullptr;

            break;
        }

        case APP_CMD_INIT_WINDOW:
        {
            Log( "[AndroidActivity] APP_CMD_INIT_WINDOW" );
            g_app_state.p_native_window = app->window;

            break;
        }

        case APP_CMD_TERM_WINDOW:
        {
            Log( "[AndroidActivity] APP_CMD_TERM_WINDOW" );
            g_app_state.p_native_window = nullptr;

            break;
        }

        case APP_CMD_PAUSE:
        {
            Log( "[AndroidActivity] APP_CMD_PAUSE" );
            g_app_state.b_app_running = false;

            break;
        }

        case APP_CMD_STOP:
        {
            Log( "[AndroidActivity] APP_CMD_STOP" );

            break;
        }

        case APP_CMD_RESUME:
        {
            Log( "[AndroidActivity] APP_CMD_RESUME" );
            g_app_state.b_app_running = true;

            break;
        }

        default:
        {
            Log( LogWarning, "[AndroidActivity] Event not handled: %d", cmd );
            break;
        }
    }
}
void android_main(struct android_app *app) {
    JNIEnv *Env;

    JavaVM *java_vm = app->activity->vm;
    java_vm->AttachCurrentThread(&Env, nullptr);

    app->userData = nullptr;
    app->onAppCmd = app_handle_cmd;

    if( !InitOGLESContext() )
    {
        throw std::runtime_error( "android_main: Failed to init GLES context" );
    }

    //Setup stuff here
    //...
    //...

    g_app_state.b_app_running = true;

    while ( app->destroyRequested == 0 )
    {
        while( true )
        {
            int events;
            struct android_poll_source *source;

            const int timeoutMilliseconds = (!g_app_state.b_app_running && app->destroyRequested == 0) ? -1 : 0;
            if (ALooper_pollAll(timeoutMilliseconds, nullptr, &events, (void**)&source) < 0) {
                break;
            }

            if ( source != nullptr )
            {
                source->process( app, source );
            }
        }

        //Loop stuff here
        //...
        //...

        Log("HELLO!!!");

        //tick here and remove the sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    DestroyOGLESContext();

    ANativeActivity_finish(app->activity);
    java_vm->DetachCurrentThread();
}