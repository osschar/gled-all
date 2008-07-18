#include "pbuffer.h"

// tmp:
#include <stdio.h>

PBuffer::PBuffer(int w, int h)
{
	initialized=false;

	width  = w;
	height = h;

	dpy = glXGetCurrentDisplay();
	if (dpy == NULL)
	{
		throw "unable to get display";
	}
	scrnum = DefaultScreen( dpy );
	FBRC = glXGetCurrentContext();
	if (!FBRC)
	{
		throw "unable to get render context";
	}
	FBDC = glXGetCurrentDrawable();
	if (!FBDC)
	{
		throw "unable to get drawable";
	}

	Create();
}

PBuffer::~PBuffer()
{
	Destroy();
}

void PBuffer::Use()
{
	// make sure the pbuffer has been initialized
	if (!initialized)
	{
		throw "pbuffer is not initialized";
	}
	// make sure we haven't lost our pbuffer due to a display mode change

	// resize view port. generally you'll want to set this to the
	// size of your pbuffer so that you render to the entire pbuffer
	// but there are cases where you might want to render to just a
	// sub-region of the pbuffer.
	glXMakeContextCurrent(dpy, PBDC, PBDC, PBRC);

	glViewport(0, 0, width, height);

	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT);
}

void PBuffer::Release()
{
	// make sure the pbuffer has been initialized
	if ( !initialized )
	{
		throw "pbuffer is not initialized";
	}
	// make sure we haven't lost our pbuffer due to a display mode change

	glXMakeContextCurrent(dpy, FBDC, FBDC, FBRC);
}

void PBuffer::Create()
{
	if(initialized)
	{
		Destroy();
	}

	// define the minimum pixel format requirements we will need for our pbuffer
	// a pbuffer is just like a frame buffer, it can have a depth buffer associated
	// with it and it can be double buffered.
	/*int attr[] =
	{
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT | GLX_WINDOW_BIT,
		GLX_DOUBLEBUFFER, False,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_ALPHA_SIZE, 1,
		GLX_DEPTH_SIZE, 1,
		0 // zero terminates the list
	};*/

	int attrib[] =
	{
		GLX_DOUBLEBUFFER,  False,
		GLX_RED_SIZE,      8,
		GLX_GREEN_SIZE,    8,
		GLX_BLUE_SIZE,     8,
		GLX_ALPHA_SIZE,    8, // 8
		GLX_STENCIL_SIZE,  1, // 1
		GLX_DEPTH_SIZE,    24, // 24
		GLX_RENDER_TYPE,   GLX_RGBA_BIT,
		GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT | GLX_WINDOW_BIT,
		None
	};

	int PBattrib[] =
	{
		GLX_PBUFFER_WIDTH,   width,
		GLX_PBUFFER_HEIGHT,  height,
		// GLX_LARGEST_PBUFFER, False,
		GLX_LARGEST_PBUFFER, True,
		None
	};

	// choose a pixel format that meets our minimum requirements
	int count = 0;
	//GLXFBConfigSGIX *config=
	//	glXChooseFBConfigSGIX(dpy, scrnum, attrib, &count);

	GLXFBConfig *config=
		glXChooseFBConfig(dpy, scrnum, attrib, &count);

	if(config == NULL || count == 0)
	{
		throw "no fitting pbuffer pixel format found";
	}

	// allocate the pbuffer
	//PBDC=glXCreateGLXPbufferSGIX(dpy, config[0], width, height, PBattrib);
	//PBRC=glXCreateContextWithConfigSGIX(dpy, config[0], GLX_RGBA_TYPE_SGIX, FBRC, true);

	PBDC=glXCreatePbuffer(dpy, config[0], PBattrib);
	PBRC=glXCreateNewContext(dpy, config[0], GLX_RGBA_TYPE, FBRC, true);

	{
	  unsigned int w,h;
	  glXQueryDrawable(dpy, PBDC, GLX_WIDTH,  &w);
	  glXQueryDrawable(dpy, PBDC, GLX_HEIGHT, &h);
          printf("GLX Pbuffer w=%u h=%u\n", w, h);
	}

	XFree(config);

	initialized=true;
}

void PBuffer::Destroy()
{
	// make sure the pbuffer has been initialized
	if ( !initialized )
	{
		throw "pbuffer is not initialized";
	}

	Release();

	glXDestroyContext(dpy, PBRC);
	glXDestroyPbuffer(dpy, PBDC);

	initialized = false;
}
