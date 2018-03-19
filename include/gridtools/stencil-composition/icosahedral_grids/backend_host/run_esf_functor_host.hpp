/*
  GridTools Libraries

  Copyright (c) 2017, ETH Zurich and MeteoSwiss
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  For information: http://eth-cscs.github.io/gridtools/
*/
#pragma once

#include "../../run_esf_functor.hpp"
#include "../iterate_domain_remapper.hpp"
#include "../../functor_decorator.hpp"

namespace gridtools {

    /*
     * @brief main functor that executes (for host) the user functor of an ESF
     * @tparam RunFunctorArguments run functor arguments
     * @tparam Interval interval where the functor gets executed
     */
    template < typename RunFunctorArguments, typename Interval >
    struct run_esf_functor_host
        : public run_esf_functor< run_esf_functor_host< RunFunctorArguments, Interval > > // CRTP
    {
        GRIDTOOLS_STATIC_ASSERT((is_run_functor_arguments< RunFunctorArguments >::value), GT_INTERNAL_ERROR);
        typedef run_esf_functor< run_esf_functor_host< RunFunctorArguments, Interval > > super;
        typedef typename RunFunctorArguments::iterate_domain_t iterate_domain_t;

        typedef typename super::run_functor_arguments_t run_functor_arguments_t;

        GT_FUNCTION
        explicit run_esf_functor_host(iterate_domain_t &iterate_domain) : super(iterate_domain) {}

        template < typename EsfArguments >
        struct color_esf_match {
            GRIDTOOLS_STATIC_ASSERT((is_esf_arguments< EsfArguments >::value), GT_INTERNAL_ERROR);
            typedef typename boost::mpl::or_< typename boost::is_same< typename RunFunctorArguments::color_t,
                                                  typename EsfArguments::esf_t::color_t >::type,
                typename boost::is_same< nocolor, typename EsfArguments::esf_t::color_t >::type >::type type;
        };

        /*
         * @brief main functor implemenation that executes (for Host) the user functor of an ESF
         *      (specialization for non reduction operations)
         * @tparam IntervalType interval where the functor gets executed
         * @tparam EsfArgument esf arguments type that contains the arguments needed to execute this ESF.
         */
        template < typename IntervalType, typename EsfArguments >
        GT_FUNCTION void do_impl(
            typename boost::enable_if< typename color_esf_match< EsfArguments >::type, int >::type = 0) const {
            GRIDTOOLS_STATIC_ASSERT((is_esf_arguments< EsfArguments >::value), GT_INTERNAL_ERROR);
            call_user_functor< IntervalType, EsfArguments >();
        }

        template < typename IntervalType, typename EsfArguments >
        GT_FUNCTION void do_impl(
            typename boost::disable_if< typename color_esf_match< EsfArguments >::type, int >::type = 0) const {}

      private:
        /*
         * @brief main functor implemenation that executes (for Host) the user functor of an ESF
         *      (specialization for non reduction operations)
         * @tparam IntervalType interval where the functor gets executed
         * @tparam EsfArgument esf arguments type that contains the arguments needed to execute this ESF.
         */
        template < typename IntervalType, typename EsfArguments >
        GT_FUNCTION void call_user_functor(
            typename boost::disable_if< typename EsfArguments::is_reduction_t, int >::type = 0) const {
            GRIDTOOLS_STATIC_ASSERT((is_esf_arguments< EsfArguments >::value), GT_INTERNAL_ERROR);

            typedef typename EsfArguments::functor_t original_functor_t;
            typedef typename EsfArguments::esf_t esf_t;
            typedef typename esf_t::template esf_function< run_functor_arguments_t::color_t::color_t::value >
                colored_functor_t;

            typedef functor_decorator< typename original_functor_t::id,
                colored_functor_t,
                typename original_functor_t::repeat_t,
                IntervalType > functor_t;

            GRIDTOOLS_STATIC_ASSERT(is_functor_decorator< functor_t >::value, GT_INTERNAL_ERROR);

            typedef typename get_trivial_iterate_domain_remapper< iterate_domain_t,
                typename EsfArguments::esf_t,
                typename run_functor_arguments_t::color_t >::type iterate_domain_remapper_t;

            iterate_domain_remapper_t iterate_domain_remapper(this->m_iterate_domain);

            _impl::call_repeated< functor_t::repeat_t::value, functor_t, iterate_domain_remapper_t, IntervalType >::
                call_do_method(iterate_domain_remapper);
        }

        /*
         * @brief main functor implemenation that executes (for Host) the user functor of an ESF
         *      (specialization for reduction operations)
         * @tparam IntervalType interval where the functor gets executed
         * @tparam EsfArgument esf arguments type that contains the arguments needed to execute this ESF.
         */
        template < typename IntervalType, typename EsfArguments >
        GT_FUNCTION void call_user_functor(
            typename boost::enable_if< typename EsfArguments::is_reduction_t, int >::type = 0) const {
            GRIDTOOLS_STATIC_ASSERT(
                (EsfArguments::is_reduction_t::value), "Reductions not supported at the moment for icosahedral grids");
            //            typedef typename EsfArguments::reduction_data_t::bin_op_t bin_op_t;
            //            GRIDTOOLS_STATIC_ASSERT((is_esf_arguments< EsfArguments >::value), "Internal Error: wrong
            //            type");
            //            typedef typename EsfArguments::functor_t functor_t;
            //            this->m_iterate_domain.set_reduction_value(bin_op_t()(this->m_iterate_domain.reduction_value(),
            //                functor_t::f_type::Do(this->m_iterate_domain, IntervalType())));
        }
    };
}