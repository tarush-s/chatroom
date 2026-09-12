#include "smart_fd.h"

#include <fcntl.h>
#include <unistd.h>

#include <gtest/gtest.h>
#include <utility>

namespace {

// Creates a real OS file descriptor that we can use for testing.
int CreateTestFd() {
    return ::open("/dev/null", O_RDONLY);
}

}  // namespace

// ------------------------------------------------------------
// Construction
// ------------------------------------------------------------

TEST(UniqueFdTest, DefaultConstructorCreatesInvalidFd) {
    SmartFd fd;

    EXPECT_FALSE(fd.IsValid());
    EXPECT_EQ(fd.Get(), -1);
}

TEST(UniqueFdTest, ConstructorTakesOwnershipOfFd) {
    int raw_fd = CreateTestFd();
    ASSERT_NE(raw_fd, -1);

    SmartFd fd(raw_fd);

    EXPECT_TRUE(fd.IsValid());
    EXPECT_EQ(fd.Get(), raw_fd);
}


// ------------------------------------------------------------
// Reset
// ------------------------------------------------------------

TEST(UniqueFdTest, ResetClosesCurrentFdAndBecomesInvalid) {
    int raw_fd = CreateTestFd();
    ASSERT_NE(raw_fd, -1);

    SmartFd fd(raw_fd);

    EXPECT_TRUE(fd.IsValid());

    fd.Reset();

    EXPECT_FALSE(fd.IsValid());
    EXPECT_EQ(fd.Get(), -1);

    // The FD should have been closed.
    EXPECT_EQ(::close(raw_fd), -1);
}

TEST(UniqueFdTest, ResetReplacesFd) {
    int first_fd = CreateTestFd();
    int second_fd = CreateTestFd();

    ASSERT_NE(first_fd, -1);
    ASSERT_NE(second_fd, -1);

    SmartFd fd(first_fd);

    fd.Reset(second_fd);

    EXPECT_TRUE(fd.IsValid());
    EXPECT_EQ(fd.Get(), second_fd);

    // first_fd should have been closed.
    EXPECT_EQ(::close(first_fd), -1);

    // second_fd is owned by SmartFd, so don't close it here.
}


// ------------------------------------------------------------
// Move constructor
// ------------------------------------------------------------

TEST(UniqueFdTest, MoveConstructorTransfersOwnership) {
    int raw_fd = CreateTestFd();
    ASSERT_NE(raw_fd, -1);

    SmartFd original(raw_fd);

    SmartFd moved(std::move(original));

    // Original should no longer own the FD.
    EXPECT_FALSE(original.IsValid());
    EXPECT_EQ(original.Get(), -1);

    // New object should own it.
    EXPECT_TRUE(moved.IsValid());
    EXPECT_EQ(moved.Get(), raw_fd);
}


// ------------------------------------------------------------
// Move assignment
// ------------------------------------------------------------

TEST(UniqueFdTest, MoveAssignmentTransfersOwnership) {
    int first_fd = CreateTestFd();
    int second_fd = CreateTestFd();

    ASSERT_NE(first_fd, -1);
    ASSERT_NE(second_fd, -1);

    SmartFd first(first_fd);
    SmartFd second(second_fd);

    first = std::move(second);

    // first should now own second_fd.
    EXPECT_TRUE(first.IsValid());
    EXPECT_EQ(first.Get(), second_fd);

    // second should no longer own anything.
    EXPECT_FALSE(second.IsValid());
    EXPECT_EQ(second.Get(), -1);

    // first_fd should have been closed during move assignment.
    EXPECT_EQ(::close(first_fd), -1);
}


// ------------------------------------------------------------
// Self move assignment
// ------------------------------------------------------------

TEST(UniqueFdTest, SelfMoveAssignmentDoesNothing) {
    int raw_fd = CreateTestFd();
    ASSERT_NE(raw_fd, -1);

    SmartFd fd(raw_fd);

    fd = std::move(fd);

    EXPECT_TRUE(fd.IsValid());
    EXPECT_EQ(fd.Get(), raw_fd);
}


// ------------------------------------------------------------
// Destructor
// ------------------------------------------------------------

TEST(UniqueFdTest, DestructorClosesFd) {
    int raw_fd = CreateTestFd();
    ASSERT_NE(raw_fd, -1);

    {
        SmartFd fd(raw_fd);

        EXPECT_TRUE(fd.IsValid());
    }

    // fd should have been closed when SmartFd was destroyed.
    EXPECT_EQ(::close(raw_fd), -1);
}
