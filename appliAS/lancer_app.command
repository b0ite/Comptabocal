#!/bin/bash

# Change directory to the script's location
cd "$(dirname "$0")"

# Check if Python 3 is installed
if ! command -v python3 &> /dev/null; then
    echo "Python 3 is not installed. Please install Python 3 and try again."
    echo "Press any key to close this window..."
    read -n 1
    exit 1
fi

# Check if required packages are installed
python3 -c "import tkinter" &> /dev/null
if [ $? -ne 0 ]; then
    echo "The tkinter package is not installed. Please install it and try again."
    echo "Press any key to close this window..."
    read -n 1
    exit 1
fi

# Run the application
echo "Starting ParserBocal application..."
python3 stuffs/app.py

# If the application exits with an error, keep the window open
if [ $? -ne 0 ]; then
    echo "Application exited with an error."
    echo "Press any key to close this window..."
    read -n 1
fi
