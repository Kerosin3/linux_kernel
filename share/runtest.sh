#!/bin/sh

SYSFS_DIR="/sys/kernel/path_scanner"
TEST_DIR="/mnt/testdir"

insmod /mnt/ex_bin_tree.ko
if [ $? -ne 0 ]; then
    echo "Failed to load ex_bin_tree.ko"
    exit 1
fi

insmod /mnt/ex_sub_bin_tree.ko
if [ $? -ne 0 ]; then
    echo "Failed to load ex_sub_bin_tree.ko"
    rmmod ex_bin_tree.ko
    exit 1
fi

# Clear the hash table
echo "1" > "$SYSFS_DIR/clean"

# Trigger scan
echo "$TEST_DIR" > "$SYSFS_DIR/scan_path"

# Read sorted output
sorted_output=$(cat "$SYSFS_DIR/sorted_files")

# Display output
echo "Sorted files from sysfs:"
echo "$sorted_output"

expected_sorted=$(sort /mnt/testfilenames)

echo "$sorted_output" > /mnt/temp_sorted.txt
echo "$expected_sorted" > /mnt/temp_expected.txt

diff_output=$(diff -u /mnt/temp_sorted.txt /mnt/temp_expected.txt)

# Clean up temp files
rm -f /mnt/temp_sorted.txt /mnt/temp_expected.txt

if [ -z "$diff_output" ]; then
    echo "All tests were completed successfully"
else
    echo "Verification failed:"
    echo "$diff_output"
    # Unload modules on failure
    rmmod ex_sub_bin_tree.ko
    rmmod ex_bin_tree.ko
    exit 1
fi

# Unload modules on success
rmmod ex_sub_bin_tree.ko
rmmod ex_bin_tree.ko
