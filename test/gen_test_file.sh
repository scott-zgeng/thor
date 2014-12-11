#!/bin/bash

FILE_NAME=$1


rm -rf  ${FILE_NAME}.cpp
cat >> ${FILE_NAME}.cpp << EOF
// DON'T EDIT, THIS FILE IS AUTOGEN BY $0

#include <gtest/gtest.h>
#include "${FILE_NAME}.h"
 

int main(int argc, char* argv[])
{
    int ret = 0;
    testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();
    return ret;
}

EOF


echo "generate the ${FILE_NAME}.cpp completed."
