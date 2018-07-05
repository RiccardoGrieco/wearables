/*
 * Copyright (C) 2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * GNU Lesser General Public License v2.1 or any later version.
 */

#ifndef XSENS_MVN_DRIVER_H
#define XSENS_MVN_DRIVER_H

#include "XSensCalibrationQualities.h"

#include "IXsensMVNControl.h"

#include <array>
#include <map>
#include <vector>

namespace xsensmvn {

    class XSensMVNDriver;
    class XSensMVNDriverImpl;

    /* ------------ *
     *  Definitions *
     * ------------ */

    using Vector3 = std::array<double, 3>;
    using Quaternion = std::array<double, 4>;
    using bodyDimensions = std::map<std::string, double>;

    struct LinkData
    {
        std::string name;
        Vector3 position;
        Vector3 linearVelocity;
        Vector3 linearAcceleration;
        Quaternion orientation;
        Vector3 angularVelocity;
        Vector3 angularAcceleration;
    };

    struct SensorData
    {
        std::string name;
        Vector3 position;
        Quaternion orientation;
        Vector3 freeBodyAcceleration;
        Vector3 magneticField;
    };

    struct JointData
    {
        std::string name;
        Vector3 jointAngles;
    };

    struct DriverDataSample
    {
        std::string suitName;
        double relativeTime; // [s]
        double absoluteTime; // [s]
        std::vector<LinkData> links;
        std::vector<SensorData> sensors;
        std::vector<JointData> joints;
    };

    struct DriverDataStreamConfig
    {
        bool enableLinkData;
        bool enableSensorData;
        bool enableJointData;
    };

    struct DriverConfiguration
    {
        const std::string licensePath;
        const std::string suitConfiguration;
        const std::string acquisitionScenario;
        const std::string defaultCalibrationType;
        const xsensmvn::CalibrationQuality minimumRequiredCalibrationQuality;
        const int scanTimeout;
        const bodyDimensions bodyDimensions;
        const DriverDataStreamConfig dataStreamConfiguration;
    };

    enum class DriverStatus
    {
        Disconnected = 0,
        Scanning,
        Connected,
        Calibrating,
        CalibratedAndReadyToRecord,
        Recording,
        Unknown,
    };

} // namespace xsensmvn

class xsensmvn::XSensMVNDriver : public IXsensMVNControl
{
private:
    /* ---------- *
     *  Variables *
     * ---------- */

    // Pointer to the driver implementation
    std::unique_ptr<XSensMVNDriverImpl> m_pimpl;

    // Struct to internally store (cache) a single data sample
    DriverDataSample m_dataSample;

public:
    /* -------------------------- *
     *  Construtors / Destructors *
     * -------------------------- */

    XSensMVNDriver() = delete;
    XSensMVNDriver(const xsensmvn::DriverConfiguration conf);
    ~XSensMVNDriver() override;

    /* ---------- *
     *  Functions *
     * ---------- */

    // Prevent copy
    XSensMVNDriver(const XSensMVNDriver& other) = delete;
    XSensMVNDriver& operator=(const XSensMVNDriver& other) = delete;

    // Driver opening and closing
    bool configureAndConnect();
    bool terminate();

    // Minimum calibration quality considered to be satisfactory set/get
    bool setMinimumAcceptableCalibrationQuality(const xsensmvn::CalibrationQuality quality) const;
    const xsensmvn::CalibrationQuality& getMinimumAcceptableCalibrationQuality() const;

    // Move the data from the implementation internal memory to the local one
    void cacheData();

    // Data accessor
    const DriverDataSample& getDataSample() const;
    const std::vector<LinkData>& getLinkDataSample() const;
    const std::vector<SensorData>& getSensorDataSample() const;
    const std::vector<JointData>& getJointDataSample() const;

    // Metadata accessor
    std::string getSuitName() const;
    double getSampleRelativeTime() const;
    double getSampleAbsoluteTime() const;

    // Labels accessor
    std::vector<std::string> getSuitLinkLabels() const;
    std::vector<std::string> getSuitSensorLabels() const;
    std::vector<std::string> getSuitJointLabels() const;

    // Status get
    xsensmvn::DriverStatus getStatus() const;

    /* --------------------------- *
     *  IXsensMVNControl Interface *
     * --------------------------- */

    bool startAcquisition() override;
    bool stopAcquisition() override;

    bool calibrate(const std::string& calibrationType = {}) override;
    bool abortCalibration() override;

    // Body-dimensions set/get
    bool setBodyDimensions(const std::map<std::string, double>& bodyDimensions) override;
    bool getBodyDimensions(std::map<std::string, double>& dimensions) const override;
    bool getBodyDimension(const std::string bodyName, double& dimension) const override;
};

#endif // XSENS_MVN_DRIVER_H