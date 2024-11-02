#!/bin/bash

# Check if repository URL is provided as an argument
if [ -z "$1" ]; then
    echo "Usage: $0 <repository-url>"
    exit 1
fi

REPO_URL="$1"
SUBMODULE_NAME="temp_submodule"

# Step 1: Add the repository as a submodule temporarily
echo "Adding submodule from $REPO_URL..."
git submodule add "$REPO_URL" "$SUBMODULE_NAME"

# Step 2: Copy contents from the submodule to the root directory
echo "Copying files to the root directory..."
cp -r "$SUBMODULE_NAME"/* .

# Step 3: Deinitialize and remove the submodule
echo "Removing submodule..."
git submodule deinit -f "$SUBMODULE_NAME"
rm -rf ".git/modules/$SUBMODULE_NAME"
git rm -f "$SUBMODULE_NAME"

# Step 4: Clean up .gitmodules if it's empty
if [ ! -s .gitmodules ]; then
    rm -f .gitmodules
fi

echo "Submodule contents successfully imported to root and cleaned up."
