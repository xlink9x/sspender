/*
 * Copyright (C) 2016 Mountassir El Hafi, (mountassirbillah1@gmail.com)
 *
 * Manager.cpp: Part of sspender
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

#include "Manager.h"

void Manager::setIpsToWatch(const vector<string> &ipToWatch) {
  for (size_t i = 0, size = ipToWatch.size(); i < size; ++i) {
    m_ipsToWatch.push_back(ipToWatch[i]);
  }
}

void Manager::setDisksToMonitor(const vector<DiskCfg> &disksToMonitor) {
  for (size_t i = 0, size = disksToMonitor.size(); i < size; ++i) {
    m_disksToMonitor.push_back(disksToMonitor[i]);
  }
}

void Manager::setCpusToMonitor(const CpuCfg &couConfig) { m_cpusToMonitor.push_back(couConfig); }

void Manager::setTimers(int check_if_idle_every, int stop_monitoring_for,
                        int reset_monitoring_after, int suspend_after) {
  m_checkIfIdleEvery = check_if_idle_every;
  m_stopMonitoringFor = stop_monitoring_for;
  m_resetMonitoringAfter = reset_monitoring_after;
  m_suspendAfter = suspend_after;
}

void Manager::monitorSystemUsage() {
  // start monitoring the usage of the passed in devices
  m_monitor.monitorSystemUsage(m_disksToMonitor, m_cpusToMonitor);

  auto idleStartTime = Clock::now();
  auto notIdleStartTime = Clock::now();

  while (true) {
    // check if the machine is idle every # minutes
    std::this_thread::sleep_for(std::chrono::minutes(m_checkIfIdleEvery));

    printHeaderMessage("Checking if clients are online", true);

    bool stayOnline = m_monitor.areClientsConnected(m_ipsToWatch);

    if (stayOnline) {
      cout << "Found clients online, will stop monitoring for " << m_stopMonitoringFor << " mins."
           << endl;

      // if any of the specified IPs is online, reset the counters and
      // stop checking if the machine is idle, note that the usage of
      // the devices will still be monitored by the detached threads
      std::this_thread::sleep_for(std::chrono::minutes(m_stopMonitoringFor));

      idleStartTime = Clock::now();
      notIdleStartTime = Clock::now();

      continue;
    }

    printHeaderMessage("Checking if system is idle", true);

    bool isIdle = isTheMachineIdle();

    double minutesTheMachineBeenBusyFor = getMinutesDuration(notIdleStartTime);
    double minutesTheMachineBeenIdleFor = getMinutesDuration(idleStartTime);

    if (isIdle) {
      cout << "System has been idle during the last " << minutesTheMachineBeenIdleFor
           << " minutes.\n";

      notIdleStartTime = Clock::now();
    } else {
      cout << "System has not been idle during the last " << minutesTheMachineBeenBusyFor
           << " minutes.\n";

      idleStartTime = Clock::now();
    }

    printTheMachineUsage();

    if (canBeSuspended()) {
      // if system is busy for # minutes
      if (minutesTheMachineBeenBusyFor >= m_resetMonitoringAfter) {
        cout << "System was not idle during the last " << m_resetMonitoringAfter
             << " minutes, reseting idle timer.\n";

        idleStartTime = Clock::now();
        notIdleStartTime = Clock::now();
      }

      // if idle for # minutes
      if (minutesTheMachineBeenIdleFor >= m_suspendAfter) {
        cout << "System has been idle for more than " << m_suspendAfter
             << " minutes, will suspend the machine.\n";

        printHeaderMessage("Suspending the machine", true);

        suspendTheMachine();

        idleStartTime = Clock::now();
        notIdleStartTime = Clock::now();
      }
    }
  }
}

void Manager::getTheMachineUsage(double *cpuLoad, double *storageLoad, double *storageRead,
                                 double *storageWritten) {
  m_monitor.getCpuLoad(cpuLoad);
  m_monitor.getStorageLoad(storageLoad, storageRead, storageWritten);
}

void Manager::printTheMachineUsage() { m_monitor.printTheMachineUsage(); }

bool Manager::canBeSuspended() { return m_monitor.canBeSuspended(); }

bool Manager::isTheMachineIdle() { return m_monitor.isTheMachineIdle(); }

void Manager::suspendTheMachine() {

  std::unique_ptr<sdbus::IProxy> proxy =
      sdbus::createProxy("org.freedesktop.login1", "/org/freedesktop/login1");
  proxy->callMethod("Suspend")
      .onInterface("org.freedesktop.login1.Manager")
      .withTimeout(2000000)
      .withArguments(false)
      .storeResultsTo();
}
