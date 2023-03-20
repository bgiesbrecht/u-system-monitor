#include <vector>

#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();
 
 private:
    std::vector<long> cpuState = {0,0,0,0,0,0,0,0,0,0};
};

#endif