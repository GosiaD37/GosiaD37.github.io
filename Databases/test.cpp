#include "pch.h"

// Global test environment setup and cleanup.
class Environment : public ::testing::Environment
{
public:
    ~Environment() override {}

    // Define how to initialize the environment.
    void SetUp() override
    {
        // Seed the random number generator.
        srand(time(nullptr));
    }

    // Define how to clean up the environment.
    void TearDown() override {}
};

// Test class containing shared data across tests.
class CollectionTest : public ::testing::Test
{
protected:
    // Smart pointer to manage the collection.
    std::unique_ptr<std::vector<int>> collection;

    void SetUp() override
    {
        // Instantiate a new collection for testing.
        collection.reset(new std::vector<int>);
    }

    void TearDown() override
    {
        // Remove all elements from the collection.
        collection->clear();
        // Deallocate memory.
        collection.reset(nullptr);
    }

    // Utility function to add a specified number of random values (0-99) to the collection.
    void add_entries(int count)
    {
        if (count > 0)
        {
            for (auto i = 0; i < count; ++i)
            {
                collection->push_back(rand() % 100);
            }
        }
    }
};

struct ParameterizedCollectionTest : CollectionTest, ::testing::WithParamInterface<int>
{
    ParameterizedCollectionTest() {}
};

INSTANTIATE_TEST_CASE_P(CollectionSizes, ParameterizedCollectionTest, ::testing::Values(0, 1, 5, 10));

// Verify that the smart pointer for the collection is initialized correctly.
TEST_F(CollectionTest, CollectionSmartPointerIsNotNull)
{
    ASSERT_TRUE(collection);
    ASSERT_NE(collection.get(), nullptr);
}

// Verify that a newly created collection is empty.
TEST_F(CollectionTest, IsEmptyOnCreate)
{
    ASSERT_TRUE(collection->empty());
    ASSERT_EQ(collection->size(), 0);
}

// Validate adding a single value to an empty collection.
TEST_F(CollectionTest, CanAddToEmptyCollection)
{
    EXPECT_TRUE(collection->empty());
    EXPECT_EQ(collection->size(), 0);
    add_entries(1);
    ASSERT_FALSE(collection->empty());
    ASSERT_EQ(collection->size(), 1);
}

// Validate inserting five values into an empty collection.
TEST_F(CollectionTest, CanAddFiveValuesToCollection)
{
    EXPECT_TRUE(collection->empty());
    EXPECT_EQ(collection->size(), 0);
    add_entries(5);
    ASSERT_FALSE(collection->empty());
    ASSERT_EQ(collection->size(), 5);
}

// Ensure that the maximum allowable size is greater than or equal to the current size.
TEST_P(ParameterizedCollectionTest, MaxSizeGreaterThanSize)
{
    EXPECT_TRUE(collection->empty());
    EXPECT_EQ(collection->size(), 0);
    add_entries(GetParam());
    ASSERT_EQ(collection->size(), GetParam());
    ASSERT_GT(collection->max_size(), collection->size());
}

// Ensure the collection's capacity is always greater than or equal to its size.
TEST_P(ParameterizedCollectionTest, CapacityGreaterThanOrEqualToSize)
{
    EXPECT_TRUE(collection->empty());
    EXPECT_EQ(collection->size(), 0);
    add_entries(GetParam());
    ASSERT_EQ(collection->size(), GetParam());
    ASSERT_GE(collection->capacity(), collection->size());
}

// Confirm that resizing increases collection size.
TEST_F(CollectionTest, ResizeIncreasesCollectionSize)
{
    EXPECT_TRUE(collection->empty());
    EXPECT_EQ(collection->size(), 0);
    collection->resize(5);
    ASSERT_EQ(collection->size(), 5);
}

// Confirm that resizing decreases collection size.
TEST_F(CollectionTest, ResizeDecreasesCollectionSize)
{
    EXPECT_TRUE(collection->empty());
    EXPECT_EQ(collection->size(), 0);
    add_entries(5);
    EXPECT_FALSE(collection->empty());
    EXPECT_EQ(collection->size(), 5);
    collection->resize(2);
    ASSERT_EQ(collection->size(), 2);
}

// Ensure that resizing down to zero clears the collection.
TEST_F(CollectionTest, ResizeCollectionSizeToZero)
{
    EXPECT_TRUE(collection->empty());
    EXPECT_EQ(collection->size(), 0);
    add_entries(5);
    EXPECT_FALSE(collection->empty());
    EXPECT_EQ(collection->size(), 5);
    collection->resize(0);
    ASSERT_EQ(collection->size(), 0);
}

// Validate that clearing the collection removes all elements.
TEST_F(CollectionTest, ClearCollection)
{
    EXPECT_TRUE(collection->empty());
    EXPECT_EQ(collection->size(), 0);
    add_entries(5);
    EXPECT_FALSE(collection->empty());
    EXPECT_EQ(collection->size(), 5);
    collection->clear();
    ASSERT_TRUE(collection->empty());
    ASSERT_EQ(collection->size(), 0);
}

// Validate that reserving capacity increases available storage but does not affect size.
TEST_F(CollectionTest, ReserveIncreaseCollectionCapacity)
{
    EXPECT_TRUE(collection->empty());
    EXPECT_EQ(collection->size(), 0);
    EXPECT_EQ(collection->capacity(), 0);
    collection->reserve(5);
    ASSERT_TRUE(collection->empty());
    ASSERT_EQ(collection->size(), 0);
    ASSERT_GT(collection->capacity(), collection->size());
}

// Ensure an out-of-range exception is thrown when accessing an invalid index.
TEST_F(CollectionTest, OutOfRangeExceptionThrown)
{
    EXPECT_TRUE(collection->empty());
    EXPECT_EQ(collection->size(), 0);
    ASSERT_THROW(collection->at(5), std::out_of_range);
}

// Verify that the assign function correctly initializes a collection with specific values.
TEST_F(CollectionTest, AssignValuesToCollection)
{
    EXPECT_TRUE(collection->empty());
    EXPECT_EQ(collection->size(), 0);
    collection->assign(5, 10);
    ASSERT_FALSE(collection->empty());
    ASSERT_EQ(collection->size(), 5);
    for (int i = 0; i < 5; i++)
    {
        ASSERT_EQ(collection->at(i), 10);
    }
}

// Ensure attempting to reserve a size greater than the max allowed results in an error.
TEST_F(CollectionTest, IncreaseCollectionReserveAboveMaxSize)
{
    EXPECT_TRUE(collection->empty());
    EXPECT_EQ(collection->size(), 0);
    ASSERT_THROW(collection->reserve(collection->max_size() + 10), std::length_error);
}
