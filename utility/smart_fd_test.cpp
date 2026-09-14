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

// Checks whether a raw fd number is still a valid open descriptor,
// without closing it (so we don't risk closing an unrelated fd that
// the OS may have since reused for something else).
bool IsFdOpen(int fd) {
    return ::fcntl(fd, F_GETFD) != -1;
}

}  // namespace

// ------------------------------------------------------------
// Construction
// ------------------------------------------------------------

TEST(SmartFdTest, DefaultConstructorCreatesInvalidFd) {
    SmartFd fd;

    EXPECT_FALSE(fd.IsValid());
    EXPECT_EQ(fd.Get(), -1);
}

TEST(SmartFdTest, ConstructorTakesOwnershipOfFd) {
    int raw_fd = CreateTestFd();
    ASSERT_NE(raw_fd, -1);

    SmartFd fd(raw_fd);

    EXPECT_TRUE(fd.IsValid());
    EXPECT_EQ(fd.Get(), raw_fd);
}


// ------------------------------------------------------------
// Reset
// ------------------------------------------------------------

TEST(SmartFdTest, ResetClosesCurrentFdAndBecomesInvalid) {
    int raw_fd = CreateTestFd();
    ASSERT_NE(raw_fd, -1);

    SmartFd fd(raw_fd);

    EXPECT_TRUE(fd.IsValid());

    fd.Reset();

    EXPECT_FALSE(fd.IsValid());
    EXPECT_EQ(fd.Get(), -1);

    // The FD should have been closed.
    EXPECT_FALSE(IsFdOpen(raw_fd));
}

TEST(SmartFdTest, ResetReplacesFd) {
    int first_fd = CreateTestFd();
    int second_fd = CreateTestFd();

    ASSERT_NE(first_fd, -1);
    ASSERT_NE(second_fd, -1);

    SmartFd fd(first_fd);

    fd.Reset(second_fd);

    EXPECT_TRUE(fd.IsValid());
    EXPECT_EQ(fd.Get(), second_fd);

    // first_fd should have been closed.
    EXPECT_FALSE(IsFdOpen(first_fd));

    // second_fd is owned by SmartFd, so don't touch it here.
}


// ------------------------------------------------------------
// Move constructor
// ------------------------------------------------------------

TEST(SmartFdTest, MoveConstructorTransfersOwnership) {
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

TEST(SmartFdTest, MoveAssignmentTransfersOwnership) {
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
    EXPECT_FALSE(IsFdOpen(first_fd));
}


// ------------------------------------------------------------
// Self move assignment
// ------------------------------------------------------------

TEST(SmartFdTest, SelfMoveAssignmentDoesNothing) {
    int raw_fd = CreateTestFd();
    ASSERT_NE(raw_fd, -1);

    SmartFd fd(raw_fd);

    // Route through a reference so the compiler can't flag this as an
    // obviously-self move at compile time (-Wself-move); this also better
    // mirrors how self-assignment actually happens in practice (via an
    // alias/reference, not a literal repeated variable name).
    SmartFd& fd_ref = fd;
    fd = std::move(fd_ref);

    EXPECT_TRUE(fd.IsValid());
    EXPECT_EQ(fd.Get(), raw_fd);
}


// ------------------------------------------------------------
// Destructor
// ------------------------------------------------------------

TEST(SmartFdTest, DestructorClosesFd) {
    int raw_fd = CreateTestFd();
    ASSERT_NE(raw_fd, -1);

    {
        SmartFd fd(raw_fd);

        EXPECT_TRUE(fd.IsValid());
    }

    // fd should have been closed when SmartFd was destroyed.
    EXPECT_FALSE(IsFdOpen(raw_fd));
}