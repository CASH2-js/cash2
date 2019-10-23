// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2018-2019 The Cash2 developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <iostream>
#include <memory>
#include <thread>

#include <string.h>

#include "Logging/LoggerManager.h"
#include "PaymentGateService.h"
#include "version.h"

#ifdef WIN32
#include <windows.h>
#include <winsvc.h>
#else
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#endif

#define SERVICE_NAME "Payment Gate"

PaymentGateService* paymentGateServicePtr;

#ifdef WIN32

SERVICE_STATUS_HANDLE serviceStatusHandle;

std::string GetLastErrorMessage(DWORD errorMessageID) {
  LPSTR messageBuffer = nullptr;
  size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, errorMessageID, 0, (LPSTR)&messageBuffer, 0, NULL);

  std::string message(messageBuffer, size);

  LocalFree(messageBuffer);

  return message;
}

void __stdcall serviceHandler(DWORD fdwControl) {
  if (fdwControl == SERVICE_CONTROL_STOP) {
    Logging::LoggerRef log(paymentGateServicePtr->getLogger(), "serviceHandler");
    log(Logging::INFO, Logging::BRIGHT_YELLOW) << "Stop signal caught";

    SERVICE_STATUS serviceStatus{ SERVICE_WIN32_OWN_PROCESS, SERVICE_STOP_PENDING, 0, NO_ERROR, 0, 0, 0 };
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    paymentGateServicePtr->stop();
  }
}

void __stdcall serviceMain(DWORD dwArgc, char **lpszArgv) {
  Logging::LoggerRef logRef(paymentGateServicePtr->getLogger(), "WindowsService");

  serviceStatusHandle = RegisterServiceCtrlHandler("PaymentGate", serviceHandler);
  if (serviceStatusHandle == NULL) {
    logRef(Logging::FATAL, Logging::BRIGHT_RED) << "Couldn't make RegisterServiceCtrlHandler call: " << GetLastErrorMessage(GetLastError());
    return;
  }

  SERVICE_STATUS serviceStatus{ SERVICE_WIN32_OWN_PROCESS, SERVICE_START_PENDING, 0, NO_ERROR, 0, 1, 3000 };
  if (SetServiceStatus(serviceStatusHandle, &serviceStatus) != TRUE) {
    logRef(Logging::FATAL, Logging::BRIGHT_RED) << "Couldn't make SetServiceStatus call: " << GetLastErrorMessage(GetLastError());
    return;
  }

  serviceStatus = { SERVICE_WIN32_OWN_PROCESS, SERVICE_RUNNING, SERVICE_ACCEPT_STOP, NO_ERROR, 0, 0, 0 };
  if (SetServiceStatus(serviceStatusHandle, &serviceStatus) != TRUE) {
    logRef(Logging::FATAL, Logging::BRIGHT_RED) << "Couldn't make SetServiceStatus call: " << GetLastErrorMessage(GetLastError());
    return;
  }

  try {
    paymentGateServicePtr->run();
  } catch (std::exception& ex) {
    logRef(Logging::FATAL, Logging::BRIGHT_RED) << "Error occurred: " << ex.what();
  }

  serviceStatus = { SERVICE_WIN32_OWN_PROCESS, SERVICE_STOPPED, 0, NO_ERROR, 0, 0, 0 };
  SetServiceStatus(serviceStatusHandle, &serviceStatus);
}

#else

int daemonize() {
  pid_t pid;
  pid = fork();

  if (pid < 0)
    return pid;

  if (pid > 0)
    return pid;

  if (setsid() < 0)
    return -1;

  signal(SIGCHLD, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);

  pid = fork();

  if (pid < 0)
    return pid;

  if (pid > 0)
    return pid;

  umask(0);

  return 0;
}

#endif

int runDaemon() {
#ifdef WIN32

  SERVICE_TABLE_ENTRY serviceTable[] {
    { "Payment Gate", serviceMain },
    { NULL, NULL }
  };

  Logging::LoggerRef logRef(paymentGateServicePtr->getLogger(), "RunService");

  if (StartServiceCtrlDispatcher(serviceTable) != TRUE) {
    logRef(Logging::FATAL, Logging::BRIGHT_RED) << "Couldn't start service: " << GetLastErrorMessage(GetLastError());
    return 1;
  }

  logRef(Logging::INFO) << "Service stopped";
  return 0;

#else

  int daemonResult = daemonize();
  if (daemonResult > 0) {
    //parent
    return 0;
  } else if (daemonResult < 0) {
    //error occurred
    return 1;
  }

  paymentGateServicePtr->run();

  return 0;

#endif
}

int main(int argc, char** argv) {

  PaymentGateService paymentGateService; 
  paymentGateServicePtr = &paymentGateService;

  try
  {

    if (!paymentGateService.init(argc, argv)) {
      return 0;
    }

    Logging::LoggerRef logger(paymentGateService.getLogger(), "main");
    logger(Logging::INFO) << "PaymentService " << " v" << PROJECT_VERSION_LONG;

    const PaymentService::WalletdConfigurationOptions& config = paymentGateService.getConfig();

    if (config.generateNewContainer) {
      System::Dispatcher d;
      generateNewWallet(paymentGateService.getCurrency(), paymentGateService.getWalletConfig(), paymentGateService.getLogger(), d);
      return 0;
    }

    if (config.daemonize) {
      if (runDaemon() != 0) {
        throw std::runtime_error("Failed to start daemon");
      }
    } else {
      paymentGateService.run();
    }

  }
  catch (std::exception& ex)
  {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
  }

  return 0;
}
