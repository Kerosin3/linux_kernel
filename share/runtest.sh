#!/bin/sh

MODULE_PATH="/mnt/kernel_stack.ko"
MODULE_NAME="kernel_stack"
SYS_PATH="/sys/kernel/${MODULE_NAME}"

# output error and exit
die() {
    echo "ERROR: $1" >&2
    exit 1
}

# check if module is loaded
is_module_loaded() {
    lsmod | grep "^${MODULE_NAME}" >/dev/null 2>&1
}

# 1. Check if module file exists
echo "Checking module file..."
if [ ! -f "${MODULE_PATH}" ]; then
    die "Module file not found: ${MODULE_PATH}"
fi

# 2. Unload module if already loaded
if is_module_loaded; then
    echo "Module ${MODULE_NAME} is already loaded. Unloading..."
    rmmod "${MODULE_NAME}" || die "Failed to unload existing module ${MODULE_NAME}"
fi

# 3. Load module
echo "Loading module: ${MODULE_PATH}"
insmod "${MODULE_PATH}" || die "Error loading module ${MODULE_PATH}"

sleep 1

# Verify module loaded
if ! is_module_loaded; then
    die "Module ${MODULE_NAME} did not appear in loaded modules list"
fi
echo "Module loaded successfully"

# 4. Check sysfs attributes
for attr in push peek pop size is_empty clear; do
    if [ ! -f "${SYS_PATH}/${attr}" ]; then
        die "Missing sysfs attribute: ${SYS_PATH}/${attr}"
    fi
done
echo "All sysfs attributes present"

# 5. Test 1: Empty stack after loading
echo "Test 1: Checking empty stack"
size=$(cat "${SYS_PATH}/size")
if [ "${size}" != "0" ]; then
    die "After loading, size should be 0, but it is ${size}"
fi
empty=$(cat "${SYS_PATH}/is_empty")
if [ "${empty}" != "1" ]; then
    die "After loading, is_empty should be 1, but it is ${empty}"
fi
# Check peek/pop on empty stack
peek=$(cat "${SYS_PATH}/peek" 2>/dev/null)
if [ -n "${peek}" ]; then
    echo "Warning: peek on empty stack returned ${peek} (expected error)"
fi
pop=$(cat "${SYS_PATH}/pop" 2>/dev/null)
if [ -n "${pop}" ]; then
    echo "Warning: pop on empty stack returned ${pop} (expected error)"
fi

# 6. Test 2: Adding elements
echo "Test 2: Adding elements"
echo 10 > "${SYS_PATH}/push" || die "Error pushing 10"
echo 20 > "${SYS_PATH}/push" || die "Error pushing 20"
echo 30 > "${SYS_PATH}/push" || die "Error pushing 30"
echo 40 > "${SYS_PATH}/push" || die "Error pushing 40"
echo 50 > "${SYS_PATH}/push" || die "Error pushing 50"
size=$(cat "${SYS_PATH}/size")
if [ "${size}" != "5" ]; then
    die "After adding 5 elements, size should be 5, but it is ${size}"
fi
empty=$(cat "${SYS_PATH}/is_empty")
if [ "${empty}" != "0" ]; then
    die "After adding, is_empty should be 0, but it is ${empty}"
fi

# 7. Test 3: Peek
echo "Test 3: Peek"
peek=$(cat "${SYS_PATH}/peek")
if [ "${peek}" != "50" ]; then
    die "Peek returned ${peek}, expected 50"
fi
peek2=$(cat "${SYS_PATH}/peek")
if [ "${peek2}" != "50" ]; then
    die "Repeated peek returned ${peek2}, expected 50"
fi
size=$(cat "${SYS_PATH}/size")  # Size should not change
if [ "${size}" != "5" ]; then
    die "After peek, size should remain 5, but it is ${size}"
fi

# 8. Test 4: Pop
echo "Test 4: Pop"
pop=$(cat "${SYS_PATH}/pop")
if [ "${pop}" != "50" ]; then
    die "Pop returned ${pop}, expected 50"
fi
pop=$(cat "${SYS_PATH}/pop")
if [ "${pop}" != "40" ]; then
    die "Pop returned ${pop}, expected 40"
fi
pop=$(cat "${SYS_PATH}/pop")
if [ "${pop}" != "30" ]; then
    die "Pop returned ${pop}, expected 30"
fi
size=$(cat "${SYS_PATH}/size")
if [ "${size}" != "2" ]; then
    die "After 3 pops, size should be 2, but it is ${size}"
fi

# 9. Test 5: Clear and recheck
echo "Test 5: Clear"
echo 1 > "${SYS_PATH}/clear" || die "Error clearing"
size=$(cat "${SYS_PATH}/size")
if [ "${size}" != "0" ]; then
    die "After clear, size should be 0, but it is ${size}"
fi
empty=$(cat "${SYS_PATH}/is_empty")
if [ "${empty}" != "1" ]; then
    die "After clear, is_empty should be 1, but it is ${empty}"
fi

# 10. Test 6: Refill and mixed operations
echo "Test 6: Refill and mixed operations"
echo 100 > "${SYS_PATH}/push"
echo 200 > "${SYS_PATH}/push"
peek=$(cat "${SYS_PATH}/peek")
if [ "${peek}" != "200" ]; then
    die "Peek returned ${peek}, expected 200"
fi
pop=$(cat "${SYS_PATH}/pop")
if [ "${pop}" != "200" ]; then
    die "Pop returned ${pop}, expected 200"
fi
echo 300 > "${SYS_PATH}/push"
size=$(cat "${SYS_PATH}/size")
if [ "${size}" != "2" ]; then
    die "After mixed operations, size should be 2, but it is ${size}"
fi
echo 1 > "${SYS_PATH}/clear"

echo "All tests passed"
echo "Unloading module..."
rmmod "${MODULE_NAME}" || die "Error unloading module ${MODULE_NAME}"
echo "Testing completed"
