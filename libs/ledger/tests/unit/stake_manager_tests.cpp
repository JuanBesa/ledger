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

#include "core/random/lcg.hpp"
#include "crypto/identity.hpp"
#include "ledger/chain/block.hpp"
#include "ledger/consensus/naive_entropy_generator.hpp"
#include "ledger/consensus/stake_manager.hpp"
#include "ledger/consensus/stake_snapshot.hpp"
#include "random_address.hpp"

#include "gtest/gtest.h"

#include <memory>

namespace {

using fetch::ledger::Block;
using fetch::ledger::Address;
using fetch::ledger::StakeSnapshot;
using fetch::ledger::StakeManager;
using fetch::ledger::NaiveEntropyGenerator;
using fetch::crypto::Identity;

using RNG             = fetch::random::LinearCongruentialGenerator;
using StakeManagerPtr = std::unique_ptr<StakeManager>;
using EntropyPtr      = std::unique_ptr<NaiveEntropyGenerator>;
using RoundStats      = std::unordered_map<Identity, std::size_t>;

constexpr char const *LOGGING_NAME = "StakeMgrTests";

bool IndexOf(std::vector<Identity> const &identities, Identity const &target, std::size_t &index)
{
  bool found{false};

  std::size_t idx{0};
  for (auto const &identity : identities)
  {
    if (identity == target)
    {
      index = idx;
      found = true;
      break;
    }

    ++idx;
  }

  return found;
}

std::size_t WeightOf(std::vector<Identity> const &identities, Identity const &target)
{
  std::size_t weight{0};
  std::size_t index{0};
  if (IndexOf(identities, target, index))
  {
    weight = identities.size() - index;
  }

  return weight;
}

class StakeManagerTests : public ::testing::Test
{
protected:
  void SetUp() override
  {
    rng_.Seed(2048);
    entropy_       = std::make_unique<NaiveEntropyGenerator>();
    stake_manager_ = std::make_unique<StakeManager>(*entropy_);
  }

  void TearDown() override
  {
    stake_manager_.reset();
  }

  void SimulateRounds(std::vector<Identity> const &identities, Block &block, std::size_t num_rounds,
                      std::size_t committee_size, RoundStats &stats)
  {
    ASSERT_GT(committee_size, 0);

    // initialise the stats (create entries if they don't exist)
    for (auto const &identity : identities)
    {
      stats.emplace(identity, 0);
    }

    for (std::size_t round = 0; round < num_rounds; ++round)
    {
      // validate the committee vs the generation weight
      auto const committee = stake_manager_->GetCommittee(block);
      ASSERT_TRUE(static_cast<bool>(committee));
      ASSERT_EQ(committee->size(), committee_size);

      // update the statistics
      stats.at(committee->at(0)) += 1;

      for (auto const &identity : identities)
      {
        EXPECT_EQ(WeightOf(*committee, identity),
                  stake_manager_->GetBlockGenerationWeight(block, Address(identity)));
      }

      // "forge" the next block
      block.body.previous_hash = block.body.hash;
      block.body.hash          = GenerateRandomAddress(rng_).address();
      block.body.block_number += 1;

      stake_manager_->UpdateCurrentBlock(block);
    }
  }

  RNG             rng_;
  EntropyPtr      entropy_;
  StakeManagerPtr stake_manager_;
};

TEST_F(StakeManagerTests, CheckBasicStakeChangeScenarios)
{
  static constexpr std::size_t COMMITTEE_SIZE = 1;

  std::vector<Identity> identities = {
      GenerateRandomIdentity(rng_),
      GenerateRandomIdentity(rng_),
      GenerateRandomIdentity(rng_),
  };

  // create the initial stake setup
  StakeSnapshot initial{};
  for (auto const &identity : identities)
  {
    initial.UpdateStake(identity, 500);
  }

  // configure the stake manager
  stake_manager_->Reset(initial, COMMITTEE_SIZE);

  // create the starting blocks (note block contains an address, not an identity)
  Block block;
  block.body.hash         = GenerateRandomAddress(rng_).address();
  block.body.block_number = 1;

  // simulate a number of rounds
  RoundStats stats{};
  SimulateRounds(identities, block, 100, COMMITTEE_SIZE, stats);

  for (auto const &identity : identities)
  {
    FETCH_LOG_DEBUG(LOGGING_NAME, "Identity: ", identity.identifier().ToBase64(),
                    " rounds: ", stats.at(address));

    EXPECT_GT(stats.at(identity), 0);
  }

  // along comes another staker
  identities.emplace_back(GenerateRandomIdentity(rng_));
  stake_manager_->update_queue().AddStakeUpdate(150, identities.back(), 500);

  stats.clear();
  SimulateRounds(identities, block, 100, COMMITTEE_SIZE, stats);

  for (auto const &identity : identities)
  {
    EXPECT_GT(stats.at(identity), 0);
  }

  // stakers have been removed
  for (std::size_t i = 1; i < identities.size(); ++i)
  {
    stake_manager_->update_queue().AddStakeUpdate(250, identities.at(i), 0);
  }

  stats.clear();
  SimulateRounds(identities, block, 100, COMMITTEE_SIZE, stats);

  for (auto const &identity : identities)
  {
    EXPECT_GT(stats.at(identity), 0);
  }

  stats.clear();
  SimulateRounds(identities, block, 100, COMMITTEE_SIZE, stats);

  std::size_t idx{0};
  for (auto const &identity : identities)
  {
    if (idx == 0)
    {
      EXPECT_GT(stats.at(identity), 0);
    }
    else
    {
      EXPECT_EQ(stats.at(identity), 0);
    }

    ++idx;
  }
}

}  // namespace
