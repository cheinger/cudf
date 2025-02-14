/*
 * Copyright (c) 2020-2022, NVIDIA CORPORATION.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cudf/column/column_view.hpp>
#include <cudf/detail/stream_compaction.hpp>
#include <cudf/types.hpp>
#include <cudf_test/base_fixture.hpp>
#include <cudf_test/column_wrapper.hpp>

#include <fixture/rmm_pool_raii.hpp>

#include <nvbench/nvbench.cuh>

#include <memory>
#include <random>

NVBENCH_DECLARE_TYPE_STRINGS(cudf::timestamp_ms, "cudf::timestamp_ms", "cudf::timestamp_ms");

template <typename Type>
void nvbench_distinct(nvbench::state& state, nvbench::type_list<Type>)
{
  cudf::rmm_pool_raii pool_raii;

  auto const num_rows = state.get_int64("NumRows");

  cudf::test::UniformRandomGenerator<long> rand_gen(0, 100);
  auto elements = cudf::detail::make_counting_transform_iterator(
    0, [&rand_gen](auto row) { return rand_gen.generate(); });
  auto valids =
    cudf::detail::make_counting_transform_iterator(0, [](auto i) { return i % 100 != 0; });
  cudf::test::fixed_width_column_wrapper<Type, long> values(elements, elements + num_rows, valids);

  auto input_column = cudf::column_view(values);
  auto input_table  = cudf::table_view({input_column, input_column, input_column, input_column});

  state.exec(nvbench::exec_tag::sync, [&](nvbench::launch& launch) {
    rmm::cuda_stream_view stream_view{launch.get_stream()};
    auto result = cudf::detail::distinct(input_table, {0}, cudf::null_equality::EQUAL, stream_view);
  });
}

using data_type = nvbench::type_list<bool, int8_t, int32_t, int64_t, float, cudf::timestamp_ms>;

NVBENCH_BENCH_TYPES(nvbench_distinct, NVBENCH_TYPE_AXES(data_type))
  .set_name("distinct")
  .set_type_axes_names({"Type"})
  .add_int64_axis("NumRows", {10'000, 100'000, 1'000'000, 10'000'000});
