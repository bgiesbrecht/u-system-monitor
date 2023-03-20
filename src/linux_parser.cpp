#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <cstdlib> 
#include <iostream>
#include <iomanip> 

#include "linux_parser.h"
#include "format.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::ostringstream;
using std::getline;
using LinuxParser::CPUStates;

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() { 
  string key;
  int value;
  string line;
  int keysNeeded = 2;
  int keysGotten = 0;
  int memtotal = 0;
  int memfree = 0;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (keysGotten < keysNeeded && std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "MemTotal") {
        memtotal = value;
        keysGotten++;
      } else if (key == "MemFree") {
        memfree = value;
        keysGotten++;
      }
    }
  }
  return (float)(memtotal - memfree) / (float)memtotal; 
}

long LinuxParser::UpTime() { 
  string line;
  long uptime = 0;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return uptime;
}

long LinuxParser::Jiffies() { 
  return ActiveJiffies() + IdleJiffies(); 
}

long GetActiveJiffies(string StatPath) { 
  string line;
  string key;
  std::ifstream stream(StatPath);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "cpu") {
        long user, nice, system;
        linestream >> user >> nice >> system;
        return user + system;
      }
    }
  }  
  return 0; 
}

long LinuxParser::ActiveJiffies() { 
  return GetActiveJiffies(kProcDirectory + kStatFilename);
}

long LinuxParser::IdleJiffies() { 
  string line;
  string key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "cpu") {
        long user, nice, system, idle;
        linestream >> user >> nice >> system >> idle;
        return idle;
      }
    }
  }  
  return 0; 
}

// Read and return CPU utilization
float LinuxParser::CpuUtilization(std::vector<long> &prev) {

  // Read cpu vals into a vector
  std::vector<long> vals;
  string line;
  string key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      long val;
      if (key == "cpu") {
        for (int i = 0; i < 10; i++) {
          linestream >> val;
          vals.push_back(val);
        }
        break;
      }
    }
  }  

  // This algorithm was taken from https://stackoverflow.com/a/23376195
  long prevIdle = prev[CPUStates::kIdle_] + prev[CPUStates::kIOwait_];
  long idle = vals[CPUStates::kIdle_] + vals[CPUStates::kIOwait_];

  long prevNonIdle = prev[CPUStates::kUser_] + prev[CPUStates::kNice_] + prev[CPUStates::kSystem_] + prev[CPUStates::kIRQ_] + prev[CPUStates::kSoftIRQ_] + prev[CPUStates::kSteal_];
  long nonIdle = vals[CPUStates::kUser_] + vals[CPUStates::kNice_] + vals[CPUStates::kSystem_] + vals[CPUStates::kIRQ_] + vals[CPUStates::kSoftIRQ_] + vals[CPUStates::kSteal_];

  prev = vals;

  long prevTotal = prevIdle + prevNonIdle;
  long total = idle + nonIdle;

  long totalDiff = total - prevTotal;
  long idleDiff = idle - prevIdle;

  return (float)(totalDiff - idleDiff) / (float)totalDiff;
}


// Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  string line;
  string key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "processes") {
        unsigned int processes;
        linestream >> processes;
        return processes;
      }
    }
  }  
  return 0; 
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  string line;
  string key;
  int procs_running = 0;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "procs_running") {
        linestream >> procs_running;
        return procs_running;
      }
    }
  }  
  return procs_running;  
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  auto j = GetActiveJiffies(kProcDirectory + to_string(pid) + kStatFilename);
  return j;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    string cmd;
    getline(stream, cmd);
    return cmd;
  }  
  return "unknown";
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) +  kStatusFilename);
  if (stream.is_open()) {
    string key;
    int kb;
    string line;
    while (getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "VmSize:") {
        linestream >> kb;
        float mb = (float)kb/1024;
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << mb;
        return ss.str();
      }
    }
  }  
  return "unknown";
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    string key, value;
    string line;
    while (getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "Uid:") {
        linestream >> value;
        return value;
      }
    }
  }  
  return "unknown";
}

string LinuxParser::User(int pid) { 
  // Get the uid for this pid
  string uid = LinuxParser::Uid(pid);

  // Get the username for this uid
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    string pName, pX, pUid;
    string line;
    while (getline(stream, line)) {
      std::istringstream linestream(line);
      std::getline(linestream, pName, ':');
      std::getline(linestream, pX, ':');
      std::getline(linestream, pUid, ':');
      if (pUid == uid) {
        return pName;
      }
    }
  }  
  return "unknown";
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string toOpen = kProcDirectory + to_string(pid) + kStatFilename;
  std::ifstream stream(toOpen);
  if (stream.is_open()) {
    string val;
    string line;
    long starttime;
    std::vector<string> statVals;
    getline(stream, line);
    std::istringstream linestream(line);
    for (int i = 0; i < 21; i++) {
      linestream >> val;
    }
    linestream >> starttime;

    // long seconds = LinuxParser::UpTime() - (starttime / sysconf(_SC_CLK_TCK));
    long seconds = starttime / sysconf(_SC_CLK_TCK);
    return seconds;
  }
  return 0;
}

// Read and return the uptime of a process
float LinuxParser::CpuUtilization(int pid) {
  string toOpen = kProcDirectory + to_string(pid) + kStatFilename;
  std::ifstream stream(toOpen);
  if (stream.is_open()) {
    string val;
    string line;
    std::vector<string> statVals;
    getline(stream, line);
    std::istringstream linestream(line);
    for (int i = 0; i < 22; i++) {
      linestream >> val;
      statVals.push_back(val);
    }

    // Algorithm taken from https://stackoverflow.com/a/16736599
    long utime = atol(statVals[13].c_str());
    long stime = atol(statVals[14].c_str());
    long cutime = atol(statVals[14].c_str());
    long cstime = atol(statVals[16].c_str());
    long starttime = atol(statVals[21].c_str());

    long total_time = utime + stime + cutime + cstime;
    long seconds = LinuxParser::UpTime() - (starttime / sysconf(_SC_CLK_TCK));
    float cpu_usage = ((float)total_time / sysconf(_SC_CLK_TCK)) / (float)seconds;

    return cpu_usage;
  }

  return 0;
}
