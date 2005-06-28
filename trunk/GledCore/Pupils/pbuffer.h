#ifndef PBUFFER_H
#define PBUFFER_H

#include <GL/glx.h>

class PBuffer
{
public:
	PBuffer(int width, int height);
	virtual ~PBuffer();

	void Use();
	void Release();

	int const get_width() const { return width; }
	int const get_height() const { return height; }

private:
	void Create();
	void Destroy();

protected:
	int width;
	int height;

	bool initialized;

	Display		*dpy;
	int		scrnum;
	GLXContext	PBRC;
	GLXPbuffer	PBDC;

	GLXContext	FBRC;
	GLXDrawable	FBDC;
};

#endif/*PBUFFER_H*/
