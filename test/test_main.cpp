#include <gtest/gtest.h>



//#include "test_mem_pool.h" 
#include "test_db.h"
 

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    getchar();
    return ret;
}

