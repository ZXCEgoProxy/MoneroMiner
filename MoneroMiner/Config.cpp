#include "Config.h"
#include "Globals.h"
#include "Platform.h"  // Replace windows.h with Platform.h
#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include "Utils.h"

Config::Config() {
    setDefaults();
}

void Config::setDefaults() {
    // Get values from environment variables if available, otherwise use defaults
    const char* envPoolAddr = getenv("POOL_ADDRESS");
    poolAddress = envPoolAddr ? envPoolAddr : "xmr-us-east1.nanopool.org";

    const char* envPoolPort = getenv("POOL_PORT");
    if (envPoolPort) {
        try {
            poolPort = std::stoi(envPoolPort);
        } catch (...) {
            poolPort = 10300;
        }
    } else {
        poolPort = 10300;
    }

    const char* envWallet = getenv("WALLET_ADDRESS");
    walletAddress = envWallet ? envWallet : "8C6hFb4Buo6dYwJiZEaFhyYhZTJaR4NyXSBzKMF1BnNKMGD92yeaY3a9PxuWp9bhTAh6dAXwqyyLfFxaPRct7j81L8t4iK2"; // Default test wallet

    const char* envWorker = getenv("WORKER_NAME");
    workerName = envWorker ? envWorker : "worker1";

    const char* envPassword = getenv("PASSWORD");
    password = envPassword ? envPassword : "x";

    const char* envThreads = getenv("THREADS");
    if (envThreads) {
        try {
            numThreads = std::stoi(envThreads);
        } catch (...) {
            numThreads = 1;
        }
    } else {
        numThreads = 1; // Optimized for Railway 2vCPU
    }

    const char* envLightMode = getenv("LIGHT_MODE");
    useLightMode = envLightMode && (std::string(envLightMode) == "true" || std::string(envLightMode) == "1");

    userAgent = "MoneroMiner/1.0.0";
    debugMode = false;  // This should be overridden by --debug flag
    useLogFile = false;
    logFileName = "monerominer.log";
    headlessMode = false; // Initialize headless mode flag
}

bool Config::parseCommandLine(int argc, char* argv[]) {
    bool threadCountSpecified = false; // Track if user specified threads

    // Parse all arguments first
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage();
            return false;  // Let the main program handle help
        }
        else if (arg == "--debug") {
            debugMode = true;
        }
        else if (arg == "--logfile") {
            useLogFile = true;
        }
        else if (arg == "--threads" && i + 1 < argc) {
            std::string threadsStr = argv[++i];
            try {
                numThreads = std::stoi(threadsStr);
                threadCountSpecified = true; // Track if user specified threads
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error: Invalid threads value '" << threadsStr << "' - using default" << std::endl;
                numThreads = 1;
            } catch (const std::out_of_range& e) {
                std::cerr << "Error: Threads value out of range '" << threadsStr << "' - using default" << std::endl;
                numThreads = 1;
            }
        }
        else if (arg == "--pool" && i + 1 < argc) {
            std::string poolStr = argv[++i];
            if (poolStr.empty()) {
                std::cerr << "Error: Pool address cannot be empty - using default" << std::endl;
                poolAddress = "xmr-us-east1.nanopool.org";
                poolPort = 10300;
            } else {
                size_t colonPos = poolStr.find(':');
                if (colonPos != std::string::npos) {
                    poolAddress = poolStr.substr(0, colonPos);
                    std::string portStr = poolStr.substr(colonPos + 1);
                    try {
                        poolPort = std::stoi(portStr);
                    } catch (const std::invalid_argument& e) {
                        std::cerr << "Error: Invalid pool port '" << portStr << "' - using default 10300" << std::endl;
                        poolPort = 10300;
                    } catch (const std::out_of_range& e) {
                        std::cerr << "Error: Pool port out of range '" << portStr << "' - using default 10300" << std::endl;
                        poolPort = 10300;
                    }
                } else {
                    poolAddress = poolStr;
                }
            }
        }
        else if (arg == "--wallet" && i + 1 < argc) {
            walletAddress = argv[++i];
            if (walletAddress.empty()) {
                std::cerr << "Error: Wallet address cannot be empty" << std::endl;
                return false;
            }
        }
        else if (arg == "--worker" && i + 1 < argc) {
            workerName = argv[++i];
        }
        else if (arg == "--password" && i + 1 < argc) {
            password = argv[++i];
        }
        else if (arg == "--headless") {
            headlessMode = true;
            useLogFile = true; // Force log file in headless mode
        }
        else if (arg == "--light-mode") {
            useLightMode = true;
        }
    }
    
    // ONLY auto-detect if user did NOT specify --threads
    if (!threadCountSpecified && numThreads <= 1) {
        unsigned int logicalProcessors = Platform::getLogicalProcessors();

        // Use the majority of logical processors but leave one core for the system.
        // This yields: 3 threads on 4-core, 7 on 8-core, 15 on 16-core, 23 on 24-core, etc.
        unsigned int recommended = (logicalProcessors > 1) ? (logicalProcessors - 1) : 1;
        numThreads = static_cast<int>(recommended);
        std::cout << "Auto-detected " << logicalProcessors
                  << " logical processors, using " << numThreads
                  << " mining threads (leaving 1 thread for system)" << std::endl;
    }
    
    // Set default worker name based on machine name if not specified
    if (workerName.empty() || workerName == "worker1") {
        std::string computerName = Platform::getComputerName();
        workerName = computerName;
        
        // Sanitize: lowercase and remove invalid chars
        for (char& c : workerName) {
            if (!std::isalnum(c)) c = '_';
            else c = std::tolower(c);
        }
    }
    
    return true;
}

bool validateConfig(const Config& config) {
    if (config.walletAddress.empty()) {
        std::cerr << "Error: Wallet address is required" << std::endl;
        return false;
    }

    if (config.numThreads <= 0) {
        std::cerr << "Error: Invalid thread count" << std::endl;
        return false;
    }

    if (config.poolPort <= 0) {
        std::cerr << "Error: Invalid pool port" << std::endl;
        return false;
    }

    return true;
}

void Config::printConfig() const {
    std::cout << "Current configuration:" << std::endl;
    std::cout << "Pool address: " << poolAddress << ":" << poolPort << std::endl;
    std::cout << "Wallet: " << walletAddress << std::endl;
    std::cout << "Worker name: " << workerName << std::endl;
    std::cout << "User agent: " << userAgent << std::endl;
    std::cout << "Number of threads: " << numThreads << std::endl;
    std::cout << "Debug mode: " << (debugMode ? "enabled" : "disabled") << std::endl;
    std::cout << "Log file: " << logFileName << std::endl;
    std::cout << std::endl;
}

void Config::printUsage() const {
    std::cout << "MoneroMiner - Monero CPU Miner" << std::endl;
    std::cout << "\nUsage: MoneroMiner [options]" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  --help                 Show this help message" << std::endl;
    std::cout << "  --debug                Enable debug output" << std::endl;
    std::cout << "  --logfile              Enable logging to file" << std::endl;
    std::cout << "  --threads N            Number of mining threads" << std::endl;
    std::cout << "  --pool ADDRESS:PORT    Pool address and port" << std::endl;
    std::cout << "  --wallet ADDRESS       Your Monero wallet address" << std::endl;
    std::cout << "  --worker NAME          Worker name" << std::endl;
    std::cout << "  --password PASS        Pool password (default: x)" << std::endl;
    std::cout << "  --headless             Enable headless mode (no GUI)" << std::endl;
    std::cout << "  --light-mode           Use RandomX light mode (256MB, for testing/low RAM)" << std::endl;
    std::cout << "\nExample:" << std::endl;
    std::cout << "  MoneroMiner.exe --wallet YOUR_WALLET --threads 4" << std::endl;
}