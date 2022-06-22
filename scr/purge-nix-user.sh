#!/bin/sh

EXENAME="verse"

echo "Running *nix purge script for '$EXENAME'..."
cd $(dirname "$0")

# Delete executable in $HOME/bin
if rm $HOME/bin/$EXENAME; then
	echo "Deletion in '$HOME/bin/' successful."
	exit 0
else
	echo "Deletion failed."
	exit 1
fi
