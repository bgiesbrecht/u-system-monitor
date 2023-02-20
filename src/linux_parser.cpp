#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <iostream>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::unordered_map;
using std::unordered_set;


template <typename T> unordered_map<string, T> GetTabularValues(string source, unordered_set<string> keys) {
  unordered_map<string, T> values;
  int keys_remaining = keys.size();
  std::ifstream filestream(source);
  string line, key;
  T value;
  if (filestream.is_open()) {
    while (keys_remaining && std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (keys.find(key) != keys.end()) {
        values[key] = value;
        keys_remaining--;
      }
    }
  }
  return values;
}

template <typename T> vector<T> getSerialValues(string source) {
  vector<T> values{};
  std::ifstream filestream(source);
  string token;
  T value;
  if (filestream.is_open()) {
    while (std::getline(filestream, token, ' ')) {
      if (token.size()) {
        std::istringstream tokenstream(token);
        tokenstream >> value;
        values.push_back(value);
      }
    }
  }
  return values;  
}

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
    std::getline(stream, line);
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
  return (memtotal - memfree) / memtotal * 1.0; 
}

long LinuxParser::UpTime() { 
  string line;
  long uptime = 0;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return uptime;
}

long LinuxParser::Jiffies() { 
  return ActiveJiffies() + IdleJiffies(); 
}

long LinuxParser::ActiveJiffies() { 
  string line;
  string key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
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

long LinuxParser::IdleJiffies() { 
  string line;
  string key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
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

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  long active = ActiveJiffies();
  long idle = IdleJiffies();
  float pct = active * 100.0 / (active + idle);
  int bars = std::ceil(std::ceil(pct) / 2.0);
  std::cout << "active: " << active << " idle: " << idle << " bars: " << bars << "\n";
  vector<string> utilization(bars);
  fill(utilization.begin(), utilization.end(), '|');
  return utilization;  
}


// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { return 0; }
// stat

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { return 0; }
// stat



// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid[[maybe_unused]]) { return 0; }
