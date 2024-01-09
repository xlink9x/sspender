/*
 * Copyright (C) 2016 Mountassir El Hafi, (mountassirbillah1@gmail.com)
 *
 * Manager.h: Part of sspender
 *
 * sspender is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * sspender is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with sspender.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MANAGER_H_
#define MANAGER_H_

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <unistd.h>

#include "Monitor.h"
#include "constants.h"
#include "utils.h"
#include <sdbus-c++/sdbus-c++.h>

using namespace std;

/*
 * Manager class: sets up devices for monitoring,
 * periodically checks if the machine is idle and
 * suspends the machine.
 */
class Manager {
private:
  Monitor m_monitor;                // machine usage monitor
  vector<string> m_ipsToWatch;      // don't suspend if any of these ips is online
  vector<DiskCfg> m_disksToMonitor; // disk that will monitored
  vector<CpuCfg> m_cpusToMonitor;   // cpus that will be monitored
  int m_checkIfIdleEvery;           // check if the machine is idle every # minutes
  int m_stopMonitoringFor;    // if any of the ip addresses is online, stop monitoring for # minutes
  int m_resetMonitoringAfter; // if the machine was busy for # minutes, reset all monitoring
                              // counters
  int m_suspendAfter;         // suspend the machine if it is idle for # minutes

public:
  Manager()
      : m_checkIfIdleEvery(CHECK_IF_IDLE_EVERY), m_stopMonitoringFor(STOP_MONITORING_FOR),
        m_resetMonitoringAfter(RESET_MONITORING_IF_BUSY_FOR), m_suspendAfter(SUSPEND_AFTER){};

  // start monitoring the system load
  void monitorSystemUsage();

  // the machine will not be suspended if any of these ip address is found online
  void setIpsToWatch(const vector<string> &ipToWatch);

  // the usage of all the passed in disks will be monitored
  void setDisksToMonitor(const vector<DiskCfg> &disksToMonitor);

  // the usage of the cpu will be monitored
  void setCpusToMonitor(const CpuCfg &couConfig);

  // monitoring tuning
  void setTimers(int check_if_idle_every, int stop_monitoring_for, int reset_monitoring_after,
                 int suspend_after);

  bool canBeSuspended();

  void printTheMachineUsage();

  void getTheMachineUsage(double *cpuLoad, double *storageLoad, double *storageRead,
                          double *storageWritten);

protected:
  // check if the machine is idle
  bool isTheMachineIdle();

  // calculate when the machine should be awake and put it to sleep
  void suspendTheMachine();
};

#endif
