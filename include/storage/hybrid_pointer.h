#pragma once

#include <iostream>
#include <stdio.h>
#include <boost/current_function.hpp>

#include "../common/gt_assert.h"
#include "wrap_pointer.h"

/** @file
    @brief double pointer mapping host and device
    implementation of a double pointer, living on both host and device, together with the algorithms to copy back/to the device. The device is supposed to be a GPU supporting CUDA.
*/
namespace gridtools {

/**\todo Note that this struct will greatly simplify when the CUDA arch 3200 and inferior will be obsolete (the "pointer_to_use" will then become useless, and the operators defined in the base class will be usable) */
    template <typename T>
    struct hybrid_pointer : public wrap_pointer<T>{

        typedef wrap_pointer<T> super;
        typedef typename super::pointee_t pointee_t;

        explicit hybrid_pointer(T* p) : wrap_pointer<T>(p), m_gpu_p(NULL), m_pointer_to_use(p), m_size(0) {}

        explicit hybrid_pointer(uint_t size) : wrap_pointer<T>(size), m_size(size) {
            allocate_it(size);
            m_pointer_to_use = this->cpu_p;
#ifndef NDEBUG
            printf(" - %X %X %X %d\n", this->cpu_p, m_gpu_p, m_pointer_to_use, m_size);
#endif
        }

// copy constructor passes on the ownership
        __device__ __host__
        explicit hybrid_pointer(hybrid_pointer const& other)
            : wrap_pointer<T>(other)
            , m_gpu_p(other.m_gpu_p)
#if defined(__CUDA_ARCH__) && (__CUDA_ARCH__ < 3200)
            , m_pointer_to_use(m_gpu_p)
#else
            , m_pointer_to_use(this->cpu_p)
#endif
            , m_size(other.m_size)
        {
#ifndef NDEBUG
            printf("cpy const hp ");
            printf("%X ", this->cpu_p);
            printf("%X ", m_gpu_p);
            printf("%X ", m_pointer_to_use);
            printf("%d ", m_size);
            printf("\n");
#endif
        }

        virtual ~hybrid_pointer(){  };

        void allocate_it(uint_t size) {
#ifdef __CUDACC__
            int err = cudaMalloc(&m_gpu_p, size*sizeof(T));
            if (err != cudaSuccess) {
                std::cout << "Error allocating storage in "
                          << BOOST_CURRENT_FUNCTION
                          << " : size = "
                          << size*sizeof(T)
                          << " bytes "
                          << std::endl;
            }
#endif
        }

        void free_it() {
#ifdef __CUDACC__
            cudaFree(m_gpu_p);
#endif
            wrap_pointer<T>::free_it();
      }

        void update_gpu() {
#ifdef __CUDACC__
#ifndef NDEBUG
            printf("update gpu "); out();
#endif
            cudaMemcpy(m_gpu_p, this->cpu_p, m_size*sizeof(T), cudaMemcpyHostToDevice);
#endif
        }

        void update_cpu() {
#ifdef __CUDACC__
#ifndef NDEBUG
            printf("update cpu "); out();
#endif
            cudaMemcpy(this->cpu_p, m_gpu_p, m_size*sizeof(T), cudaMemcpyDeviceToHost);
#endif
        }

        __host__ __device__
        void out() const {
            printf("out hp ");
            printf("%X ", this->cpu_p);
            printf("%X ", m_gpu_p);
            printf("%X ", m_pointer_to_use);
            printf("%d ", m_size);
            printf("\n");
        }

        __host__ __device__
        operator T*() {
            return m_pointer_to_use;
        }

        __host__ __device__
        operator T const*() const {
            return m_pointer_to_use;
        }

        __host__ __device__
        T& operator[](uint_t i) {
            /* assert(i<size); */
            /* assert(i>=0); */
            // printf(" [%d %e] ", i, m_pointer_to_use[i]);
            return m_pointer_to_use[i];
        }

        __host__ __device__
        T const& operator[](uint_t i) const {
            /* assert(i<size); */
            /* assert(i>=0); */
            // printf(" [%d %e] ", i, m_pointer_to_use[i]);

            return m_pointer_to_use[i];
        }

        __host__ __device__
        T& operator*() {
            return *m_pointer_to_use;
        }

        __host__ __device__
        T const& operator*() const {
            return *m_pointer_to_use;
        }

        __host__ __device__
        T* operator+(uint_t i) {
            return &m_pointer_to_use[i];
        }

        __host__ __device__
        T* const& operator+(uint_t i) const {
            return &m_pointer_to_use[i];
        }

        GT_FUNCTION
        T* get_gpu_p(){return m_gpu_p;};

        GT_FUNCTION
        T* get_pointer_to_use(){return m_pointer_to_use;}

        GT_FUNCTION
        int get_size(){return m_size;}

    private:
        T * m_gpu_p;
        T * m_pointer_to_use;
        uint_t m_size;
    };

} // namespace gridtools
