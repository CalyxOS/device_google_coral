/*
 * Copyright 2018 The Android Open Source Project
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

#define LOG_TAG "dumpstate"

#include "DumpstateDevice.h"

#include <android-base/file.h>
#include <android-base/properties.h>
#include <android-base/unique_fd.h>
#include <cutils/properties.h>
#include <hidl/HidlBinderSupport.h>

#include <log/log.h>
#include <string.h>

#define _SVID_SOURCE
#include <dirent.h>

#include "DumpstateUtil.h"

#define MODEM_LOG_PREFIX_PROPERTY "ro.vendor.radio.log_prefix"
#define MODEM_LOG_LOC_PROPERTY "ro.vendor.radio.log_loc"
#define MODEM_LOGGING_SWITCH "persist.vendor.radio.smlog_switch"

#define DIAG_MDLOG_PERSIST_PROPERTY "persist.vendor.sys.modem.diag.mdlog"
#define DIAG_MDLOG_PROPERTY "vendor.sys.modem.diag.mdlog"
#define DIAG_MDLOG_STATUS_PROPERTY "vendor.sys.modem.diag.mdlog_on"

#define DIAG_MDLOG_NUMBER_BUGREPORT "persist.vendor.sys.modem.diag.mdlog_br_num"

#define UFS_BOOTDEVICE "ro.boot.bootdevice"

#define TCPDUMP_NUMBER_BUGREPORT "persist.vendor.tcpdump.log.br_num"
#define TCPDUMP_PERSIST_PROPERTY "persist.vendor.tcpdump.log.alwayson"

#define MODEM_EFS_DUMP_PROPERTY "vendor.sys.modem.diag.efsdump"

#define HW_VERSION_PROPERTY "ro.boot.hardware.revision"

using android::os::dumpstate::CommandOptions;
using android::os::dumpstate::DumpFileToFd;
using android::os::dumpstate::PropertiesHelper;
using android::os::dumpstate::RunCommandToFd;

namespace android {
namespace hardware {
namespace dumpstate {
namespace V1_0 {
namespace implementation {

#define DIAG_LOG_PREFIX "diag_log_"
#define TCPDUMP_LOG_PREFIX "tcpdump"
#define EXTENDED_LOG_PREFIX "extended_log_"

void DumpstateDevice::dumpLogs(int fd, std::string srcDir, std::string destDir,
                               int maxFileNum, const char *logPrefix) {
    struct dirent **dirent_list = NULL;
    int num_entries = scandir(srcDir.c_str(),
                              &dirent_list,
                              0,
                              (int (*)(const struct dirent **, const struct dirent **)) alphasort);
    if (!dirent_list) {
        return;
    } else if (num_entries <= 0) {
        return;
    }

    int copiedFiles = 0;

    for (int i = num_entries - 1; i >= 0; i--) {
        ALOGD("Found %s\n", dirent_list[i]->d_name);

        if (0 != strncmp(dirent_list[i]->d_name, logPrefix, strlen(logPrefix))) {
            continue;
        }

        if ((copiedFiles >= maxFileNum) && (maxFileNum != -1)) {
            ALOGD("Skipped %s\n", dirent_list[i]->d_name);
            continue;
        }

        copiedFiles++;

        CommandOptions options = CommandOptions::WithTimeout(120).Build();
        std::string srcLogFile = srcDir + "/" + dirent_list[i]->d_name;
        std::string destLogFile = destDir + "/" + dirent_list[i]->d_name;

        std::string copyCmd = "/vendor/bin/cp " + srcLogFile + " " + destLogFile;

        ALOGD("Copying %s to %s\n", srcLogFile.c_str(), destLogFile.c_str());
        RunCommandToFd(fd, "CP DIAG LOGS", { "/vendor/bin/sh", "-c", copyCmd.c_str() }, options);
    }

    while (num_entries--) {
        free(dirent_list[num_entries]);
    }

    free(dirent_list);
}

void DumpstateDevice::dumpModem(int fd, int fdModem)
{
    std::string modemLogDir = android::base::GetProperty(MODEM_LOG_LOC_PROPERTY, "");
    if (modemLogDir.empty()) {
        ALOGD("No modem log place is set");
        return;
    }

    std::string filePrefix = android::base::GetProperty(MODEM_LOG_PREFIX_PROPERTY, "");

    if (filePrefix.empty()) {
        ALOGD("Modem log prefix is not set");
        return;
    }

    const std::string modemLogCombined = modemLogDir + "/" + filePrefix + "all.tar";
    const std::string modemLogAllDir = modemLogDir + "/modem_log";

    RunCommandToFd(fd, "MKDIR MODEM LOG", {"/vendor/bin/mkdir", "-p", modemLogAllDir.c_str()}, CommandOptions::WithTimeout(2).Build());

    if (!PropertiesHelper::IsUserBuild()) {
        RunCommandToFd(fd, "MODEM RFS INFO", {"/vendor/bin/find /data/vendor/rfs/mpss/OEMFI/"}, CommandOptions::WithTimeout(2).Build());
        RunCommandToFd(fd, "MODEM DIAG SYSTEM PROPERTIES", {"/vendor/bin/getprop | grep vendor.sys.modem.diag"}, CommandOptions::WithTimeout(2).Build());

        android::base::SetProperty(MODEM_EFS_DUMP_PROPERTY, "true");

        const std::string diagLogDir = "/data/vendor/radio/diag_logs/logs";
        const std::string tcpdumpLogDir = "/data/vendor/tcpdump_logger/logs";
        const std::string extendedLogDir = "/data/vendor/radio/extended_logs";
        const std::vector <std::string> rilAndNetmgrLogs
            {
                "/data/vendor/radio/ril_log0",
                "/data/vendor/radio/ril_log0_old",
                "/data/vendor/radio/ril_log1",
                "/data/vendor/radio/ril_log1_old",
                "/data/vendor/radio/qmi_fw_log",
                "/data/vendor/radio/qmi_fw_log_old",
                "/data/vendor/radio/imsdatadaemon_log",
                "/data/vendor/radio/imsdatadaemon_log_old",
                "/data/vendor/netmgr/netmgr_log",
                "/data/vendor/netmgr/netmgr_log_old",
                "/data/vendor/radio/power_anomaly_data.txt",
                "/data/vendor/radio/diag_logs/diag_trace.txt",
                "/data/vendor/radio/diag_logs/diag_trace_old.txt",
                "/data/vendor/radio/diag_logs/logs/diag_poweron_log.qmdl",
                "/data/vendor/radio/metrics_data",
                "/data/vendor/ssrlog/ssr_log.txt",
                "/data/vendor/ssrlog/ssr_log_old.txt",
                "/data/vendor/rfs/mpss/modem_efs",
                "/sys/kernel/debug/ipa/ipa_statistics_msg"
            };

        bool smlogEnabled = android::base::GetBoolProperty(MODEM_LOGGING_SWITCH, false) && !access("/vendor/bin/smlog_dump", X_OK);
        bool diagLogEnabled = android::base::GetBoolProperty(DIAG_MDLOG_PERSIST_PROPERTY, false);
        bool tcpdumpEnabled = android::base::GetBoolProperty(TCPDUMP_PERSIST_PROPERTY, false);

        if (smlogEnabled) {
            RunCommandToFd(fd, "SMLOG DUMP", {"smlog_dump", "-d", "-o", modemLogAllDir.c_str()}, CommandOptions::WithTimeout(10).Build());
        } else if (diagLogEnabled) {
            bool diagLogStarted = android::base::GetBoolProperty( DIAG_MDLOG_STATUS_PROPERTY, false);

            if (diagLogStarted) {
                android::base::SetProperty(DIAG_MDLOG_PROPERTY, "false");
                ALOGD("Stopping diag_mdlog...\n");
                if (android::base::WaitForProperty(DIAG_MDLOG_STATUS_PROPERTY, "false", std::chrono::seconds(20))) {
                    ALOGD("diag_mdlog exited");
                } else {
                    ALOGE("Waited mdlog timeout after 20 second");
                }
            } else {
                ALOGD("diag_mdlog is not running");
            }

            dumpLogs(fd, diagLogDir, modemLogAllDir, android::base::GetIntProperty(DIAG_MDLOG_NUMBER_BUGREPORT, 100), DIAG_LOG_PREFIX);

            if (diagLogStarted) {
                ALOGD("Restarting diag_mdlog...");
                android::base::SetProperty(DIAG_MDLOG_PROPERTY, "true");
            }
        }

        if (tcpdumpEnabled) {
            dumpLogs(fd, tcpdumpLogDir, modemLogAllDir, android::base::GetIntProperty(TCPDUMP_NUMBER_BUGREPORT, 5), TCPDUMP_LOG_PREFIX);
        }

        for (const auto& logFile : rilAndNetmgrLogs) {
            RunCommandToFd(fd, "CP MODEM LOG", {"/vendor/bin/cp", logFile.c_str(), modemLogAllDir.c_str()}, CommandOptions::WithTimeout(2).Build());
        }

        dumpLogs(fd, extendedLogDir, modemLogAllDir, 100, EXTENDED_LOG_PREFIX);
        android::base::SetProperty(MODEM_EFS_DUMP_PROPERTY, "false");
    }

    RunCommandToFd(fd, "TAR LOG", {"/vendor/bin/tar", "cvf", modemLogCombined.c_str(), "-C", modemLogAllDir.c_str(), "."}, CommandOptions::WithTimeout(120).Build());
    RunCommandToFd(fd, "CHG PERM", {"/vendor/bin/chmod", "a+w", modemLogCombined.c_str()}, CommandOptions::WithTimeout(2).Build());

    std::vector<uint8_t> buffer(65536);
    android::base::unique_fd fdLog(TEMP_FAILURE_RETRY(open(modemLogCombined.c_str(), O_RDONLY | O_CLOEXEC | O_NONBLOCK)));

    if (fdLog >= 0) {
        while (1) {
            ssize_t bytes_read = TEMP_FAILURE_RETRY(read(fdLog, buffer.data(), buffer.size()));

            if (bytes_read == 0) {
                break;
            } else if (bytes_read < 0) {
                ALOGD("read(%s): %s\n", modemLogCombined.c_str(), strerror(errno));
                break;
            }

            ssize_t result = TEMP_FAILURE_RETRY(write(fdModem, buffer.data(), bytes_read));

            if (result != bytes_read) {
                ALOGD("Failed to write %ld bytes, actually written: %ld", bytes_read, result);
                break;
            }
        }
    }

    RunCommandToFd(fd, "RM MODEM DIR", { "/vendor/bin/rm", "-r", modemLogAllDir.c_str()}, CommandOptions::WithTimeout(2).Build());
    RunCommandToFd(fd, "RM LOG", { "/vendor/bin/rm", modemLogCombined.c_str()}, CommandOptions::WithTimeout(2).Build());
}

static void DumpTouch(int fd) {
    const char touch_spi_path[] = "/sys/class/spi_master/spi1/spi1.0";
    char cmd[256];

    snprintf(cmd, sizeof(cmd), "%s/appid", touch_spi_path);
    if (!access(cmd, R_OK)) {
        // Touch firmware version
        DumpFileToFd(fd, "STM touch firmware version", cmd);

        // Touch controller status
        snprintf(cmd, sizeof(cmd), "%s/status", touch_spi_path);
        DumpFileToFd(fd, "STM touch status", cmd);

        // Mutual raw data
        snprintf(cmd, sizeof(cmd),
                 "echo 13 00 > %s/stm_fts_cmd && cat %s/stm_fts_cmd",
                 touch_spi_path, touch_spi_path);
        RunCommandToFd(fd, "Mutual Raw", {"/vendor/bin/sh", "-c", cmd});

        // Mutual strength data
        snprintf(cmd, sizeof(cmd),
                 "echo 17 > %s/stm_fts_cmd && cat %s/stm_fts_cmd",
                 touch_spi_path, touch_spi_path);
        RunCommandToFd(fd, "Mutual Strength", {"/vendor/bin/sh", "-c", cmd});

        // Self raw data
        snprintf(cmd, sizeof(cmd),
                 "echo 15 00 > %s/stm_fts_cmd && cat %s/stm_fts_cmd",
                 touch_spi_path, touch_spi_path);
        RunCommandToFd(fd, "Self Raw", {"/vendor/bin/sh", "-c", cmd});
    }

    if (!access("/proc/fts/driver_test", R_OK)) {
        RunCommandToFd(fd, "Mutual Raw Data",
                       {"/vendor/bin/sh", "-c",
                        "echo 23 00 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Mutual Baseline Data",
                       {"/vendor/bin/sh", "-c",
                        "echo 23 03 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Mutual Strength Data",
                       {"/vendor/bin/sh", "-c",
                        "echo 23 02 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Self Raw Data",
                       {"/vendor/bin/sh", "-c",
                        "echo 24 00 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Self Baseline Data",
                       {"/vendor/bin/sh", "-c",
                        "echo 24 03 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Self Strength Data",
                       {"/vendor/bin/sh", "-c",
                        "echo 24 02 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Mutual Compensation",
                       {"/vendor/bin/sh", "-c",
                        "echo 32 10 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Self Compensation",
                       {"/vendor/bin/sh", "-c",
                        "echo 33 12 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Golden Mutual Raw Data",
                       {"/vendor/bin/sh", "-c",
                        "echo 34 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
    }
}

static void DumpDisplay(int fd) {
    DumpFileToFd(fd, "PANEL VENDOR NAME", "/sys/class/panel_info/panel0/panel_vendor_name");
    DumpFileToFd(fd, "PANEL SN", "/sys/class/panel_info/panel0/serial_number");
    DumpFileToFd(fd, "PANEL EXTRA INFO", "/sys/class/panel_info/panel0/panel_extinfo");

    const std::string pmic_regmap_path = "/sys/kernel/debug/regmap/spmi0-05";
    using android::base::WriteStringToFile;

    if (WriteStringToFile("0x80", pmic_regmap_path + "/count", true) &&
            WriteStringToFile("0xE000", pmic_regmap_path + "/address", true)) {
        DumpFileToFd(fd, "OLEDB Register Dump", pmic_regmap_path + "/data");
    } else {
	dprintf(fd, "Unable to print OLEDB Register Dump\n");
    }

    if (WriteStringToFile("0x80", pmic_regmap_path + "/count", true) &&
            WriteStringToFile("0xDE00", pmic_regmap_path + "/address", true)) {
        DumpFileToFd(fd, "ELVDD Register Dump", pmic_regmap_path + "/data");
    } else {
	dprintf(fd, "Unable to print ELVDD Register Dump\n");
    }

    if (WriteStringToFile("0x60", pmic_regmap_path + "/count", true) &&
            WriteStringToFile("0xDC00", pmic_regmap_path + "/address", true)) {
        DumpFileToFd(fd, "ELVSS Register Dump", pmic_regmap_path + "/data");
    } else {
	dprintf(fd, "Unable to print ELVSS Register Dump\n");
    }
}

static void DumpF2FS(int fd) {
    DumpFileToFd(fd, "F2FS", "/sys/kernel/debug/f2fs/status");
    DumpFileToFd(fd, "F2FS - fragmentation", "/proc/fs/f2fs/dm-3/segment_info");
}

static void DumpUFS(int fd) {
    DumpFileToFd(fd, "UFS model", "/sys/block/sda/device/model");
    DumpFileToFd(fd, "UFS rev", "/sys/block/sda/device/rev");
    DumpFileToFd(fd, "UFS size", "/sys/block/sda/size");
    DumpFileToFd(fd, "UFS show_hba", "/sys/kernel/debug/ufshcd0/show_hba");
    DumpFileToFd(fd, "UFS err_stats", "/sys/kernel/debug/ufshcd0/stats/err_stats");
    DumpFileToFd(fd, "UFS io_stats", "/sys/kernel/debug/ufshcd0/stats/io_stats");
    DumpFileToFd(fd, "UFS req_stats", "/sys/kernel/debug/ufshcd0/stats/req_stats");

    std::string bootdev = android::base::GetProperty(UFS_BOOTDEVICE, "");
    if (!bootdev.empty()) {
        DumpFileToFd(fd, "UFS Slow IO Read", "/sys/devices/platform/soc/" + bootdev + "/slowio_read_cnt");
        DumpFileToFd(fd, "UFS Slow IO Write", "/sys/devices/platform/soc/" + bootdev + "/slowio_write_cnt");
        DumpFileToFd(fd, "UFS Slow IO Unmap", "/sys/devices/platform/soc/" + bootdev + "/slowio_unmap_cnt");
        DumpFileToFd(fd, "UFS Slow IO Sync", "/sys/devices/platform/soc/" + bootdev + "/slowio_sync_cnt");

        std::string ufs_health = "for f in $(find /sys/devices/platform/soc/" + bootdev + "/health -type f); do if [[ -r $f && -f $f ]]; then echo --- $f; cat $f; echo ''; fi; done";
        RunCommandToFd(fd, "UFS health", {"/vendor/bin/sh", "-c", ufs_health.c_str()});
    }
}

static void DumpPower(int fd) {
    RunCommandToFd(fd, "Power Stats Times", {"/vendor/bin/sh", "-c",
                   "echo -n \"Boot: \" && /vendor/bin/uptime -s &&"
                   "echo -n \"Now: \" && date"});
    DumpFileToFd(fd, "Sleep Stats", "/sys/power/system_sleep/stats");
    DumpFileToFd(fd, "Power Management Stats", "/sys/power/rpmh_stats/master_stats");
    DumpFileToFd(fd, "WLAN Power Stats", "/sys/kernel/wlan/power_stats");
}

// Methods from ::android::hardware::dumpstate::V1_0::IDumpstateDevice follow.
Return<void> DumpstateDevice::dumpstateBoard(const hidl_handle& handle) {
    // Exit when dump is completed since this is a lazy HAL.
    addPostCommandTask([]() {
        exit(0);
    });

    if (handle == nullptr || handle->numFds < 1) {
        ALOGE("no FDs\n");
        return Void();
    }

    int fd = handle->data[0];
    if (fd < 0) {
        ALOGE("invalid FD: %d\n", handle->data[0]);
        return Void();
    }

    RunCommandToFd(fd, "Notify modem", {"/vendor/bin/modem_svc", "-s"}, CommandOptions::WithTimeout(1).Build());
    RunCommandToFd(fd, "VENDOR PROPERTIES", {"/vendor/bin/getprop"});
    DumpFileToFd(fd, "SoC serial number", "/sys/devices/soc0/serial_number");
    DumpFileToFd(fd, "CPU present", "/sys/devices/system/cpu/present");
    DumpFileToFd(fd, "CPU online", "/sys/devices/system/cpu/online");
    DumpTouch(fd);
    DumpDisplay(fd);

    DumpF2FS(fd);
    DumpUFS(fd);

    DumpFileToFd(fd, "INTERRUPTS", "/proc/interrupts");

    DumpPower(fd);

    DumpFileToFd(fd, "LL-Stats", "/d/wlan0/ll_stats");
    DumpFileToFd(fd, "WLAN Connect Info", "/d/wlan0/connect_info");
    DumpFileToFd(fd, "WLAN Offload Info", "/d/wlan0/offload_info");
    DumpFileToFd(fd, "WLAN Roaming Stats", "/d/wlan0/roam_stats");
    DumpFileToFd(fd, "ICNSS Stats", "/d/icnss/stats");
    DumpFileToFd(fd, "SMD Log", "/d/ipc_logging/smd/log");
    RunCommandToFd(fd, "ION HEAPS", {"/vendor/bin/sh", "-c", "for d in $(ls -d /d/ion/*); do for f in $(ls $d); do echo --- $d/$f; cat $d/$f; done; done"});
    DumpFileToFd(fd, "dmabuf info", "/d/dma_buf/bufinfo");
    DumpFileToFd(fd, "dmabuf process info", "/d/dma_buf/dmaprocs");
    RunCommandToFd(fd, "Temperatures", {"/vendor/bin/sh", "-c", "for f in /sys/class/thermal/thermal* ; do type=`cat $f/type` ; temp=`cat $f/temp` ; echo \"$type: $temp\" ; done"});
    RunCommandToFd(fd, "Cooling Device Current State", {"/vendor/bin/sh", "-c", "for f in /sys/class/thermal/cooling* ; do type=`cat $f/type` ; temp=`cat $f/cur_state` ; echo \"$type: $temp\" ; done"});
    RunCommandToFd(
        fd, "LMH info",
        {"/vendor/bin/sh", "-c",
         "for f in /sys/bus/platform/drivers/msm_lmh_dcvs/*qcom,limits-dcvs@*/lmh_freq_limit; do "
         "state=`cat $f` ; echo \"$f: $state\" ; done"});
    RunCommandToFd(fd, "CPU time-in-state", {"/vendor/bin/sh", "-c", "for cpu in /sys/devices/system/cpu/cpu*; do f=$cpu/cpufreq/stats/time_in_state; if [ ! -f $f ]; then continue; fi; echo $f:; cat $f; done"});
    RunCommandToFd(fd, "CPU cpuidle", {"/vendor/bin/sh", "-c", "for cpu in /sys/devices/system/cpu/cpu*; do for d in $cpu/cpuidle/state*; do if [ ! -d $d ]; then continue; fi; echo \"$d: `cat $d/name` `cat $d/desc` `cat $d/time` `cat $d/usage`\"; done; done"});
    RunCommandToFd(fd, "Airbrush debug info", {"/vendor/bin/sh", "-c", "for f in `ls /sys/devices/platform/soc/c84000.i2c/i2c-*/*-0066/@(*curr|temperature|vbat)`; do echo \"$f: `cat $f`\" ; done; file=/sys/devices/platform/soc/soc:abc-sm/mapped_chip_state; echo \"$file: `cat $file`\"; file=/sys/devices/platform/soc/soc:abc-sm/state_stats; echo \"$file: `cat $file`\""});
    DumpFileToFd(fd, "MDP xlogs", "/data/vendor/display/mdp_xlog");
    DumpFileToFd(fd, "TCPM logs", "/d/tcpm/usbpd0");
    DumpFileToFd(fd, "PD Engine", "/d/logbuffer/usbpd");
    DumpFileToFd(fd, "PPS", "/d/logbuffer/pps");
    DumpFileToFd(fd, "BMS", "/d/logbuffer/ssoc");
    DumpFileToFd(fd, "smblib", "/d/logbuffer/smblib");
    DumpFileToFd(fd, "batt_ce", "/d/logbuffer/batt_ce");
    DumpFileToFd(fd, "WLC logs", "/d/logbuffer/wireless");
    DumpFileToFd(fd, "ipc-local-ports", "/d/msm_ipc_router/dump_local_ports");
    RunCommandToFd(fd, "USB Device Descriptors", {"/vendor/bin/sh", "-c", "cd /sys/bus/usb/devices/1-1 && cat product && cat bcdDevice; cat descriptors | od -t x1 -w16 -N96"});
    RunCommandToFd(fd, "Power supply properties", {"/vendor/bin/sh", "-c", "for f in `ls /sys/class/power_supply/*/uevent` ; do echo \"------ $f\\n`cat $f`\\n\" ; done"});
    RunCommandToFd(fd, "PMIC Votables", {"/vendor/bin/sh", "-c", "cat /sys/kernel/debug/pmic-votable/*/status"});
    DumpFileToFd(fd, "Charger Stats", "/sys/class/power_supply/battery/charge_details");
    DumpFileToFd(fd, "Maxim FG History", "/dev/maxfg_history");

    std::string hwVersion = android::base::GetProperty(HW_VERSION_PROPERTY, "");
    if (hwVersion == "PROTO1.0" || hwVersion == "PROTO1.1" || hwVersion == "EVT1.0") {
        RunCommandToFd(fd, "Maxim FG registers", {"/vendor/bin/sh", "-c", "cat /d/regmap/1-0036/registers"});
        RunCommandToFd(fd, "Maxim FG NV RAM", {"/vendor/bin/sh", "-c", "cat /d/regmap/1-000b/registers"});
    } else {
        RunCommandToFd(fd, "Maxim FG registers", {"/vendor/bin/sh", "-c", "cat /d/regmap/2-0036/registers"});
        RunCommandToFd(fd, "Maxim FG NV RAM", {"/vendor/bin/sh", "-c", "cat /d/regmap/2-000b/registers"});
    }

    RunCommandToFd(fd, "Google Charger", {"/vendor/bin/sh", "-c", "cd /d/google_charger/; for f in `ls pps_*` ; do echo \"$f: `cat $f`\" ; done"});
    RunCommandToFd(fd, "Google Battery", {"/vendor/bin/sh", "-c", "cd /d/google_battery/; for f in `ls ssoc_*` ; do echo \"$f: `cat $f`\" ; done"});
    DumpFileToFd(fd, "WLC VER", "/sys/devices/platform/soc/880000.i2c/i2c-1/1-0061/version");
    DumpFileToFd(fd, "WLC STATUS", "/sys/devices/platform/soc/880000.i2c/i2c-1/1-0061/status");

    RunCommandToFd(fd, "eSIM Status", {"/vendor/bin/sh", "-c", "od -t x1 /sys/firmware/devicetree/base/chosen/cdt/cdb2/esim"});
    DumpFileToFd(fd, "Modem Stat", "/data/vendor/modem_stat/debug.txt");
    DumpFileToFd(fd, "Pixel trace", "/d/tracing/instances/pixel-trace/trace");

    // Slower dump put later in case stuck the rest of dump
    // Timeout after 3s as TZ log missing EOF
    RunCommandToFd(fd, "QSEE logs", {"/vendor/bin/sh", "-c", "/vendor/bin/timeout 3 cat /d/tzdbg/qsee_log"});
    RunCommandToFd(fd, "TZ logs", {"/vendor/bin/sh", "-c", "/vendor/bin/timeout 3 cat /d/tzdbg/log"});
    RunCommandToFd(fd, "HYP logs", {"/vendor/bin/sh", "-c", "/vendor/bin/timeout 3 cat /d/tzdbg/hyp_log"});
    if (handle->numFds < 2) {
        ALOGE("no FD for modem\n");
    } else {
        int fdModem = handle->data[1];
        dumpModem(fd, fdModem);
    }

    // Citadel info (only enabled on -eng and -userdebug builds)
    if (!PropertiesHelper::IsUserBuild()) {
        RunCommandToFd(fd, "Citadel ID", {"/vendor/bin/hw/citadel_updater", "--id"});
        RunCommandToFd(fd, "Citadel VER", {"/vendor/bin/hw/citadel_updater", "-lv"});
        RunCommandToFd(fd, "Citadel SELFTEST", {"/vendor/bin/hw/citadel_updater", "--selftest"});
    }

    // Keep this at the end as very long on not for humans
    DumpFileToFd(fd, "WLAN FW Log Symbol Table", "/vendor/firmware/Data.msc");

    // Report Knowles framework info
    RunCommandToFd(fd, "KN version", {"/vendor/bin/sh", "-c", "for f in `ls -d /sys/devices/platform/soc/a8c000.spi/spi_master/spi5/spi5.0/iaxxx/*_version` ; do echo \"------ $f\\n`cat $f`\\n\" ; done"});
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace dumpstate
}  // namespace hardware
}  // namespace android
