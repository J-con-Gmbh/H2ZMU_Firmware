#!/bin/bash

# Run the process with a timeout of 5 seconds
timeout -s 15 5 ./build/src/h2zmu_v1 $(pwd)/files/

# Capture the exit code of the timeout command
EXIT_CODE=$?

# Check the exit code and handle accordingly
if [ $EXIT_CODE -eq 124 ]; then
  echo "SUCCESS: Process was killed due to timeout"
  exit 0
else
  echo "FAILED: Process exited with an error (exit code: $EXIT_CODE)"
  exit 1
fi