#include <gtest/gtest.h>

#include <iostream>
#include "ring_buffer.hpp"

class test_ring_buffer : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        buff = new gcom::ring_buffer<uint32_t>(100);
    }

    virtual void TearDown()
    {
        delete buff;
    }

    gcom::ring_buffer<uint32_t>* buff;
};

TEST_F(test_ring_buffer, push_case1)
{
    std::string str1(10, 'a');
    std::string str2(20, 'b');
    EXPECT_EQ(0, buff->push((unsigned char *)str1.c_str(), str1.size()));
    EXPECT_EQ(10, buff->push((unsigned char *)str2.c_str(), str2.size()));
}

TEST_F(test_ring_buffer, push_case2)
{
    std::string str1(5000, 'a');
    EXPECT_THROW(buff->push((unsigned char *)str1.c_str(), str1.size()), std::exception);
}

// TEST_F(TestRingBuffer, PushEmptyCase1)
// {
//     EXPECT_THROW(buff->push_empty(5000), std::exception);
//     EXPECT_EQ(10, buff->push_empty(10));
// }

// TEST_F(test_ring_buffer, pop_case1)
// {
//     std::string str1(10, 'a');
//     char str3[20];
//     EXPECT_EQ(0, buff->push((unsigned char *)str1.c_str(), str1.size()));
//     buff->pop((unsigned char *)str3, 10);
//     str3[10] = '\0';
//     std::cout << str3 << std::endl;
//     EXPECT_EQ(str1, str3);
// }

// TEST_F(TestRingBuffer, SetCase1)
// {
//     std::string str1(10, 'a');
//     ASSERT_THROW(buff->set(0, str1), std::exception);
// }

// TEST_F(TestRingBuffer, SetGetCase1)
// {
//     std::string str1(10, 'a');
//     std::string str2(20, 'b');
//     EXPECT_EQ(30, buff->push_empty(30));
//     ASSERT_NO_THROW(buff->set(0, str1));
//     EXPECT_EQ(str1, buff->get(0, 10));
//     ASSERT_NO_THROW(buff->set(10, str2));
//     EXPECT_EQ(str2, buff->get(10, 20));
//     EXPECT_EQ(str1 + str2, buff->pop(30));
// }