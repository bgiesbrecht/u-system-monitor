#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process() { pid_ = 0; }
Process::Process(int APid) { pid_ = APid; }

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() { return LinuxParser::ActiveJiffies(pid_); }

// Return the command that generated this process
string Process::Command() { return LinuxParser::Command(Process::pid_); }

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(Process::pid_); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(Process::pid_); }

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a[[maybe_unused]]) const { return true; }