#ifndef BRAMBLE_DEVICE_H
#define BRAMBLE_DEVICE_H

#include "common/shared.h"
#include "common/uncopyable.h"
#include "core/rendertarget.h"

struct InputEvent;

struct DeviceImpl;

class Device : public RenderTarget, Uncopyable
{
public:
	struct Options
	{
		Options( int width = -1, int height = -1, bool fullscreen = false )
		: width(width), height(height), fullscreen(fullscreen) {}
		int width;
		int height;
		bool fullscreen;
	};

	Device( Options const &options = Options() );
	~Device();

	void swap();

    virtual int width() const override;
    virtual int height() const override;
    
    void resize( int width, int height );

private:
    virtual void do_bind() override;
	SharedPtr< DeviceImpl > m_impl;
};

#endif //BRAMBLE_DEVICE_H
