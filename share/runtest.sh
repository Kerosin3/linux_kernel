#!/bin/sh

ERROR_COUNT=0

# Load the module
insmod /mnt/kernel_fifo.ko
if [ $? -ne 0 ]; then
    echo "Module load: FAIL"
    ERROR_COUNT=$((ERROR_COUNT + 1))
    exit 1
else
    echo "Module load: PASS"
fi

PARAM_DIR="/sys/module/kernel_fifo/parameters"

# Test initial state
if [ "$(cat $PARAM_DIR/is_empty)" = "1" ]; then
    echo "Initial is_empty: PASS"
else
    echo "Initial is_empty: FAIL"
    ERROR_COUNT=$((ERROR_COUNT + 1))
fi

if [ "$(cat $PARAM_DIR/is_full)" = "0" ]; then
    echo "Initial is_full: PASS"
else
    echo "Initial is_full: FAIL"
    ERROR_COUNT=$((ERROR_COUNT + 1))
fi

if [ "$(cat $PARAM_DIR/size)" = "0" ]; then
    echo "Initial size: PASS"
else
    echo "Initial size: FAIL"
    ERROR_COUNT=$((ERROR_COUNT + 1))
fi

# Determine capacity from initial available
CAP=$(cat $PARAM_DIR/available)
echo "Detected capacity: $CAP elements"

if [ $CAP -le 0 ]; then
    echo "Capacity detection: FAIL"
    ERROR_COUNT=$((ERROR_COUNT + 1))
    rmmod kernel_fifo
    exit 1
fi

# Test dequeue and peek on empty queue (expect failure)
cat $PARAM_DIR/dequeue 2>&1
if [ $? -ne 0 ]; then
    echo "Dequeue on empty: PASS (failed as expected)"
else
    echo "Dequeue on empty: FAIL"
    ERROR_COUNT=$((ERROR_COUNT + 1))
fi

cat $PARAM_DIR/peek 2>&1
if [ $? -ne 0 ]; then
    echo "Peek on empty: PASS (failed as expected)"
else
    echo "Peek on empty: FAIL"
    ERROR_COUNT=$((ERROR_COUNT + 1))
fi

# Enqueue sequential values until full
i=1
while [ $i -le $((CAP + 1)) ]; do
    echo $i > $PARAM_DIR/enqueue
    if [ $? -ne 0 ]; then
        break
    fi
    i=$((i + 1))
done

# Check if filled to capacity
if [ "$(cat $PARAM_DIR/size)" = "$CAP" ] && [ "$(cat $PARAM_DIR/is_full)" = "1" ] && [ "$(cat $PARAM_DIR/is_empty)" = "0" ] && [ "$(cat $PARAM_DIR/available)" = "0" ]; then
    echo "Fill to full: PASS"
else
    echo "Fill to full: FAIL"
    ERROR_COUNT=$((ERROR_COUNT + 1))
fi

# Test enqueue on full (expect failure)
echo 100 > $PARAM_DIR/enqueue 2>&1
if [ $? -ne 0 ]; then
    echo "Enqueue on full: PASS (failed as expected)"
else
    echo "Enqueue on full: FAIL"
    ERROR_COUNT=$((ERROR_COUNT + 1))
fi

# Test peek on full queue (should return first enqueued: 1)
PEEK_VAL=$(cat $PARAM_DIR/peek)
if [ "$PEEK_VAL" = "1" ] && [ "$(cat $PARAM_DIR/size)" = "$CAP" ]; then
    echo "Peek on full: PASS"
else
    echo "Peek on full: FAIL"
    ERROR_COUNT=$((ERROR_COUNT + 1))
fi

# Dequeue all values and verify FIFO order
FAIL=0
for i in $(seq 1 $CAP); do
    VAL=$(cat $PARAM_DIR/dequeue)
    if [ $? -ne 0 ] || [ "$VAL" != "$i" ]; then
        FAIL=1
        break
    fi
done
if [ $FAIL -eq 0 ] && [ "$(cat $PARAM_DIR/is_empty)" = "1" ] && [ "$(cat $PARAM_DIR/size)" = "0" ] && [ "$(cat $PARAM_DIR/available)" = "$CAP" ]; then
    echo "Dequeue all (order check): PASS"
else
    echo "Dequeue all (order check): FAIL"
    ERROR_COUNT=$((ERROR_COUNT + 1))
fi

# Partial fill for clear test
for i in $(seq 1 10); do
    echo $i > $PARAM_DIR/enqueue
done
if [ "$(cat $PARAM_DIR/size)" = "10" ]; then
    echo "Partial fill: PASS"
else
    echo "Partial fill: FAIL"
    ERROR_COUNT=$((ERROR_COUNT + 1))
fi

# Clear the queue
echo 1 > $PARAM_DIR/clear
if [ $? -eq 0 ] && [ "$(cat $PARAM_DIR/is_empty)" = "1" ] && [ "$(cat $PARAM_DIR/size)" = "0" ] && [ "$(cat $PARAM_DIR/available)" = "$CAP" ]; then
    echo "Clear: PASS"
else
    echo "Clear: FAIL"
    ERROR_COUNT=$((ERROR_COUNT + 1))
fi

# Unload the module
rmmod kernel_fifo
if [ $? -eq 0 ]; then
    echo "Module unload: PASS"
else
    echo "Module unload: FAIL"
    ERROR_COUNT=$((ERROR_COUNT + 1))
fi

# Final check
if [ $ERROR_COUNT -eq 0 ]; then
    echo "All tests were complited succesfully"
else
    echo "Some test failed ( FAIL test count: $ERROR_COUNT)"
fi
