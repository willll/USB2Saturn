#!/bin/bash

# Exit on any error
set -e

echo "Building Docker image 'usb2saturn-builder'..."
docker build -t usb2saturn-builder .

echo "Running compilation inside Docker container..."
# Run the container, mount the current directory to /workspace
docker run --rm -v "$(pwd)":/workspace usb2saturn-builder

echo "Build complete! Check the 'build' directory for your .uf2 file."
