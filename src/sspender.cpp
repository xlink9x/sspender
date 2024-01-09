/*
 * Copyright (C) 2016 Mountassir El Hafi, (mountassirbillah1@gmail.com)
 *
 * sspender.cpp: Part of sspender
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

#include "ConfigParser.h"
#include "Manager.h"
#include "PartitionTable.h"

using namespace std;

int main(int argc, char *argv[]) {
  string filePath;

  if (argc > 1) {
    filePath = argv[1];
  }

  vector<string> ipToWatch;
  vector<DiskCfg> disksToMonitor;
  CpuCfg cpuConfig;
  int check_if_idle_every;
  int stop_monitoring_for;
  int reset_monitoring_after;
  int suspend_after;

  PartitionTable partitionTable;

  partitionTable.loadPartitionTable();

  cout << "PartitionTable loaded:\n" << partitionTable << endl;

  ConfigParser configParser(partitionTable);
  Manager manager;

  bool configParsed = configParser.loadConfigs(
      filePath, partitionTable, &ipToWatch, &cpuConfig, &disksToMonitor, &check_if_idle_every,
      &stop_monitoring_for, &reset_monitoring_after, &suspend_after);

  if (configParsed) {
    cout << std::fixed << std::setprecision(2);
    printHeaderMessage("Using the following validated configuration:", false);

    cout << "\nDon't suspend the machine if any of these IPs is online: ";
    for (size_t i = 0, size = ipToWatch.size(); i < size; ++i) {
      cout << ipToWatch[i] << ",";
    }

    cout << "\nSuspend the machine if all these devices are idle: ";
    if (cpuConfig.suspendIfIdle) {
      cout << cpuConfig.cpuName << ",";
    }

    for (size_t i = 0, size = disksToMonitor.size(); i < size; ++i) {
      if (disksToMonitor[i].suspendIfIdle) {
        cout << disksToMonitor[i].diskName << ",";
      }
    }

    cout << "\nSpin down these drives if they are idle: ";
    for (size_t i = 0, size = disksToMonitor.size(); i < size; ++i) {
      if (disksToMonitor[i].spinDown) {
        cout << disksToMonitor[i].diskName << ",";
      }
    }

    manager.setIpsToWatch(ipToWatch);
    manager.setDisksToMonitor(disksToMonitor);
    manager.setCpusToMonitor(cpuConfig);
    manager.setTimers(check_if_idle_every, stop_monitoring_for, reset_monitoring_after,
                      suspend_after);

    printHeaderMessage("Monitoring the machine", false);

    manager.monitorSystemUsage();
  } else {
    cout << "Failed to parse the cfg file, quitting." << endl;
  }

  return (EXIT_SUCCESS);
}
