#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "format.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process() { pid_ = 0; }
Process::Process(int APid) { pid_ = APid; }

size_t lengthCmd = 40;

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const { 
    return LinuxParser::CpuUtilization(pid_); 
}

// Return the command that generated this process
string Process::Command() { 
    string cmd = LinuxParser::Command(Process::pid_);
    if (cmd.length() >= lengthCmd - 3) {
        return cmd.substr(0, lengthCmd - 3) + "...";
    }
    return cmd; 
}

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(Process::pid_); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(Process::pid_); }

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const { 
    return CpuUtilization() < a.CpuUtilization();
}

// Overload the "greater than" comparison operator for Process objects
bool Process::operator>(Process const& a) const { 
    return CpuUtilization() > a.CpuUtilization();
}