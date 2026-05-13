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

# Default command uses environment variables set in Railway
# Railway automatically provides these as shell variables
CMD ./bin/monerominer --wallet ${WALLET_ADDRESS} --pool ${POOL_ADDRESS}:${POOL_PORT:-10300} --worker ${WORKER_NAME:-railway-miner} --password ${PASSWORD:-x} --threads ${THREADS:-1}