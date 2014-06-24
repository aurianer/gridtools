#include <gridtools.h>
#include <common/halo_descriptor.h>

#include <boundary-conditions/apply.h>

using gridtools::direction;
using gridtools::sign;
using gridtools::minus;
using gridtools::zero;
using gridtools::plus;

#ifdef CUDA_EXAMPLE
#include <stencil-composition/backend_cuda.h>
#else
#include <stencil-composition/backend_block.h>
#include <stencil-composition/backend_naive.h>
#endif

#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>

#include <stdlib.h>
#include <stdio.h>

#ifdef CUDA_EXAMPLE
#define BACKEND backend_cuda
#else
#ifdef BACKEND_BLOCK
#define BACKEND backend_block
#else
#define BACKEND backend_naive
#endif
#endif


template <typename T>
struct direction_bc_input {
    T value;

    direction_bc_input()
        : value(1)
    {}

    direction_bc_input(T v)
        : value(v)
    {}

    // relative coordinates
    template <typename Direction, typename DataField0, typename DataField1>
    void operator()(Direction,
                    DataField0 & data_field0, DataField1 const & data_field1,
                    int i, int j, int k) const {
        std::cout << "General implementation AAA" << std::endl;
        data_field0(i,j,k) = data_field1(i,j,k) * value;
    }

    // relative coordinates
    template <sign I, sign K, typename DataField0, typename DataField1>
    void operator()(direction<I, minus, K>,
                    DataField0 & data_field0, DataField1 const & data_field1,
                    int i, int j, int k) const {
        std::cout << "Implementation going A-A" << std::endl;
        data_field0(i,j,k) = 88 * value;
    }

    // relative coordinates
    template <sign K, typename DataField0, typename DataField1>
    void operator()(direction<minus, minus, K>,
                    DataField0 & data_field0, DataField1 const & data_field1,
                    int i, int j, int k) const {
        std::cout << "Implementation going --A" << std::endl;
        data_field0(i,j,k) = 77777 * value;
    }

    template <typename DataField0, typename DataField1>
    void operator()(direction<minus, minus, minus>,
                    DataField0 & data_field0, DataField1 const & data_field1,
                    int i, int j, int k) const {
        std::cout << "Implementation going ---" << std::endl;
        data_field0(i,j,k) = 55555 * value;
    }
};



int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " dimx dimy dimz\n"
               " where args are integer sizes of the data fields" << std::endl;
        return EXIT_FAILURE;
    }

    int d1 = atoi(argv[1]);
    int d2 = atoi(argv[2]);
    int d3 = atoi(argv[3]);

    typedef gridtools::BACKEND::storage_type<int, gridtools::layout_map<0,1,2> >::type storage_type;

    // Definition of the actual data fields that are used for input/output
    storage_type in(d1,d2,d3,-1, std::string("in"));
    storage_type out(d1,d2,d3,-7.3, std::string("out"));
    storage_type coeff(d1,d2,d3,8, std::string("coeff"));

    for (int i=0; i<d1; ++i) {
        for (int j=0; j<d2; ++j) {
            for (int k=0; k<d3; ++k) {
                in(i,j,k) = 0;
                out(i,j,k) = i+j+k;
            }
        }
    }

    for (int i=0; i<d1; ++i) {
        for (int j=0; j<d2; ++j) {
            for (int k=0; k<d3; ++k) {
                printf("%d ", in(i,j,k));
            }
            printf("\n");
        }
        printf("\n");
    }

    gridtools::array<gridtools::halo_descriptor, 3> halos;
    halos[0] = gridtools::halo_descriptor(1,1,1,d1-2,d1);
    halos[1] = gridtools::halo_descriptor(1,1,1,d2-2,d2);
    halos[2] = gridtools::halo_descriptor(1,1,1,d3-2,d3);

    gridtools::boundary_apply<direction_bc_input<int> >(halos).apply(in, out);

    for (int i=0; i<d1; ++i) {
        for (int j=0; j<d2; ++j) {
            for (int k=0; k<d3; ++k) {
                printf("%d ", in(i,j,k));
            }
            printf("\n");
        }
        printf("\n");
    }

    printf("\nNow doing the same but with a stateful user struct:\n\n");

    gridtools::boundary_apply<direction_bc_input<int> >(halos, direction_bc_input<int>(2)).apply(in, out);

    for (int i=0; i<d1; ++i) {
        for (int j=0; j<d2; ++j) {
            for (int k=0; k<d3; ++k) {
                printf("%d ", in(i,j,k));
            }
            printf("\n");
        }
        printf("\n");
    }
}