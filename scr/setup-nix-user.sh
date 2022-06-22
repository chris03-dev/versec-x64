#!/bin/sh

EXENAME="verse"

echo "Running *nix setup script for '$EXENAME'..."
cd $(dirname "$0")

# Run build script if executable does not exist
if ! ls ../bin/$EXENAME > /dev/null 2>&1; then
	echo "'$EXENAME' not found. Attempting to build executable..."
	if ! ../scr/build-nix.sh; then
		echo "Setup failed."
		exit 1
	fi
fi

# Copy executable to /usr/local/bin
if cp ../bin/$EXENAME $HOME/bin/; then
	echo "Setup successful."
	echo "You may now call '$EXENAME' from anywhere in your Linux terminal."
	exit 0
else
	echo "Setup failed."
	exit 1
fi
