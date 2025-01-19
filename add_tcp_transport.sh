#!/bin/bash

# Directory containing the test cases
test_dir="./tests"

# Line to be added or removed
line_to_manage="export UCX_TLS=tcp"

# Check if the correct number of arguments is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <1|0>"
    echo "1: Add the TCP transport line"
    echo "0: Remove the TCP transport line"
    exit 1
fi

# Check if the provided argument is valid
option="$1"
if [[ "$option" != "1" && "$option" != "0" ]]; then
    echo "Invalid option. Use 1 to add or 0 to remove the line."
    exit 1
fi

# Loop through all .sh files in the test directory
for file in "$test_dir"/*.sh; do
    # Skip if no files are found
    if [ ! -e "$file" ]; then
        echo "No .sh files found in $test_dir."
        exit 0
    fi

    if [ "$option" -eq 1 ]; then
        # Add the line before each srun command if it doesn't already exist
        if ! grep -qF "$line_to_manage" "$file"; then
            sed -i "/srun /i$line_to_manage" "$file"
            echo "Added line to $file"
        else
            echo "Line already exists in $file"
        fi
    elif [ "$option" -eq 0 ]; then
        # Remove the line if it exists
        if grep -qF "$line_to_manage" "$file"; then
            sed -i "/$line_to_manage/d" "$file"
            echo "Removed line from $file"
        else
            echo "Line not found in $file"
        fi
    fi
done