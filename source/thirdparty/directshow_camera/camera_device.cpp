#include "camera_device.h"

namespace DirectShowCamera
{

#pragma region constructor and destructor

    /**
     * @brief Constructor
     * @param directShowCamera Directshow Camera
    */
    CameraDevice::CameraDevice(DirectShowCameraDevice* directShowCamera)
    {
        m_friendlyName = directShowCamera->getFriendlyName();
        m_description = directShowCamera->getDescription();
        m_devicePath = directShowCamera->getDevicePath();

        // Get frame size
        std::vector<DirectShowVideoFormat> videoForamts = directShowCamera->getDirectShowVideoFormats();	
        for (int i = 0; i < videoForamts.size(); i++)
        {
            int width = videoForamts[i].getWidth();
            int height = videoForamts[i].getHeight();
            GUID subType = videoForamts[i].getVideoType();

            if (width != 0 && height != 0)
            {
                if (DirectShowVideoFormat::supportRGBConvertion(subType))
                {
                    // RGB

                    // Check existed
                    bool found = false;
                    for (int j = 0; j < m_rgbResolutions.size(); j++) {
                        if (m_rgbResolutions[j].first == width && m_rgbResolutions[j].second == height)
                        {
                            found = true;
                        }
                    }

                    // Add
                    if (!found)
                    {
                        m_rgbResolutions.push_back(std::pair<int, int>(width, height));
                    }
                }
                else if (DirectShowVideoFormat::isMonochrome(subType))
                {
                    // Monochrome 

                    // Check existed
                    bool found = false;
                    for (int j = 0; j < m_monoResolutions.size(); j++) {
                        if (m_monoResolutions[j].first == width && m_monoResolutions[j].second == height)
                        {
                            found = true;
                        }
                    }

                    // Add
                    if (!found)
                    {
                        m_monoResolutions.push_back(std::pair<int, int>(width, height));
                    }
                }
            }
        }

        // Sort
        if (m_monoResolutions.size() > 0) std::sort(m_monoResolutions.begin(), m_monoResolutions.end());
        if (m_rgbResolutions.size() > 0) std::sort(m_rgbResolutions.begin(), m_rgbResolutions.end());
    }

    /**
     * @brief Copy constructor
     * @param cameraDevice CameraDevice
    */
    CameraDevice::CameraDevice(const CameraDevice& cameraDevice)
    {
        *this = cameraDevice;
    }

#pragma endregion constructor and destructor

#pragma region Monochrome

    /**
     * @brief Return true if camera supported monochrome image
     * @return Return true if camera supported monochrome image
    */
    bool CameraDevice::supportMonochrome()
    {
        return m_monoResolutions.size() > 0 ? true : false;
    }

    /**
     * @brief Get the supported monochrome image resolution in (width, height)
     * @return Return (width, height)[]
    */
    std::vector<std::pair<int, int>> CameraDevice::getMonoResolutions()
    {
        return m_monoResolutions;
    }

#pragma endregion Monochrome

#pragma region RGB

    /**
     * @brief Return true if camera supported rgb image.
     * @return Return true if camera supported rgb image.
    */
    bool CameraDevice::supportRGB()
    {
        return m_rgbResolutions.size() > 0 ? true : false;
    }

    /**
     * @brief Get the supported rgb image resolution in (width, height)
     * @return Return (width, height)[]
    */
    std::vector<std::pair<int, int>> CameraDevice::getRGBResolutions()
    {
        return m_rgbResolutions;
    }

    /**
     * @brief Get the supported resolution in (width, height)
     * @return Return (width, height)[]
    */
    std::vector<std::pair<int,int>> CameraDevice::getResolutions()
    {
        if (m_rgbResolutions.size() > 0)
        {
            return m_rgbResolutions;
        }
        else
        {
            return m_monoResolutions;
        }
    }

    /**
     * @brief Check whether resolution existed
     * 
     * @param width Width
     * @param height Height
     * @return Return true if existed.
    */
    bool CameraDevice::containResolution(int width, int height)
    {
        for (int i=0;i< m_monoResolutions.size();i++)
        {
            if (m_monoResolutions[i].first == width && m_monoResolutions[i].second == height)
            {
                return true;
            }
        }

        for (int i = 0; i < m_rgbResolutions.size(); i++)
        {
            if (m_rgbResolutions[i].first == width && m_rgbResolutions[i].second == height)
            {
                return true;
            }
        }

        return false;
    }

#pragma endregion RGB

#pragma region Camera Getter

    /**
     * @brief Get camera friendly name
     * @return Return the camera friendly name
    */
    std::string CameraDevice::getFriendlyName()
    {
        return m_friendlyName;
    }

    /**
     * @brief Get the camera description
     * @return Return camera description
    */
    std::string CameraDevice::getDescription()
    {
        return m_description;
    }

    /**
     * @brief Get the camera device path. It is the camera id.
     * @return Return the camera device path.
    */
    std::string CameraDevice::getDevicePath()
    {
        return m_devicePath;
    }

#pragma endregion Camera Getter

    /**
     * @brief Copy assignment operator
     * @param cameraDevice CameraDevice
     * @return
    */
    CameraDevice& CameraDevice::operator=(const CameraDevice& cameraDevice)
    {
        if (this != &cameraDevice)
        {
            m_friendlyName = cameraDevice.m_friendlyName;
            m_description = cameraDevice.m_description;
            m_devicePath = cameraDevice.m_devicePath;

            m_monoResolutions.clear();
            for (int i = 0; i < cameraDevice.m_monoResolutions.size(); i++)
            {
                m_monoResolutions.push_back(std::pair<int, int>(cameraDevice.m_monoResolutions[i].first, cameraDevice.m_monoResolutions[i].second));
            }

            m_rgbResolutions.clear();
            for (int i = 0; i < cameraDevice.m_rgbResolutions.size(); i++)
            {
                m_rgbResolutions.push_back(std::pair<int, int>(cameraDevice.m_rgbResolutions[i].first, cameraDevice.m_rgbResolutions[i].second));
            }
        }

        return *this;
    }

}