#include "gles.h"

#include "log.h"

EGLDisplay egl_display;
EGLSurface egl_surface;
EGLContext egl_context;
EGLConfig egl_config;


#define EGL_ZBITS 16
static EGLint const config_attribute_list[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, EGL_ZBITS,
        EGL_NONE
};

static const EGLint context_attribute_list[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
};

static EGLint window_attribute_list[] = {
        EGL_NONE
};

bool InitOGLESContext()
{
    EGLint egl_major, egl_minor;

    egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if ( egl_display == EGL_NO_DISPLAY)
    {
        Log( LogError, "InitOGLESContext: No display found!" );
        return false;
    }

    if ( !eglInitialize( egl_display, &egl_major, &egl_minor ))
    {
        Log( LogError, "InitOGLESContext: eglInitialize failed!" );
        return false;
    }

    Log( "InitOGLESContext: EGL Version: %s", eglQueryString( egl_display, EGL_VERSION ));
    Log( "InitOGLESContext: EGL Vendor: %s", eglQueryString( egl_display, EGL_VENDOR ));
    Log( "InitOGLESContext: EGL Extensions: %s", eglQueryString( egl_display, EGL_EXTENSIONS ));

    //don't use eglChooseConfig! EGL code pushes in multisample flags, wasted on the time warped frontbuffer
    enum
    {
        MAX_CONFIGS = 1024
    };

    EGLConfig configs[MAX_CONFIGS];
    EGLint numConfigs = 0;
    if ( !eglGetConfigs( egl_display, configs, MAX_CONFIGS, &numConfigs ))
    {
        Log( LogError, "InitOGLESContext: eglGetConfigs failed!" );
        return false;
    }

    for ( int i = 0; i < numConfigs; i++ )
    {
        EGLint value = 0;

        eglGetConfigAttrib( egl_display, configs[ i ], EGL_RENDERABLE_TYPE, &value );
        if ((value & EGL_OPENGL_ES3_BIT) != EGL_OPENGL_ES3_BIT )
        {
            continue;
        }

        // Without EGL_KHR_surfaceless_context, the config needs to support both pbuffers and window surfaces.
        eglGetConfigAttrib( egl_display, configs[ i ], EGL_SURFACE_TYPE, &value );
        if ((value & (EGL_WINDOW_BIT | EGL_PBUFFER_BIT)) != (EGL_WINDOW_BIT | EGL_PBUFFER_BIT))
        {
            continue;
        }

        int j = 0;
        for ( ; config_attribute_list[ j ] != EGL_NONE; j += 2 )
        {
            eglGetConfigAttrib( egl_display, configs[ i ], config_attribute_list[ j ], &value );
            if ( value != config_attribute_list[ j + 1 ] )
            {
                break;
            }
        }

        if ( config_attribute_list[ j ] == EGL_NONE )
        {
            egl_config = configs[ i ];
            break;
        }
    }

    if ( egl_config == 0 )
    {
        Log( LogError, "InitOGLESContext: Failed to find egl config!" );
        return false;
    }

    egl_context = eglCreateContext( egl_display, egl_config, EGL_NO_CONTEXT, context_attribute_list );
    if ( egl_context == EGL_NO_CONTEXT)
    {
        Log( LogError, "eglCreateContext failed: 0x%08X", eglGetError());
        return false;
    }

    Log( "InitOGLESContext: Created context %p", egl_context );

    const EGLint surfaceAttribs[] = {EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE};
    egl_surface = eglCreatePbufferSurface( egl_display, egl_config, surfaceAttribs );
    if ( egl_surface == EGL_NO_SURFACE)
    {
        Log( LogError, "eglCreatePbufferSurface failed: 0x%08X", eglGetError());

        eglDestroyContext( egl_display, egl_context );
        egl_context = EGL_NO_CONTEXT;

        return false;
    }

    if ( !eglMakeCurrent( egl_display, egl_surface, egl_surface, egl_context ))
    {
        Log( LogError, "InitOGLESContext: eglMakeCurrent() failed: 0x%08X", eglGetError());
        return false;
    }

    Log( "GL Vendor: \"%s\"\n", glGetString( GL_VENDOR ));
    Log( "GL Renderer: \"%s\"\n", glGetString( GL_RENDERER ));
    Log( "GL Version: \"%s\"\n", glGetString( GL_VERSION ));
    Log( "GL Extensions: \"%s\"\n", glGetString( GL_EXTENSIONS ));

    return true;
}

bool DestroyOGLESContext()
{
    if( egl_display != 0 )
    {
        eglMakeCurrent( egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
    }

    if( egl_context != EGL_NO_CONTEXT )
    {
        eglDestroyContext( egl_display, egl_context );
    }

    if( egl_surface != EGL_NO_SURFACE )
    {
        eglDestroySurface( egl_display, egl_surface );
    }

    eglTerminate(egl_display);

    return true;
}
