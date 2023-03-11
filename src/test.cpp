#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "linux_parser.h"

using std::string;
using std::cout;
using std::vector;
using std::unordered_map;
using std::unordered_set;



template <typename T> unordered_map<string, T> GetTabularValues(const string &source, const unordered_set<string> &keys = {}) {
  unordered_map<string, T> values;
  int keys_remaining = keys.size();
  bool getAllValues = keys_remaining == 0;
  std::ifstream filestream(source);
  string line, key;
  T value;
  if (filestream.is_open()) {
    while ((getAllValues || keys_remaining) && std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (getAllValues) {
        values[key] = value;
      } else {
        if (keys.find(key) != keys.end()) {
            values[key] = value;
            keys_remaining--;
        }
      }
    }
  }
  return values;
}


template <typename T> vector<T> GetSerialValues(string &source) {
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


int main() {
//   string name, unit;
//   int value;
//   string line;

//   unordered_set<string> keys = {"MemTotal:", "MemFree:"};
//   unordered_map<string, int> results = GetTabularValues<int>("/proc/meminfo", keys);
//   for (auto result : results) {
//     cout << result.first << result.second << "\n";
//   }

//   long uptime = 0;
//   std::ifstream stream("/proc/uptime");
//   if (stream.is_open()) {
//     std::getline(stream, line);
//     std::istringstream linestream(line);
//     linestream >> uptime;
//   }
//   cout << "Uptime: " << uptime << "\n";

  // auto u = LinuxParser::CpuUtilization();
  // for (string x : u) {
  //   cout << x;
  // }

  cout << LinuxParser::Uid(21102) << "\n";
  cout << LinuxParser::User(21102) << "\n";

}
