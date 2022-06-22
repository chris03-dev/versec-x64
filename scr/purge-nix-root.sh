#!/bin/sh

EXENAME="verse"

echo "Running *nix purge script for '$EXENAME'..."
cd $(dirname "$0")

# Check if necessary permissions (i.e. root) are enabled for writing to /usr/local/bin
if [ `whoami` != root ]; then
    echo "Cannot write to '/usr/local/bin/' due to non-root shell command."
    echo "Run the script as root user or input 'sudo sh <file>'."
	exit 1

# Delete executable in /usr/local/bin
else
    if rm /usr/local/bin/$EXENAME; then
		echo "Deletion in 'usr/local/bin/' successful."
		exit 0
	else
		echo "Deletion failed."
		exit 1
	fi
fi
