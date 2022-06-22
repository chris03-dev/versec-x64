#!/bin/sh

EXENAME="versec"
CFILES="*.c"
CFLAGSD="-lm"
CFLAGS0="-Wall -s -O2 -std=c99"
CFLAGS1="-ffunction-sections -fdata-sections -fstack-protector-all"
CFLAGS2="-Wl,--gc-sections"

echo "Running *nix build script for '$EXENAME'..."
cd "$(dirname "$0")"
mkdir -p ../bin
mkdir -p ../test
cd ../src

# Determine which C compiler to use
if [ $# -eq 1 ]; then
	
	# Check if selected compiler is installed
	if command -v "$1" > /dev/null 2>&1; then COMPILER=$1
	else
		echo "'$1' is not installed on your computer."
		exit 1
	fi
	
elif command -v "clang" > /dev/null 2>&1; then COMPILER="clang"
elif command -v "gcc"   > /dev/null 2>&1; then COMPILER="gcc"
else
	echo "Neither 'clang' nor 'gcc' is installed on your computer."
	echo "Install either compiler specified to proceed with the build process."
	exit 1
fi

# Inform user which C compiler has been selected for building executable
echo "Selected '$(which "$COMPILER")' as C compiler."

# Compile using selected C compiler
if $COMPILER $CFLAGS0 -o ../bin/$EXENAME $CFILES $CFLAGSD; then
	
	# Attempt to compile with additional flags
	if $COMPILER $CFLAGS0 $CFLAGS1 $CFILES -o ../bin/$EXENAME $CFLAGSD > /dev/null 2>&1; then

		# Attempt to compile with more additional flags
		if $COMPILER $CFLAGS0 $CFLAGS1 $CFLAGS2 $CFILES -o ../bin/$EXENAME $CFLAGSD > /dev/null 2>&1; then
			echo "Build successful!"
			exit 0
		else
			echo "Build successful with the following flags omitted: $CFLAGS2"
			exit 0
		fi
		
	else
		echo "Build successful with the following flags omitted: $CFLAGS1 $CFLAGS2"
		exit 0
	fi
	
else
	echo "Build failure."
	exit 1
fi
