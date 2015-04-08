#pragma once

class verifier
{
public:
    verifier(const double precision, const int halo_size) : m_precision(precision), m_halo_size(halo_size) {}
    ~verifier(){}

    template<typename storage_type>
    bool verify(storage_type& field1, storage_type& field2)
    {
        assert(field1.template dims<0>() == field2.template dims<0>());
        assert(field1.template dims<1>() == field2.template dims<1>());
        assert(field1.template dims<2>() == field2.template dims<2>());

        const int idim = field1.template dims<0>();
        const int jdim = field1.template dims<1>();
        const int kdim = field1.template dims<2>();

        bool verified = true;
        for(int i=m_halo_size; i < idim-m_halo_size; ++i)
        {
            for(int j=m_halo_size; j < jdim-m_halo_size; ++j)
            {
                for(int k=0; k < kdim; ++k)
                {
                    typename storage_type::value_type expected = field1(i,j,k);
                    typename storage_type::value_type actual = field2(i,j,k);

                    if(!compare_below_threashold(expected, actual))
                    {
                        // std::cout << "Error in position " << i << " " << j << " " << k << " ; expected : " << expected <<
                        //         " ; actual : " << actual << "  " << std::fabs((expected-actual)/(expected))  << std::endl;
                        verified = false;
                    }
                }
            }
        }

        return verified;
    }
private:
    template<typename value_type>
    bool compare_below_threashold(value_type expected, value_type actual)
    {
        if (std::fabs(expected) < 1e-3 && std::fabs(actual) < 1e-3)
        {
            if(std::fabs(expected-actual) < m_precision) return true;
        }
        else
        {
            if(std::fabs((expected-actual)/(m_precision*expected)) < 1.0) return true;
        }
        return false;
    }
    double m_precision;
    int m_halo_size;
};