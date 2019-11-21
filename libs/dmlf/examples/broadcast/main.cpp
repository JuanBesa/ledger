
#include "dmlf/colearn/random_double.hpp"

#include <cmath>
#include <iostream>
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
    double whole;
    double temp = randomiser_.GetNew();
    bool willProcess = std::modf(temp + random_factor, &whole) <= proportion;
    //processed_.push_back(willProcess);
    return willProcess;
  }

  double AverageProcessed() const
  {
    return TotalProcessed() / static_cast<double>(processed_.size());
  }
  long TotalProcessed() const
  {
    return std::count(processed_.begin(), processed_.end(), true);
  }

private:
  std::vector<bool> processed_;
  Randomiser randomiser_;
  double randomising_offset_;
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

int main()
{
  std::size_t size = 3000;
  int         n    = 10000;
  std::cout << "Size: " << size << "\tn " << n << '\n';
  double step = 0.01;
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

    std::cout << "Proportion " << proportion*100 
    << "\tProcessed " << averageProcessed << "\tsd: " << sdProcessed 
    << "\tRatio " << averageRatio*100 << "\tsd: " << sdRatio*100 
    << "\tMin " << *std::min_element(experiment.results.begin(), experiment.results.end()) 
    << "\tMax " << *std::max_element(experiment.results.begin(), experiment.results.end()) 
    << '\n';
  }

  return EXIT_SUCCESS;
}


