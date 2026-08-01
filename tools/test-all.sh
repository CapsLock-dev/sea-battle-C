#!/usr/bin/env sh
set -e
cmake --workflow --preset test-hashtable
cmake --workflow --preset test-tree
cmake --workflow --preset test-release-hashtable
cmake --workflow --preset test-release-tree
