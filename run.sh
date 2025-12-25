#!/bin/bash
set -e

echo "Cleaning previous build..."
docker compose down --volumes --remove-orphans
rm -rf ./server

echo "Building Docker image..."
docker compose build

echo "Starting container..."
docker compose up -d

echo "Done. Use 'docker compose logs -f' to view logs."
