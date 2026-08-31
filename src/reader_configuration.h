#ifndef _READER_CONFIGURATION_H_
#define _READER_CONFIGURATION_H_

bool readerConfigurationIsActive() {
  return readerConfigurationStep != ReaderConfigurationStep::Idle &&
         readerConfigurationStep != ReaderConfigurationStep::Completed &&
         readerConfigurationStep != ReaderConfigurationStep::Error;
}

void failReaderConfiguration(const char* reason) {
  readerConfigurationStep = ReaderConfigurationStep::Error;
  Serial.printf("[ERROR][RFID] Konfiguracja czytnika przerwana: %s\n", reason);
}

bool startReaderConfigurationStep(ReaderConfigurationStep step) {
  readerConfigurationStep = step;
  bool started = false;
  readerConfigurationDispatching = true;

  switch (step) {
    case ReaderConfigurationStep::ReaderInfo: started = requestReaderInfo(); break;
    case ReaderConfigurationStep::WorkMode: started = requestWorkMode(); break;
    case ReaderConfigurationStep::ReadRegion:
    case ReaderConfigurationStep::VerifyRegion: started = requestReadRegion(); break;
    case ReaderConfigurationStep::SetRegion: started = requestSetRegion(); break;
    case ReaderConfigurationStep::ReadPower:
    case ReaderConfigurationStep::VerifyPower: started = requestReadAntennaPower(); break;
    case ReaderConfigurationStep::SetPower: started = requestSetRfPower(); break;
    case ReaderConfigurationStep::EnableAntennaCheck: started = requestEnableAntennaCheck(); break;
    case ReaderConfigurationStep::DetectAntennas: started = startAntennaDetection(); break;
    case ReaderConfigurationStep::ConfigureAntennas: started = requestConfigureAntennas(); break;
    case ReaderConfigurationStep::DisableTagFocus: started = requestDisableTagFocus(); break;
    case ReaderConfigurationStep::SetQAndSession: started = requestSetQAndSession(); break;
    case ReaderConfigurationStep::SetEpcMode: started = requestSetEpcMode(); break;
    case ReaderConfigurationStep::ClearMask: started = requestClearInventoryMask(); break;
    case ReaderConfigurationStep::Completed:
      readerConfigurationDispatching = false;
      Serial.println("[RFID] Konfiguracja zakończona; oczekiwanie na żądanie MQTT");
      return true;
    default:
      readerConfigurationDispatching = false;
      return false;
  }

  readerConfigurationDispatching = false;
  if (!started) failReaderConfiguration("nie można rozpocząć kolejnego kroku");
  return started;
}

void continueReaderConfiguration(ReaderConfigurationStep expected,
                                 ReaderConfigurationStep next) {
  if (readerConfigurationStep == expected) startReaderConfigurationStep(next);
}

bool startReaderConfiguration() {
  if (readerConfigurationIsActive() || pendingRequest != PendingRequest::None ||
      antennaDetectionActive || fastInventoryState != FastInventoryState::Idle) {
    Serial.println("[ERROR][RFID] Czytnik jest zajęty");
    return false;
  }

  regionSetAttempts = 0;
  powerSetAttempts = 0;
  Serial.println("[RFID] Rozpoczynam konfigurację czytnika");
  return startReaderConfigurationStep(ReaderConfigurationStep::ReaderInfo);
}

void handleRegionConfigurationResult(ConfigurationCheck result) {
  const bool initialRead = readerConfigurationStep == ReaderConfigurationStep::ReadRegion;
  const bool verification = readerConfigurationStep == ReaderConfigurationStep::VerifyRegion;
  if (!initialRead && !verification) return;

  if (result == ConfigurationCheck::Matches) {
    startReaderConfigurationStep(ReaderConfigurationStep::ReadPower);
  } else if (result == ConfigurationCheck::NeedsUpdate && initialRead &&
             regionSetAttempts++ < MAX_CONFIG_SET_ATTEMPTS) {
    startReaderConfigurationStep(ReaderConfigurationStep::SetRegion);
  } else {
    failReaderConfiguration(result == ConfigurationCheck::Error
      ? "nie można odczytać regionu" : "region nadal jest niezgodny");
  }
}

void handlePowerConfigurationResult(ConfigurationCheck result) {
  const bool initialRead = readerConfigurationStep == ReaderConfigurationStep::ReadPower;
  const bool verification = readerConfigurationStep == ReaderConfigurationStep::VerifyPower;
  if (!initialRead && !verification) return;

  if (result == ConfigurationCheck::Matches) {
    startReaderConfigurationStep(ReaderConfigurationStep::EnableAntennaCheck);
  } else if (result == ConfigurationCheck::NeedsUpdate && initialRead &&
             powerSetAttempts++ < MAX_CONFIG_SET_ATTEMPTS) {
    startReaderConfigurationStep(ReaderConfigurationStep::SetPower);
  } else {
    failReaderConfiguration(result == ConfigurationCheck::Error
      ? "nie można odczytać mocy anten" : "moc anten nadal jest niezgodna");
  }
}

#endif
