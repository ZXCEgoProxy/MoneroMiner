# MoneroMiner on Railway

This guide explains how to deploy MoneroMiner to Railway.

## Prerequisites

- Railway account
- Monero wallet address
- GitHub Personal Access Token (if repository is private)

## Deployment Steps

1. Fork or clone this repository
2. Connect your GitHub repository to Railway
3. Set environment variables in Railway dashboard:
   - `WALLET_ADDRESS`: Your Monero wallet address
   - `POOL_ADDRESS`: Mining pool address (default: xmr-us-east1.nanopool.org)
   - `POOL_PORT`: Pool port (default: 10300)
   - `WORKER_NAME`: Worker identifier (optional)
   - `PASSWORD`: Pool password (default: x)
   - `THREADS`: Number of threads (fixed to 1)
   - `THREADS`: Number of mining threads (default: 1)

4. Deploy the application

## Configuration

The miner will automatically:
- Build RandomX and MoneroMiner
- Use RandomX light mode (256MB) optimized for 1GB RAM
- Use 1 mining thread optimized for 2 vCPU
- Connect to the specified pool
- Start mining with your wallet address

**Note:** Light mode uses only 256MB cache and produces invalid shares for Monero mining. This configuration is optimized for Railway's resource constraints but will not earn mining rewards. For production mining, upgrade to a plan with more RAM.

## Monitoring

Check Railway logs to monitor mining progress, hashrate, and accepted shares.

## Security Notes

### Sensitive Data Required:
- **WALLET_ADDRESS**: Your Monero wallet address (keep private for mining rewards)
- **POOL_PASSWORD**: Pool password (usually 'x', not sensitive)
- **GITHUB_TOKEN**: GitHub Personal Access Token (if deploying private repository)

### Security Best Practices:
- Never commit wallet addresses or sensitive credentials to the repository
- Use Railway's environment variable system for sensitive data
- The default wallet in Config.cpp is for donations only
- Rotate credentials regularly
- Monitor mining activity for unauthorized access