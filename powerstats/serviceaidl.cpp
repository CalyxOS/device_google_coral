/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "android.hardware.power.stats-service.pixel"

#include <PowerStatsAidl.h>
#include <dataproviders/PowerStatsEnergyConsumer.h>
#include <dataproviders/IioEnergyMeterDataProvider.h>
#include <dataproviders/GenericStateResidencyDataProvider.h>

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <log/log.h>

using aidl::android::hardware::power::stats::EnergyConsumerId;
using aidl::android::hardware::power::stats::IioEnergyMeterDataProvider;
using aidl::android::hardware::power::stats::PowerStats;
using aidl::android::hardware::power::stats::PowerStatsEnergyConsumer;
using aidl::android::hardware::power::stats::GenericStateResidencyDataProvider;

void addNFCStats(std::shared_ptr<PowerStats> p) {
    GenericStateResidencyDataProvider::StateResidencyConfig nfcStateConfig = {
        .entryCountSupported = true,
        .entryCountPrefix = "Cumulative count:",
        .totalTimeSupported = true,
        .totalTimePrefix = "Cumulative duration msec:",
        .lastEntrySupported = true,
        .lastEntryPrefix = "Last entry timestamp msec:"
    };
    std::vector<std::pair<std::string, std::string>> nfcStateHeaders = {
        std::make_pair("Idle", "Idle mode:"),
        std::make_pair("Active", "Active mode:"),
        std::make_pair("Active-RW", "Active Reader/Writer mode:"),
    };

    std::vector<GenericStateResidencyDataProvider::PowerEntityConfig> cfgs;
    cfgs.emplace_back(generateGenericStateResidencyConfigs(nfcStateConfig, nfcStateHeaders), "NFC");
    android::sp<GenericStateResidencyDataProvider> nfcSdp = new GenericStateResidencyDataProvider(
            "/sys/class/misc/st21nfc/device/power_stats", cfgs);
    p->addStateResidencyDataProvider(nfcSdp);
}

void addDisplayEnergyConsumer(std::shared_ptr<PowerStats> p) {
    p->addEnergyConsumer(PowerStatsEnergyConsumer::createMeterConsumer(p, EnergyConsumerId::DISPLAY,
        {"PPVAR_VPH_PWR_OLED"}));
}

void setEnergyMeter(std::shared_ptr<PowerStats> p) {
    p->setEnergyMeterDataProvider(std::make_unique<IioEnergyMeterDataProvider>("microchip,pac1934"));
}

int main() {
    LOG(INFO) << "Pixel PowerStats HAL AIDL Service is starting.";

    // single thread
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    std::shared_ptr<PowerStats> p = ndk::SharedRefBase::make<PowerStats>();
    if (!p) {
        return EXIT_FAILURE;
    }

    setEnergyMeter(p);
    addNFCStats(p);
    addDisplayEnergyConsumer(p); // Must be done after setEnergyMeter

    const std::string instance = std::string() + PowerStats::descriptor + "/default";
    binder_status_t status = AServiceManager_addService(p->asBinder().get(), instance.c_str());
    LOG_ALWAYS_FATAL_IF(status != STATUS_OK);

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
