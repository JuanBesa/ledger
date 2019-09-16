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

#include "core/random.hpp"
#include "math/base_types.hpp"

#include <mutex>

namespace fetch {
namespace ml {
namespace distributed_learning {

template <class TensorType>
class TrainingClient;

enum class CoordinatorMode
{
  SYNCHRONOUS,
  SEMI_SYNCHRONOUS,
  ASYNCHRONOUS
};

enum class CoordinatorState
{
  RUN,
  STOP,
};

struct CoordinatorParams
{
  using SizeType = fetch::math::SizeType;

  CoordinatorMode mode;
  SizeType        iterations_count;
};

template <typename TensorType>
class Coordinator
{
public:
  using SizeType      = fetch::math::SizeType;
  using ClientPtrType = std::shared_ptr<TrainingClient<TensorType>>;

  Coordinator(CoordinatorParams const &params);

  void IncrementIterationsCounter();

  void Reset();

  CoordinatorMode GetMode() const;

  CoordinatorState GetState() const;

  void AddClient(ClientPtrType const &new_client);
  void SetClientsList(std::vector<ClientPtrType> const &new_client);

private:
  CoordinatorMode                                          mode_;
  CoordinatorState                                         state_           = CoordinatorState::RUN;
  SizeType                                                 iterations_done_ = 0;
  SizeType                                                 iterations_count_;
  std::mutex                                               iterations_mutex_;
  std::vector<std::shared_ptr<TrainingClient<TensorType>>> clients_;
  SizeType                                                 number_of_peers_;

  // random number generator for shuffling peers
  fetch::random::LaggedFibonacciGenerator<> gen_;
};

template <typename TensorType>
Coordinator<TensorType>::Coordinator(CoordinatorParams const &params)
  : mode_(params.mode)
  , iterations_count_(params.iterations_count)
{}

template <typename TensorType>
void Coordinator<TensorType>::IncrementIterationsCounter()
{
  std::lock_guard<std::mutex> l(iterations_mutex_);
  iterations_done_++;

  if (iterations_done_ >= iterations_count_)
  {
    state_ = CoordinatorState::STOP;
  }
}

template <typename TensorType>
void Coordinator<TensorType>::Reset()
{
  iterations_done_ = 0;
  state_           = CoordinatorState::RUN;
}

template <typename TensorType>
CoordinatorMode Coordinator<TensorType>::GetMode() const
{
  return mode_;
}

template <typename TensorType>
CoordinatorState Coordinator<TensorType>::GetState() const
{
  return state_;
}

/**
 * Add pointer to client
 * @param clients
 */
template <typename TensorType>
void Coordinator<TensorType>::AddClient(
    std::shared_ptr<TrainingClient<TensorType>> const &new_client)
{
  clients_.push_back(new_client);
}

/**
 * Add pointer to client
 * @param clients
 */
template <typename TensorType>
void Coordinator<TensorType>::SetClientsList(
    std::vector<std::shared_ptr<TrainingClient<TensorType>>> const &new_clients)
{
  clients_ = new_clients;
}

}  // namespace distributed_learning
}  // namespace ml
}  // namespace fetch
