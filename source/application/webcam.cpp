#include "webcam.h"
#include "thirdparty/directshow_camera/uvc_camera.h"

using namespace DirectShowCamera;

WebCam::WebCam()
{
	
}

WebCam::~WebCam()
{
	if (cameraPtr)
	{
		Shutdown();
	}
}

void WebCam::Initialize()
{
	if (!cameraPtr)
		cameraPtr = (void*) (new UVCCamera());
}

void WebCam::Shutdown()
{
	if (cameraPtr)
	{
		Stop();
		texture.Destroy();

		delete ((UVCCamera*)cameraPtr);
		cameraPtr = nullptr;
	}
}

void WebCam::Start()
{
	if (bStarted)
		return;

	UVCCamera& camera = *((UVCCamera*) cameraPtr);

	std::vector<CameraDevice> deviceList = camera.getCameras();
	if (deviceList.size() > 0)
	{
		bStarted = true;
		camera.open(camera.getCameras()[0]);
		camera.startCapture();
	}
}

void WebCam::Stop()
{
	if (bStarted)
	{
		bStarted = false;

		UVCCamera& camera = *((UVCCamera*) cameraPtr);
		camera.close();
	}
}

void WebCam::CaptureFrame()
{
	UVCCamera& camera = *((UVCCamera*) cameraPtr);

	int width = camera.getWidth();
	int height = camera.getHeight();
	int channels = camera.getNumOfBytePerPixel();

	if (!bStarted || channels != 3 && channels != 4)
	{
		return;
	}

	if (!texture.textureId)
	{
		texture.Initialize();
		texture.SetSize(camera.getWidth(), height);
	}

	size_t desiredSizeRGBA = camera.getWidth() * height * 4;
	if (buffer.size() != desiredSizeRGBA)
	{
		buffer.resize(desiredSizeRGBA);
		texture.SetSize(camera.getWidth(), height);
	}

	// we load 3 or 4 channels regardless - the colors come in BGR and we need to swizzle anyway
	int numBytes;
	if (camera.getFrame(buffer.data(), &numBytes))
	{
		// We must add the alpha channel
		GLubyte* pBufStart = buffer.data();
		GLubyte* pTexStart = texture.glData.data();

		size_t numPixels = camera.getWidth() * height;
		if (channels == 3)
		{
			for (size_t i = 0; i < numPixels; i++)
			{
				GLubyte* pBuf = pBufStart + i * 3;
				GLubyte* pTex = pTexStart + i * 4;

				*(pTex) = *(pBuf + 2);	// b -> r
				*(++pTex) = *(pBuf + 1);	// g -> g
				*(++pTex) = *(pBuf);	// r -> b
				*(++pTex) = 255;
			}
		}
		else
		{
			for (size_t i = 0; i < numPixels; i++)
			{
				GLubyte* pBuf = pBufStart + i * 4;
				GLubyte* pTex = pTexStart + i * 4;

				*(pTex) = *(pBuf + 2);	// b -> r
				*(++pTex) = *(pBuf + 1);	// g -> g
				*(++pTex) = *(pBuf + 0);	// r -> b
				*(++pTex) = *(pBuf + 3);	// a -> a
			}
		}

		texture.CopyToGPU();
	}
}

std::string WebCam::DebugString()
{
	UVCCamera& camera = *((UVCCamera*) cameraPtr);
	return std::format("WebCam[{}x{}x{}] FPS[{:.0f}]", camera.getWidth(), camera.getHeight(), camera.getNumOfBytePerPixel(), camera.getFPS());
}
