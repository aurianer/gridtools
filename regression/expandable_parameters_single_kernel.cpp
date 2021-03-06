/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <vector>

#include <gtest/gtest.h>

#include <gridtools/stencil_composition/cartesian.hpp>
#include <gridtools/tools/cartesian_regression_fixture.hpp>

using namespace gridtools;
using namespace cartesian;

struct functor_single_kernel {
    using parameters1_out = inout_accessor<0>;
    using parameters2_out = inout_accessor<1>;
    using parameters3_out = inout_accessor<2>;
    using parameters4_out = inout_accessor<3>;
    using parameters5_out = inout_accessor<4>;

    using parameters1_in = in_accessor<5>;
    using parameters2_in = in_accessor<6>;
    using parameters3_in = in_accessor<7>;
    using parameters4_in = in_accessor<8>;
    using parameters5_in = in_accessor<9>;

    using param_list = make_param_list<parameters1_out,
        parameters2_out,
        parameters3_out,
        parameters4_out,
        parameters5_out,
        parameters1_in,
        parameters2_in,
        parameters3_in,
        parameters4_in,
        parameters5_in>;

    template <typename Evaluation>
    GT_FUNCTION static void apply(Evaluation eval) {
        eval(parameters1_out()) = eval(parameters1_in());
        eval(parameters2_out()) = eval(parameters2_in());
        eval(parameters3_out()) = eval(parameters3_in());
        eval(parameters4_out()) = eval(parameters4_in());
        eval(parameters5_out()) = eval(parameters5_in());
    }
};

using expandable_parameters_single_kernel = regression_fixture<>;

TEST_F(expandable_parameters_single_kernel, test) {
    std::vector<storage_type> out = {
        make_storage(1), make_storage(2), make_storage(3), make_storage(4), make_storage(5)};
    std::vector<storage_type> in = {
        make_storage(-1), make_storage(-2), make_storage(-3), make_storage(-4), make_storage(-5)};

    run(
        [](auto out0, auto out1, auto out2, auto out3, auto out4, auto in0, auto in1, auto in2, auto in3, auto in4) {
            GT_DECLARE_TMP(float_type, tmp0, tmp1, tmp2, tmp3, tmp4);
            return execute_parallel()
                .ij_cached(tmp0, tmp1, tmp2, tmp3, tmp4)
                .stage(functor_single_kernel(), tmp0, tmp1, tmp2, tmp3, tmp4, in0, in1, in2, in3, in4)
                .stage(functor_single_kernel(), out0, out1, out2, out3, out4, tmp0, tmp1, tmp2, tmp3, tmp4);
        },
        backend_t(),
        make_grid(),
        out[0],
        out[1],
        out[2],
        out[3],
        out[4],
        in[0],
        in[1],
        in[2],
        in[3],
        in[4]);

    for (size_t i = 0; i != in.size(); ++i)
        verify(in[i], out[i]);
}
