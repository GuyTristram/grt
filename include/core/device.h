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
	Device();
	~Device();

	void swap();

	virtual int width() const;
	virtual int height() const;

private:
	virtual void do_bind();
    SharedPtr< DeviceImpl > m_impl;
};

#endif //BRAMBLE_DEVICE_H
