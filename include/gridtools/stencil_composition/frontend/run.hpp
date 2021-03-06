/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <utility>

#include "../../common/hymap.hpp"
#include "../../meta.hpp"
#include "../common/caches.hpp"
#include "../core/backend.hpp"
#include "../core/cache_info.hpp"
#include "../core/compute_extents_metafunctions.hpp"
#include "../core/esf.hpp"
#include "../core/execution_types.hpp"
#include "../core/mss.hpp"

namespace gridtools {
    namespace frontend_impl_ {
        template <class...>
        struct spec {};

        template <class ExecutionType, class Esfs, class Caches>
        struct spec<core::mss_descriptor<ExecutionType, Esfs, Caches>> {
            template <class F, class... Args>
            constexpr spec<core::mss_descriptor<ExecutionType,
                meta::push_back<Esfs, core::esf_descriptor<F, meta::list<Args...>, void>>,
                Caches>>
            stage(F, Args...) const {
                return {};
            }

            template <class Extent, class F, class... Args>
            constexpr spec<core::mss_descriptor<ExecutionType,
                meta::push_back<Esfs, core::esf_descriptor<F, meta::list<Args...>, Extent>>,
                Caches>>
            stage_with_extent(Extent, F, Args...) const {
                return {};
            }
        };

        template <class ExecutionType, class... Caches>
        struct empty_spec : spec<core::mss_descriptor<ExecutionType, meta::list<>, meta::list<Caches...>>> {
            template <class... Args>
            constexpr empty_spec<ExecutionType, Caches..., core::cache_info<Args, meta::list<cache_type::ij>>...>
            ij_cached(Args...) const {
                return {};
            }
            template <class... Args>
            constexpr empty_spec<ExecutionType, Caches..., core::cache_info<Args, meta::list<cache_type::k>>...>
            k_cached(Args...) const {
                return {};
            }
            template <class... Args>
            constexpr empty_spec<ExecutionType,
                Caches...,
                core::cache_info<Args, meta::list<cache_type::k>, meta::list<cache_io_policy::flush>>...>
            k_cached(cache_io_policy::flush, Args...) const {
                return {};
            }
            template <class... Args>
            constexpr empty_spec<ExecutionType,
                Caches...,
                core::cache_info<Args, meta::list<cache_type::k>, meta::list<cache_io_policy::fill>>...>
            k_cached(cache_io_policy::fill, Args...) const {
                return {};
            }
            template <class... Args>
            constexpr empty_spec<ExecutionType,
                Caches...,
                core::cache_info<Args,
                    meta::list<cache_type::k>,
                    meta::list<cache_io_policy::fill, cache_io_policy::flush>>...>
            k_cached(cache_io_policy::fill, cache_io_policy::flush, Args...) const {
                return {};
            }
            template <class... Args>
            constexpr empty_spec<ExecutionType,
                Caches...,
                core::cache_info<Args,
                    meta::list<cache_type::k>,
                    meta::list<cache_io_policy::fill, cache_io_policy::flush>>...>
            k_cached(cache_io_policy::flush, cache_io_policy::fill, Args...) const {
                return {};
            }
        };

        constexpr empty_spec<core::parallel> execute_parallel() { return {}; }
        constexpr empty_spec<core::forward> execute_forward() { return {}; }
        constexpr empty_spec<core::backward> execute_backward() { return {}; }

        template <class... Msses>
        constexpr spec<Msses...> multi_pass(spec<Msses>...) {
            return {};
        }

        template <size_t>
        struct arg {};

        template <class Comp, class Backend, class Grid, class... Fields, size_t... Is>
        void run_impl(Comp comp, Backend, Grid const &grid, std::index_sequence<Is...>, Fields &&... fields) {
            using spec_t = decltype(comp(arg<Is>()...));
            using entry_point_t = core::backend_entry_point_f<Backend, spec_t>;
            using data_store_map_t = typename hymap::keys<arg<Is>...>::template values<Fields &...>;
            entry_point_t()(grid, data_store_map_t{fields...});
        }

        template <class Comp, class Backend, class Grid, class... Fields>
        void run(Comp comp, Backend be, Grid const &grid, Fields &&... fields) {
            run_impl(comp, be, grid, std::index_sequence_for<Fields...>(), std::forward<Fields>(fields)...);
        }

        template <class F, class Backend, class Grid, class... Fields>
        void run_single_stage(F, Backend be, Grid const &grid, Fields &&... fields) {
            return run([](auto... args) { return execute_parallel().stage(F(), args...); },
                be,
                grid,
                std::forward<Fields>(fields)...);
        }

        template <class... Msses, class Arg>
        constexpr core::lookup_extent_map<core::get_extent_map_from_msses<spec<Msses...>>, Arg> get_arg_extent(
            spec<Msses...>, Arg) {
            return {};
        }

        template <class Mss>
        using rw_args_from_mss = core::compute_readwrite_args<typename Mss::esf_sequence_t>;

        template <class Msses,
            class RwArgsLists = meta::transform<rw_args_from_mss, Msses>,
            class RawRwArgs = meta::flatten<RwArgsLists>>
        using all_rw_args = meta::dedup<RawRwArgs>;

        template <class... Msses,
            class Arg,
            class RwPlhs = all_rw_args<spec<Msses...>>,
            intent Intent = meta::st_contains<RwPlhs, Arg>::value ? intent::inout : intent::in>
        constexpr std::integral_constant<intent, Intent> get_arg_intent(spec<Msses...>, Arg) {
            return {};
        }
    } // namespace frontend_impl_
    using frontend_impl_::execute_backward;
    using frontend_impl_::execute_forward;
    using frontend_impl_::execute_parallel;
    using frontend_impl_::get_arg_extent;
    using frontend_impl_::get_arg_intent;
    using frontend_impl_::multi_pass;
    using frontend_impl_::run;
    using frontend_impl_::run_single_stage;
} // namespace gridtools
