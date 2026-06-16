#pragma once

#include "domain/alert/SecurityAlert.hpp"

#include <functional>
#include <string>
#include <vector>

namespace domain {

struct ZapUpdateInfo {
    std::string localVersion;
    std::string latestVersion;
    std::string releaseUrl;
    bool updateAvailable{false};
};

class IZapGateway {
public:
    virtual ~IZapGateway() = default;

    virtual bool isDaemonRunning() const = 0;
    virtual void boot() = 0;
    virtual void shutdown() = 0;

    virtual void checkConnection(std::function<void(bool ok, std::string version)> callback) = 0;
    virtual void accessUrl(const std::string& url, std::function<void(bool ok, std::string error)> callback) = 0;
    virtual void startAjaxSpider(const std::string& url, std::function<void(bool ok, std::string message)> callback) = 0;
    virtual void startActiveScan(const std::string& url, std::function<void(bool ok, std::string message)> callback) = 0;
    virtual void stopScans() = 0;
    virtual void ajaxSpiderStatus(std::function<void(std::string status)> callback) = 0;
    virtual void activeScanStatus(std::function<void(std::string status)> callback) = 0;
    virtual void fetchAlerts(std::function<void(std::vector<SecurityAlert>, std::string error)> callback) = 0;
    virtual void checkForUpdates(std::function<void(ZapUpdateInfo)> callback) = 0;
    virtual void runUpdateScript(std::function<void(bool ok, std::string error)> callback) = 0;
};

}  // namespace domain
