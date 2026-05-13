# Dockerfile for MoneroMiner deployment on Railway
# Uses Ubuntu base image for compatibility with build tools

FROM ubuntu:20.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies required for RandomX and MoneroMiner
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory for the application
WORKDIR /app

# Copy all project files to the container
COPY . .

# Build RandomX library and MoneroMiner executable
RUN make

# Ensure the executable has proper permissions
RUN chmod +x bin/monerominer

# No ports need to be exposed for mining (outbound connections only)

# Default command - environment variables are read in code
# Use light mode for low RAM (1GB)
CMD ["./bin/monerominer", "--light-mode"]