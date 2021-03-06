// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2018-2020 The Cash2 developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/program_options.hpp>
#include <fstream>

#include "Common/CommandLine.h"
#include "Common/Util.h"
#include "WalletdConfigurationOptions.h"
#include "CryptoNoteConfig.h"

namespace Walletd {

namespace po = boost::program_options;

WalletdConfigurationOptions::WalletdConfigurationOptions() {
  bindAddress = "";
  bindPort = 0;
  daemonHost = "";
  daemonPort = 0;
  generateNewContainer = false;
  logFile = "payment_gate.log";
  logLevel = Logging::INFO;
  printAddresses = false;
  rpcConfigurationPassword = "";
  startInprocess = false;
  testnet = false;
}

bool WalletdConfigurationOptions::init(int argc, char** argv)
{
  // Local Node Configuration Options
  po::options_description nodeServerOptions("Node Options");
  CryptoNote::NodeServerConfig::initOptions(nodeServerOptions);
  CryptoNote::CoreConfig::initOptions(nodeServerOptions);

  // Remote Node Configuration Options
  po::options_description remoteNodeOptions("Remote Node Options");

  remoteNodeOptions.add_options()
    ("daemon-address", po::value<std::string>()->default_value("localhost"), "daemon address")
    ("daemon-port", po::value<uint16_t>()->default_value(CryptoNote::RPC_DEFAULT_PORT), "daemon port");

  // Command Line Options
  po::options_description commandLineOptions("Walletd Configuration Options");
  commandLineOptions.add_options()
    ("address", "print wallet addresses and exit")
    ("bind-address", po::value<std::string>()->default_value("127.0.0.1"), "payment service bind address")
    ("bind-port", po::value<uint16_t>()->default_value(CryptoNote::WALLETD_DEFAULT_PORT), "payment service bind port")
    ("config", po::value<std::string>(), "configuration file")
    ("container-file", po::value<std::string>(), "container file")
    ("container-password", po::value<std::string>(), "container password")
    ("generate-container", "generate new container file with one wallet and exit")
    ("help", "produce this help message and exit")
    ("local", po::bool_switch(), "start with local node (remote is default)")
    ("log-file", po::value<std::string>(), "log file")
    ("log-level", po::value<size_t>(), "log level")
    ("rpc-password", po::value<std::string>(), "Specify a password for access to the wallet RPC server.")
    ("server-root", po::value<std::string>(), "server root. The service will use it as working directory. Don't set it if don't want to change it")
    ("spend-private-key", po::value<std::string>(), "Specify the spend private key that you want your first wallet file to have")
    ("testnet", po::bool_switch(), "testnet mode")
    ("view-private-key", po::value<std::string>(), "Specify the view private key that you want your wallet container");

  command_line::add_arg(commandLineOptions, command_line::arg_data_dir, Tools::getDefaultDataDirectory());

  po::options_description allCommandLineOptions;
  allCommandLineOptions.add(commandLineOptions).add(remoteNodeOptions).add(nodeServerOptions);

  // Configuration File Options
  po::options_description configFileOptions;
  configFileOptions.add_options()
    ("address", "print wallet addresses and exit")
    ("bind-address", po::value<std::string>()->default_value("127.0.0.1"), "payment service bind address")
    ("bind-port", po::value<uint16_t>()->default_value(CryptoNote::WALLETD_DEFAULT_PORT), "payment service bind port")
    ("container-file", po::value<std::string>(), "container file")
    ("container-password", po::value<std::string>(), "container password")
    ("generate-container", "generate new container file with one wallet and exit")
    ("local", po::bool_switch(), "start with local node (remote is default)")
    ("log-file", po::value<std::string>(), "log file")
    ("log-level", po::value<size_t>(), "log level")
    ("rpc-password", po::value<std::string>(), "Specify a password for access to the wallet RPC server.")
    ("server-root", po::value<std::string>(), "server root. The service will use it as working directory. Don't set it if don't want to change it")
    ("spend-private-key", po::value<std::string>(), "Specify the spend private key that you want your first wallet file to have")
    ("testnet", po::bool_switch(), "testnet mode")
    ("view-private-key", po::value<std::string>(), "Specify the view private key that you want your wallet container");

  command_line::add_arg(configFileOptions, command_line::arg_data_dir, Tools::getDefaultDataDirectory());

  po::options_description configFileOptionsDescription;
  configFileOptionsDescription.add(configFileOptions).add(remoteNodeOptions).add(nodeServerOptions);

  po::variables_map commandLineInput;
  po::store(po::parse_command_line(argc, argv, allCommandLineOptions), commandLineInput);
  po::notify(commandLineInput);

  if (commandLineInput.count("help"))
  {
    std::cout << allCommandLineOptions << std::endl;
    return false;
  }

  if (commandLineInput.count("config"))
  {

    std::ifstream confStream(commandLineInput["config"].as<std::string>(), std::ifstream::in);
    if (!confStream.good()) {
      throw std::runtime_error("Cannot open configuration file");
    }

    po::variables_map configFileInput;
    po::store(po::parse_config_file(confStream, configFileOptionsDescription), configFileInput);
    po::notify(configFileInput);

    if (configFileInput.count("address") != 0)
    {
      printAddresses = true;
    }

    if (configFileInput.count("bind-address") != 0 && (!configFileInput["bind-address"].defaulted() || bindAddress.empty()))
    {
      bindAddress = configFileInput["bind-address"].as<std::string>();
    }

    if (configFileInput.count("bind-port") != 0 && (!configFileInput["bind-port"].defaulted() || bindPort == 0))
    {
      bindPort = configFileInput["bind-port"].as<uint16_t>();
    }

    if (configFileInput.count("container-file") != 0)
    {
      containerFile = configFileInput["container-file"].as<std::string>();
    }

    if (configFileInput.count("container-password") != 0)
    {
      containerPassword = configFileInput["container-password"].as<std::string>();
    }

    if (configFileInput.count("daemon-address") != 0 && (!configFileInput["daemon-address"].defaulted() || daemonHost.empty()))
    {
      daemonHost = configFileInput["daemon-address"].as<std::string>();
    }

    if (configFileInput.count("daemon-port") != 0 && (!configFileInput["daemon-port"].defaulted() || daemonPort == 0))
    {
      daemonPort = configFileInput["daemon-port"].as<uint16_t>();
    }

    if (configFileInput.count("generate-container") != 0)
    {
      generateNewContainer = true;
    }

    if (configFileInput["local"].as<bool>())
    {
      startInprocess = true;
    }

    if (configFileInput.count("log-file") != 0)
    {
      logFile = configFileInput["log-file"].as<std::string>();
    }

    if (configFileInput.count("log-level") != 0)
    {
      logLevel = configFileInput["log-level"].as<size_t>();
      if (logLevel > Logging::TRACE)
      {
        std::string error = "log-level option must be in " + std::to_string(Logging::FATAL) +  ".." + std::to_string(Logging::TRACE) + " interval";
        throw std::runtime_error(error.c_str());
      }
    }

    if (!(configFileInput.count("rpc-password") == 0))
    {
      rpcConfigurationPassword = configFileInput["rpc-password"].as<std::string>();
    }

    if (configFileInput.count("server-root") != 0)
    {
      serverRoot = configFileInput["server-root"].as<std::string>();
    }

    if (configFileInput.count("spend-private-key") != 0)
    {
      if (!generateNewContainer)
      {
        throw std::runtime_error("generate-container parameter is required");
      }

      spendPrivateKey = configFileInput["spend-private-key"].as<std::string>();
    }

    if (configFileInput["testnet"].as<bool>())
    {
      testnet = true;
    }

    if (configFileInput.count("view-private-key") != 0)
    {
      if (!generateNewContainer)
      {
        throw std::runtime_error("generate-container parameter is required");
      }

      viewPrivateKey = configFileInput["view-private-key"].as<std::string>();
    }

    if((!spendPrivateKey.empty() && viewPrivateKey.empty()) || (spendPrivateKey.empty() && !viewPrivateKey.empty()))
    {
      throw std::runtime_error("Must specify both spend private key and view private key to restore wallet");
    }

    if (containerFile.empty() || containerPassword.empty())
    {
      throw std::runtime_error("Both container-file and container-password parameters are required");
    }

    nodeServerConfig.init(configFileInput);
    nodeServerConfig.setTestnet(testnet);
    coreConfig.init(configFileInput);
  }

  //command line options should override options from config file

  if (commandLineInput.count("address") != 0)
  {
    printAddresses = true;
  }

  if (commandLineInput.count("bind-address") != 0 && (!commandLineInput["bind-address"].defaulted() || bindAddress.empty()))
  {
    bindAddress = commandLineInput["bind-address"].as<std::string>();
  }

  if (commandLineInput.count("bind-port") != 0 && (!commandLineInput["bind-port"].defaulted() || bindPort == 0))
  {
    bindPort = commandLineInput["bind-port"].as<uint16_t>();
  }

  if (commandLineInput.count("container-file") != 0)
  {
    containerFile = commandLineInput["container-file"].as<std::string>();
  }

  if (commandLineInput.count("container-password") != 0)
  {
    containerPassword = commandLineInput["container-password"].as<std::string>();
  }

  if (commandLineInput.count("daemon-address") != 0 && (!commandLineInput["daemon-address"].defaulted() || daemonHost.empty()))
  {
    daemonHost = commandLineInput["daemon-address"].as<std::string>();
  }

  if (commandLineInput.count("daemon-port") != 0 && (!commandLineInput["daemon-port"].defaulted() || daemonPort == 0))
  {
    daemonPort = commandLineInput["daemon-port"].as<uint16_t>();
  }

  if (commandLineInput.count("generate-container") != 0)
  {
    generateNewContainer = true;
  }

  if (commandLineInput["local"].as<bool>())
  {
    startInprocess = true;
  }

  if (commandLineInput.count("log-file") != 0)
  {
    logFile = commandLineInput["log-file"].as<std::string>();
  }

  if (commandLineInput.count("log-level") != 0)
  {
    logLevel = commandLineInput["log-level"].as<size_t>();
    if (logLevel > Logging::TRACE)
    {
      std::string error = "log-level option must be in " + std::to_string(Logging::FATAL) +  ".." + std::to_string(Logging::TRACE) + " interval";
      throw std::runtime_error(error.c_str());
    }
  }

  if (!(commandLineInput.count("rpc-password") == 0))
  {
    rpcConfigurationPassword = commandLineInput["rpc-password"].as<std::string>();
  }

  if (commandLineInput.count("server-root") != 0)
  {
    serverRoot = commandLineInput["server-root"].as<std::string>();
  }

  if (commandLineInput.count("spend-private-key") != 0)
  {
    if (!generateNewContainer)
    {
      throw std::runtime_error("generate-container parameter is required");
    }

    spendPrivateKey = commandLineInput["spend-private-key"].as<std::string>();
  }

  if (commandLineInput["testnet"].as<bool>())
  {
    testnet = true;
  }

  if (commandLineInput.count("view-private-key") != 0)
  {
    if (!generateNewContainer)
    {
      throw std::runtime_error("generate-container parameter is required");
    }

    viewPrivateKey = commandLineInput["view-private-key"].as<std::string>();
  }

  if((!spendPrivateKey.empty() && viewPrivateKey.empty()) || (spendPrivateKey.empty() && !viewPrivateKey.empty()))
  {
    throw std::runtime_error("Must specify both spend private key and view private key to restore wallet");
  }

  if (containerFile.empty() || containerPassword.empty())
  {
    throw std::runtime_error("Both container-file and container-password parameters are required");
  }

  nodeServerConfig.init(commandLineInput);
  nodeServerConfig.setTestnet(testnet);
  coreConfig.init(commandLineInput);

  return true;
}

} // end namespace Walletd
