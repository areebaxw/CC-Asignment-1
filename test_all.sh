#!/bin/bash

echo "Testing all JSON files..."
echo "========================"

for file in test/*.json; do
    echo ""
    echo "Testing: $file"
    echo "---"
    ./json2xml.exe < "$file"
    echo ""
done

echo "========================"
echo "All tests completed."
