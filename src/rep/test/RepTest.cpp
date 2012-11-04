#include "gtest/gtest.h"
#include "rep/Instance.h"

TEST(Instance, CreateInstanceManager){
    Ptr<Instance::Manager> manager = shippingInstanceManager();
    ASSERT_TRUE(manager);
}

