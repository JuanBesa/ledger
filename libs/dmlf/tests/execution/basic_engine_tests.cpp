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

#include "gtest/gtest.h"

#include "dmlf/execution/basic_vm_engine.hpp"

#include "dmlf/execution/execution_error_message.hpp"
#include "variant/variant.hpp"
#include "vectorise/fixed_point/fixed_point.hpp"

#include <limits>

namespace {

namespace {

using namespace fetch::vm;
using namespace fetch::dmlf;

using Stage = ExecutionErrorMessage::Stage;
using Code  = ExecutionErrorMessage::Code;

using LedgerVariant = BasicVmEngine::LedgerVariant;
using Params        = BasicVmEngine::Params;

using fp64_t = fetch::fixed_point::fp64_t;
using fp32_t = fetch::fixed_point::fp32_t;

auto const add = R"(

 function add(a : Int32, b : Int32) : Int32
  return a + b;
 endfunction

)";

auto const Add3Array = R"(

function add(array2 : Array<Array<Array<Int32>>>, array : Array<Int32>) : Int32
  return array[0] + array[1] + array2[0][1][2];
endfunction

)";

}  // namespace

TEST(BasicVmEngineDmlfTests, Add)
{
  BasicVmEngine engine;

  ExecutionResult createdProgram = engine.CreateExecutable("add", {{"etch", add}});
  EXPECT_TRUE(createdProgram.succeeded());

  ExecutionResult createdState = engine.CreateState("state");
  EXPECT_TRUE(createdState.succeeded());

  ExecutionResult result =
      engine.Run("add", "state", "add", Params{LedgerVariant(1), LedgerVariant(2)});
  EXPECT_TRUE(result.succeeded());
  // std::cout << result.error().message() << '\n';
  EXPECT_EQ(result.output().As<int>(), 3);
}

TEST(BasicVmEngineDmlfTests, Add3Array)
{
  BasicVmEngine engine;

  ExecutionResult createdProgram = engine.CreateExecutable("add", {{"etch", Add3Array}});
  EXPECT_TRUE(createdProgram.succeeded()) << createdProgram.error().message() << '\n';

  ExecutionResult createdState = engine.CreateState("state");
  EXPECT_TRUE(createdState.succeeded());

  ExecutionResult result = engine.Run(
      "add", "state", "add", Params{LedgerVariant(), LedgerVariant()});
  EXPECT_TRUE(result.succeeded()) << result.error().message() << '\n';
  //EXPECT_EQ(result.output().As<fp64_t>(), 9.5);
}

}  // namespace
