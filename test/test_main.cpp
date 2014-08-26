#include <gtest/gtest.h>



//#include "test_mem_pool.h" 
#include "test_db.h"
 

int main(int argc, char* argv[])
{
    int ret = 0;
    //testing::InitGoogleTest(&argc, argv);
    //ret = RUN_ALL_TESTS();

    test_func();

    getchar();
    return ret;
}

