#pragma once
//------------------------------------------------------------------------------
//
//   Copyright 2018-2019 Fetch.AI Limited
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//------------------------------------------------------------------------------

#include <memory>

#include "dmlf/ishuffle_algorithm.hpp"

namespace fetch {
namespace dmlf {

class IUpdate;

class ILearnerNetworker
{
public:
  ILearnerNetworker()
  {
  }
  virtual ~ILearnerNetworker()
  {
  }

  virtual void pushUpdate( std::shared_ptr<IUpdate> update) = 0;
  virtual std::size_t getUpdateCount() const = 0;
  virtual std::shared_ptr<IUpdate> getUpdate() = 0;

  virtual void setShuffleAlgorithm(std::shared_ptr<IShuffleAlgorithm> alg)
  {
    this -> alg = alg;
  }

  virtual std::size_t getCount() = 0;
protected:
  std::shared_ptr<IShuffleAlgorithm> alg; // used by descendents
private:

  ILearnerNetworker(const ILearnerNetworker &other) = delete;
  ILearnerNetworker &operator=(const ILearnerNetworker &other) = delete;
  bool operator==(const ILearnerNetworker &other) = delete;
  bool operator<(const ILearnerNetworker &other) = delete;
};

}
}