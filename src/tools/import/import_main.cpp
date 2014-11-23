// for the test

#include "hash_join.h"
#include "hash_group.h"

//int main(int argc, char* argv[])
//{
//    DB_TRACE("TEST PARALLETING HASH JOIN");
//    current_hash_join_t::main();
//    return 0;
//}

int main(int argc, char* argv[])
{
    DB_TRACE("TEST PARALLETING HASH JOIN");
    current_hash_group_t instance;
    instance.test(argc, argv);
    return 0;
}


