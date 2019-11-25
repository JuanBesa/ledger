
#include "dmlf/colearn/random_double.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <random>
#include <vector>

namespace fetch {
namespace dmlf {
namespace colearn {

class Node
{
  using Randomiser = RandomDouble;

public:

  Node()
  : randomising_offset_(randomiser_.GetNew())
  {
  } 

  std::vector<bool> const& processed() const
  {
    return processed_;
  }
  double randomising_offset() const
  {
    return randomising_offset_;
  }
  double MakeUpdate() 
  {
    return randomiser_.GetNew();
  }
  bool Process(double proportion, double random_factor)
  {
    DOUBLE whole;
    //double temp = randomiser_.GetNew();
    bool willProcess = std::modf(randomising_offset_ + random_factor, &whole) <= proportion;
    //processed_.push_back(willProcess);
    totalProcessed_ += willProcess;
    return willProcess;
  }

  double AverageProcessed() const
  {
    return TotalProcessed() / static_cast<double>(processed_.size());
  }
  long TotalProcessed() const
  {
    return totalProcessed_;
  }

private:
  using DOUBLE = double;

  std::vector<bool> processed_;
  Randomiser randomiser_;
  double randomising_offset_;
  long totalProcessed_;
};

}  // namespace colearn
}  // namespace dmlf
}  // namespace fetch

using fetch::dmlf::colearn::Node;
using Network = std::vector<Node>;

struct Experiment
{
  Experiment(std::size_t size)
  :network(size)
  {
  }

  int Broadcast(double proportion)
  {
    if (network.empty()) return 0;

    double random_factor = network[0].MakeUpdate();
    int numProcessed = 0;
    std::for_each(network.begin(), network.end(), [&proportion, &random_factor, &numProcessed] (Node &node) 
    {
      bool was_processed = node.Process(proportion, random_factor);
      numProcessed += was_processed;
    });
    results.push_back(numProcessed);
    return numProcessed;
  }

  double AverageProcessed() const
  {
  double sum = 0.0;
  std::for_each(network.cbegin(), network.cend(), [&sum] (Node const &node) 
      {
        sum += node.AverageProcessed();
      });

    return sum / network.size();
  }

  Network network;
  std::vector<int> results;
};

std::tuple<double,double,double,double> ProcessGlobal(Experiment const& experiment, std::size_t size, int n)
{
    double averageRatio = 0.0;
    double averageProcessed = 0.0;
    for (auto const &r : experiment.results)
    {
      averageProcessed += r;
      averageRatio        += static_cast<double>(r) / size;
    }
    averageProcessed /= n;
    averageRatio     /= n;

    double sdProcessed = 0.0;
    double sdRatio     = 0.0;
    for (auto const &r : experiment.results)
    {
      sdProcessed += std::pow(r - averageProcessed, 2);
      sdRatio     += std::pow((static_cast<double>(r) / size) - averageRatio, 2);
    }
    sdProcessed /= n-1;
    sdProcessed  = std::sqrt(sdProcessed);
    sdRatio     /= n-1;
    sdRatio      = std::sqrt(sdRatio);

    return std::make_tuple(averageProcessed, sdProcessed, averageRatio, sdRatio);
}

void ProportionTest(std::size_t size, int n)
{
  std::ofstream out("PropTest_Hot.csv");

  out << "Size," << size << ",n," << n << '\n';
  out << "Proportion,ProcAvg,ProcSD,RatioAvg,RatioSD,Min,Max\n";
  double step = 0.1;
  for (double proportion = 0.0; proportion <= 1.0; proportion += step)
  {
    Experiment experiment(size);
    for (int i = 0; i < n; ++i)
    {
      experiment.Broadcast(proportion);
    }

    double averageProcessed;
    double sdProcessed;
    double averageRatio;
    double sdRatio;
    std::tie(averageProcessed, sdProcessed, averageRatio, sdRatio) =  ProcessGlobal(experiment, size, n);

    out << proportion*100 
        << "," << averageProcessed << "," << sdProcessed 
        << "," << averageRatio*100 << "," << sdRatio*100 
        << "," << *std::min_element(experiment.results.begin(), experiment.results.end()) 
        << "," << *std::max_element(experiment.results.begin(), experiment.results.end()) 
        << '\n';
  }
}

void UsageTest(std::size_t size, int n, double proportion)
{
  std::ofstream out("UsageTest.csv");

  Experiment experiment(size);
  for (int i = 0; i < n; ++i)
  {
    experiment.Broadcast(proportion);
  }

  std::vector<long> allProcessed;
  allProcessed.reserve(size);
  std::transform(experiment.network.cbegin(), experiment.network.cend(), std::back_inserter(allProcessed), [] (Node const& n) { return n.TotalProcessed(); });

  std::sort(allProcessed.begin(), allProcessed.end());

  out << "Size," << size << ",n," << n << ",proportion," << proportion << '\n';
  out << "Num Processed\n"; 
  for (auto i : allProcessed)
  {
    out << i << '\n';
  }
}


int main()
{
  std::size_t size = 3000;
  int         n    = 100000;

  ProportionTest(size,n);
  //double proportion = 0.5;
  //UsageTest(size,n, proportion);

  return EXIT_SUCCESS;
}


