/**
 *  \file reader.h
 *  Konfiguracja i diagnostyka czytnika RFID oraz pojedyncza inwentaryzacja.
 *  Komunikacja z czytnikiem odbywa się przez UART, a lista EPC trafia do MQTT.
 */
#ifndef _READER_H_
#define _READER_H_



/** Port UART używany do komunikacji z czytnikiem RFID. */
HardwareSerial RfidSerial(1);

/** @name Parametry sumy kontrolnej CRC16 */
///@{
/** Wartość początkowa rejestru CRC16. */
#define PRESET_VALUE 0xFFFF
/** Wielomian CRC16 w postaci używanej przy przesuwaniu bitów w prawo. */
#define POLYNOMIAL 0x8408
///@}

/**
 *  Obliczenie sumy CRC16 dla wskazanego ciągu bajtów.
 *  \param pucY Wskaźnik na dane, bez końcowych bajtów CRC.
 *  \param ucX Liczba bajtów uwzględnianych w obliczeniu.
 *  \return Obliczona 16-bitowa suma kontrolna.
 */
unsigned int uiCrc16(unsigned char const *pucY, unsigned char ucX) {
  unsigned char ucI, ucJ;
  unsigned short int uiCrcValue = PRESET_VALUE;
  for (ucI = 0; ucI < ucX; ucI++) {
    uiCrcValue = uiCrcValue ^ *(pucY + ucI);
    for (ucJ = 0; ucJ < 8; ucJ++) {
      if (uiCrcValue & 0x0001) {
        uiCrcValue = (uiCrcValue >> 1)^POLYNOMIAL; // not really a polynomial, just bitwise XOR (^)
      }
      else {
        uiCrcValue = (uiCrcValue >> 1);
      }
    }
  }
  return uiCrcValue;
}
////////////////////////////////////////////////////////
/** @name Stałe komunikacji, konfiguracji i inwentaryzacji
 *  Czasy podane są w milisekundach, jeśli opis nie wskazuje innej jednostki.
 */
///@{
/** Adres rozgłoszeniowy czytnika; domyślny adres urządzenia to 0x00. */
constexpr uint8_t ADDRESS = 0xFF;
/** Maksymalny czas oczekiwania na odpowiedź komendy. */
constexpr uint32_t COMM_TIMEOUT_MS = 1000;
/** Maksymalny odstęp między bajtami jednej ramki. */
constexpr uint8_t INTER_BYTE_TIMEOUT_MS = 15;
/** Prędkość transmisji UART w bitach na sekundę. */
constexpr uint32_t RFID_BAUD_RATE = 115200; // 115200;
/** Rozmiar bufora odbiorczego sterownika UART w bajtach. */
constexpr size_t RFID_RX_BUFFER_SIZE = 4096;
/** Liczba obsługiwanych portów antenowych. */
constexpr uint8_t ANTENNA_PORT_COUNT = 16;
/** Czas pojedynczego skanowania, liczony od potwierdzenia START. */
constexpr uint32_t FAST_INVENTORY_TIME_MS = 5000;
/** Wymagany czas ciszy przed zakończeniem odbioru danych po STOP. */
constexpr uint32_t FAST_INVENTORY_DRAIN_QUIET_MS = 50;
/** Maksymalny czas odbierania końcowych danych po potwierdzeniu STOP. */
constexpr uint32_t FAST_INVENTORY_DRAIN_MAX_MS = 500;
/** Limit ramek przetwarzanych podczas jednego wywołania handleReaderRequest(). */
constexpr uint8_t MAX_RX_FRAMES_PER_LOOP = 32;
/** Flaga docelowa tagów podczas inwentaryzacji: Target A. */
constexpr uint8_t FAST_INVENTORY_TARGET = 0x00; // Target A
/** Maksymalna liczba unikalnych EPC przechowywanych w wyniku. */
constexpr size_t MAX_FAST_INVENTORY_TAGS = 160;
/** Maksymalna długość identyfikatora EPC w bajtach. */
constexpr size_t MAX_FAST_EPC_LENGTH = 0x3F;
/** Docelowa moc wyjściowa portów antenowych w dBm. */
constexpr uint8_t RFID_POWER_DBM = 33;
/** Limit prób ustawienia niezgodnego regionu lub mocy. */
constexpr uint8_t MAX_CONFIG_SET_ATTEMPTS = 1;
/** Parametr Q krótkiego odczytu sprawdzającego połączenie anteny. */
constexpr uint8_t ANTENNA_PROBE_Q_VALUE = 0x00;
/** Sesja używana do sprawdzania połączenia anteny. */
constexpr uint8_t ANTENNA_PROBE_SESSION = 0x00;
/** Flaga docelowa tagów podczas sprawdzania połączenia anteny. */
constexpr uint8_t ANTENNA_PROBE_TARGET = 0x00;
/** Czas sprawdzania anteny w jednostkach po 100 ms. */
constexpr uint8_t ANTENNA_PROBE_SCAN_TIME = 0x02; // 2 * 100 ms
/** Bity komendy wybierającej pojedynczy port antenowy. */
constexpr uint8_t ANTENNA_SELECTOR_BASE = 0x80;
/** Bezwzględny limit czasu sprawdzania jednego portu antenowego. */
constexpr uint32_t ANTENNA_PROBE_MAX_DURATION_MS = 2000;
/** Przerwa na odbiór opóźnionych danych po przekroczeniu czasu sprawdzania anteny. */
constexpr uint32_t ANTENNA_PROBE_TIMEOUT_RECOVERY_MS = 400;

/** Parametr Q określający liczbę szczelin rundy inwentaryzacji jako 2 do potęgi Q. */
constexpr uint8_t Q_VALUE = 0x06; // 2^Q tags in the range of the antena <0 - 15>
/** Sesja inwentaryzacji: 0-3 oznacza S0-S3, a 0xFF wybór automatyczny. */
constexpr uint8_t SESSION = 0x00; // S0, S1, S2, S3, FF is auto
/** Wartość ustawienia TagFocus; 0x00 wyłącza tę funkcję. */
constexpr uint8_t TAG_FOCUS = 0x00;

static_assert(
  ANTENNA_PORT_COUNT > 0 && ANTENNA_PORT_COUNT <= 16,
  "Antenna masks support from 1 to 16 ports");

/** Maska wszystkich obsługiwanych portów; bit 0 odpowiada antenie 1. */
constexpr uint16_t ALL_ANTENNA_PORTS_MASK =
  static_cast<uint16_t>(
    (uint32_t{1} << ANTENNA_PORT_COUNT) - 1U);

/** Kod regionu EU3 dla kanałów 865,7; 866,3; 866,9 i 867,5 MHz. */
constexpr uint8_t RFID_REGION_EU3 = 0x09;
/** Pierwszy kanał używanego zakresu częstotliwości. */
constexpr uint8_t RFID_REGION_MIN_CHANNEL = 0x00;
/** Ostatni kanał używanego zakresu częstotliwości. */
constexpr uint8_t RFID_REGION_MAX_CHANNEL = 0x03;
///@}

/** @name Komendy czytnika i identyfikatory parametrów konfiguracji */
///@{
/** Odczyt wersji firmware i modelu czytnika. */
constexpr uint8_t CMD_GET_READER_INFO = 0x21;
/** Odczyt temperatury czytnika. */
constexpr uint8_t CMD_GET_READER_TEMPERATURE = 0x92;
/** Odczyt trybu pracy czytnika. */
constexpr uint8_t CMD_GET_WORK_MODE = 0x77;
/** Odczyt regionu i zakresu kanałów. */
constexpr uint8_t CMD_READ_REGION = 0x9E;
/** Ustawienie regionu i zakresu kanałów. */
constexpr uint8_t CMD_SET_REGION = 0x22;
/** Odczyt mocy portów antenowych. */
constexpr uint8_t CMD_READ_ANTENNA_POWER = 0x94;
/** Ustawienie mocy wyjściowej portów antenowych. */
constexpr uint8_t CMD_SET_RF_POWER = 0x2F;
/** Krótka inwentaryzacja Gen2 używana do sprawdzania połączenia anteny. */
constexpr uint8_t CMD_INVENTORY_G2 = 0x01;
/** Ustawienie maski aktywnych anten. */
constexpr uint8_t CMD_SET_ACTIVE_ANTENNAS = 0x3F;
/** Ustawienie parametru konfiguracji wskazanego identyfikatorem CFG. */
constexpr uint8_t CMD_SET_CFG = 0xEA;
/** Identyfikator ustawienia TagFocus. */
constexpr uint8_t CFG_TAG_FOCUS = 0x08; // check later
/** Identyfikator ustawienia Q i sesji. */
constexpr uint8_t CFG_Q_AND_SESSION = 0x09;
/** Identyfikator ustawienia odczytu TID. */
constexpr uint8_t CFG_TID = 0x0A;
/** Identyfikator maski filtrującej tagi. */
constexpr uint8_t CFG_MASK = 0x0B;


/** Uruchomienie ciągłego raportowania tagów w trybie fast inventory. */
constexpr uint8_t CMD_START_FAST_INVENTORY = 0x50;
/** Zatrzymanie trybu fast inventory. */
constexpr uint8_t CMD_STOP_FAST_INVENTORY = 0x51;
/** Włączenie kontroli połączenia anten. */
constexpr uint8_t CMD_SET_ANTENNA_CHECK = 0x66;
/** Kod asynchronicznej ramki zawierającej odczytany tag. */
constexpr uint8_t CMD_FAST_INVENTORY_TAG = 0xEE;
///@}

/** @name Statusy odpowiedzi używane przy sprawdzaniu połączenia anten */
///@{
/** Zakończenie inwentaryzacji Gen2. */
constexpr uint8_t STATUS_INVENTORY_COMPLETED = 0x01;
/** Upłynięcie czasu inwentaryzacji Gen2. */
constexpr uint8_t STATUS_INVENTORY_TIMEOUT = 0x02;
/** Zapowiedź kolejnych ramek wyniku inwentaryzacji Gen2. */
constexpr uint8_t STATUS_INVENTORY_MORE_FRAMES = 0x03;
/** Zapełnienie pamięci wyniku inwentaryzacji Gen2. */
constexpr uint8_t STATUS_INVENTORY_MEMORY_FULL = 0x04;
/** Błąd połączenia anteny. */
constexpr uint8_t STATUS_ANTENNA_CONNECTION_ERROR = 0xF8;
/** Błąd komunikacji czytnika z tagiem. */
constexpr uint8_t STATUS_TAG_COMMUNICATION_ERROR = 0xFA;
/** Brak tagów, na których można wykonać operację. */
constexpr uint8_t STATUS_NO_OPERABLE_TAGS = 0xFB;
/** Błąd zgłoszony przez tag. */
constexpr uint8_t STATUS_TAG_ERROR = 0xFC;
///@}
////////////////////////////////////////////////////////

/** Rodzaj wysłanej komendy, której odpowiedzi oczekuje program. */
enum class PendingRequest {
  None,                 ///< Brak oczekującej komendy.
  ReaderInfo,           ///< Odczyt firmware i modelu.
  Temperature,          ///< Odczyt temperatury.
  WorkMode,             ///< Odczyt trybu pracy.
  ReadRegion,           ///< Odczyt regionu.
  SetRegion,            ///< Ustawienie regionu.
  ReadAntennaPower,     ///< Odczyt mocy anten.
  SetRfPower,           ///< Ustawienie mocy anten.
  EnableAntennaCheck,   ///< Włączenie kontroli połączenia anten.
  ProbeAntenna,         ///< Sprawdzenie jednego portu antenowego.
  ConfigureAntennas,    ///< Ustawienie aktywnych anten.
  DisableTagFocus,      ///< Wyłączenie TagFocus.
  SetQAndSession,       ///< Ustawienie Q i sesji.
  SetEpcMode,           ///< Wybór zwracania EPC bez TID.
  ClearInventoryMask,  ///< Wyłączenie filtrowania tagów.
  StartFastInventory,  ///< Uruchomienie inwentaryzacji.
  StopFastInventory    ///< Zatrzymanie inwentaryzacji.
};

/** Etap pojedynczej inwentaryzacji obsługiwanej w głównej pętli programu. */
enum class FastInventoryState {
  Idle,      ///< Oczekiwanie na nowe żądanie.
  Starting,  ///< Oczekiwanie na potwierdzenie START.
  Running,   ///< Odbiór tagów i odmierzanie czasu skanowania.
  Stopping,  ///< Oczekiwanie na potwierdzenie STOP.
  Draining,  ///< Odbiór końcowych danych po potwierdzonym STOP.
  Error      ///< Błąd blokujący rozpoczęcie kolejnej inwentaryzacji.
};

/** Kroki automatycznej konfiguracji realizowanej przez reader_configuration.h. */
enum class ReaderConfigurationStep {
  Idle,                ///< Konfiguracja nie została rozpoczęta.
  ReaderInfo,          ///< Odczyt firmware i modelu.
  WorkMode,            ///< Sprawdzenie trybu pracy.
  ReadRegion,          ///< Odczyt i porównanie regionu.
  SetRegion,           ///< Ustawienie wymaganego regionu.
  VerifyRegion,        ///< Weryfikacja ustawionego regionu.
  ReadPower,           ///< Odczyt i porównanie mocy anten.
  SetPower,            ///< Ustawienie wymaganej mocy anten.
  VerifyPower,         ///< Weryfikacja ustawionej mocy.
  EnableAntennaCheck,  ///< Włączenie kontroli połączenia anten.
  DetectAntennas,      ///< Wykrywanie podłączonych portów.
  ConfigureAntennas,   ///< Ustawienie maski wykrytych anten.
  DisableTagFocus,     ///< Wyłączenie TagFocus.
  SetQAndSession,      ///< Ustawienie parametrów Q i sesji.
  SetEpcMode,          ///< Wyłączenie dołączania TID.
  ClearMask,           ///< Wyłączenie filtrowania tagów.
  Completed,           ///< Konfiguracja zakończona poprawnie.
  Error                ///< Konfiguracja przerwana błędem.
};

/** Wynik porównania odczytanego ustawienia z konfiguracją wymaganą przez program. */
enum class ConfigurationCheck {
  Error,        ///< Nie udało się odczytać poprawnej wartości.
  Matches,      ///< Ustawienie jest zgodne.
  NeedsUpdate   ///< Ustawienie wymaga zmiany.
};

/** Interpretacja odpowiedzi na sprawdzenie połączenia portu antenowego. */
enum class AntennaProbeResult {
  Ignore,        ///< Odpowiedź nie dotyczy aktualnie sprawdzanego portu.
  MoreFrames,    ///< Należy oczekiwać na kolejne ramki tego sprawdzenia.
  Connected,     ///< Odpowiedź wskazuje podłączoną antenę.
  Disconnected,  ///< Czytnik zgłosił błąd połączenia anteny.
  Unknown        ///< Nie można określić stanu połączenia.
};

/** Bufor i stan składania pojedynczej ramki odpowiedzi z bajtów UART. */
struct RfidReceiver {
  /** Odebrane bajty ramki, łącznie z nagłówkiem i CRC16. */
  uint8_t buffer[256] = {};
  /** Liczba bajtów zgromadzonych w bieżącej ramce. */
  size_t byteCount = 0;
  /** Całkowita długość ramki wyznaczona na podstawie jej pierwszego bajtu. */
  size_t expectedLength = 0;
  /** Moment odebrania ostatniego bajtu w milisekundach. */
  uint32_t lastByteMs = 0;
};

/**
 *  Widok statusu i danych odebranej odpowiedzi, bez kopiowania bufora.
 *  Wskaźnik data pozostaje użyteczny tylko do nadpisania bufora rx.
 */
struct RfidResponseView {
  /** Status operacji zwrócony przez czytnik. */
  uint8_t status = 0;
  /** Początek danych odpowiedzi w buforze rx, za bajtem statusu. */
  const uint8_t* data = nullptr;
  /** Długość danych odpowiedzi, bez nagłówka i CRC16. */
  size_t dataLength = 0;
};

/** Jeden unikalny EPC zapisany w wyniku bieżącej inwentaryzacji. */
struct FastInventoryTag {
  /** Bajty identyfikatora EPC, bez dołączonego TID. */
  uint8_t epc[MAX_FAST_EPC_LENGTH] = {};
  /** Liczba używanych bajtów w tablicy epc. */
  uint8_t epcLength = 0;
};

/** Odbiornik ramki aktualnie składanej z danych UART. */
RfidReceiver rx;

/** @name Stan obsługi komend i konfiguracji */
///@{
/** Komenda oczekująca na odpowiedź. */
PendingRequest pendingRequest = PendingRequest::None;
/** Aktualny etap pojedynczej inwentaryzacji. */
FastInventoryState fastInventoryState = FastInventoryState::Idle;
/** Aktualny krok konfiguracji czytnika. */
ReaderConfigurationStep readerConfigurationStep =
  ReaderConfigurationStep::Idle;
/** Zezwolenie na wysłanie komendy przez automat konfiguracji. */
bool readerConfigurationDispatching = false;
///@}

/** @name Zmienne czasowe
 *  Momenty zdarzeń zapisywane na podstawie millis().
 */
///@{
/** Początek oczekiwania na odpowiedź; odświeżany przy kolejnych ramkach sprawdzania anteny. */
uint32_t requestStartedMs = 0;
/** Moment otrzymania poprawnego potwierdzenia START. */
uint32_t fastInventoryStartedMs = 0;
/** Moment ostatniej ramki tagu albo otrzymania potwierdzenia START lub STOP. */
uint32_t fastInventoryLastFrameMs = 0;
/** Moment rozpoczęcia odbioru końcowych danych po potwierdzeniu STOP. */
uint32_t fastInventoryDrainStartedMs = 0;
/** Początek sprawdzania portu, używany do kontroli bezwzględnego limitu czasu. */
uint32_t antennaProbeStartedMs = 0;
/** Najwcześniejszy dozwolony moment rozpoczęcia sprawdzania kolejnego portu. */
uint32_t nextAntennaProbeEarliestMs = 0;
///@}

/** @name Liczniki prób ustawienia konfiguracji */
///@{
/** Licznik kontroli limitu prób ustawienia regionu. */
uint8_t regionSetAttempts = 0;
/** Licznik kontroli limitu prób ustawienia mocy anten. */
uint8_t powerSetAttempts = 0;
///@}

/** @name Stan wykrywania anten
 *  W maskach bit 0 oznacza port 1, a kolejne bity odpowiadają kolejnym portom.
 */
///@{
/** Aktualnie sprawdzany port od 1 do ANTENNA_PORT_COUNT; 0 oznacza brak portu. */
uint8_t testedAntennaPort = 0;
/** Informacja, czy trwa sprawdzanie połączenia anten. */
bool antennaDetectionActive = false;
/** Porty, na których wykryto podłączoną antenę. */
uint16_t activeAntennaMask = 0;
/** Porty, na których czytnik zgłosił błąd połączenia anteny. */
uint16_t disconnectedAntennaMask = 0;
/** Porty jeszcze niesprawdzone albo bez jednoznacznego wyniku. */
uint16_t unknownAntennaMask = 0;
///@}

/** @name Wynik pojedynczej inwentaryzacji */
///@{
/** Tablica unikalnych EPC w kolejności pierwszego wykrycia. */
FastInventoryTag fastInventoryTags[MAX_FAST_INVENTORY_TAGS];
/** Liczba zapisanych elementów tablicy fastInventoryTags. */
size_t fastInventoryTagCount = 0;
/** Przepełnienie tablicy EPC, uniemożliwiające publikację pełnego wyniku. */
bool fastInventoryOverflow = false;
/** Odebrano nieprawidłowe dane tagu; komunikat zostanie wypisany przy zakończeniu. */
bool fastInventoryInvalidTagReceived = false;
///@}

/**
 *  Wyznaczenie bitu maski odpowiadającego portowi antenowemu.
 *  \param port Numer portu od 1 do ANTENNA_PORT_COUNT.
 *  \return Maska z jednym ustawionym bitem albo 0 dla nieprawidłowego portu.
 */
uint16_t antennaBit(uint8_t port) {
  if (port < 1 || port > ANTENNA_PORT_COUNT) {
    return 0;
  }

  return static_cast<uint16_t>(
    1UL << (port - 1));
}
////////////////////////////////////////////////////////

// Function declarations ///////////////////////////////
// Frame functions /////////////////////////////////////
bool validFrame(const uint8_t* frame, size_t frameLength);
size_t buildCommandFrame(uint8_t command, const uint8_t* data, size_t dataLength, uint8_t* frame, size_t frameCapacity);
void resetResponseReceiver();
bool sendFrame(uint8_t command, const uint8_t* data, size_t dataLength);
bool receiveFrame(size_t &responseLength);


// Reader command functions
bool startTrackedRequest(PendingRequest request, uint8_t command, const uint8_t* data, size_t dataLength);
bool finishTrackedRequest(PendingRequest expectedRequest);
bool prepareResponse(const char* operationName, size_t responseLength, RfidResponseView& response);
bool prepareSuccessfulResponse(const char* operationName, size_t responseLength, size_t minimumDataLength, RfidResponseView& response);
/**
 *  Rozpoczęcie automatycznej konfiguracji wolnego czytnika.
 *  Zeruje liczniki prób ustawień i wysyła żądanie informacji o czytniku.
 *  \return true, jeśli rozpoczęto pierwszy krok; false, jeśli czytnik jest zajęty
 *  albo nie udało się wysłać żądania.
 */
bool startReaderConfiguration();
/**
 *  Uruchomienie wskazanego kroku automatu konfiguracji.
 *  \param step Krok, który ma zostać wykonany.
 *  \return true, jeśli rozpoczęto krok lub osiągnięto Completed; false w razie
 *  błędu uruchomienia albo przekazania nieobsługiwanego kroku.
 */
bool startReaderConfigurationStep(ReaderConfigurationStep step);
/**
 *  Przejście do kolejnego kroku tylko wtedy, gdy trwa oczekiwany etap konfiguracji.
 *  \param expected Etap, którego odpowiedź została obsłużona.
 *  \param next Etap uruchamiany po zakończeniu oczekiwanego kroku.
 */
void continueReaderConfiguration(ReaderConfigurationStep expected, ReaderConfigurationStep next);
/**
 *  Wybór dalszego kroku konfiguracji na podstawie porównania regionu.
 *  Zgodny region pozwala przejść do mocy anten; niezgodny wymaga ustawienia
 *  i weryfikacji, a błąd przerywa konfigurację.
 *  \param result Wynik odczytu i porównania regionu.
 */
void handleRegionConfigurationResult(ConfigurationCheck result);
/**
 *  Wybór dalszego kroku konfiguracji na podstawie porównania mocy anten.
 *  Zgodna moc pozwala przejść do kontroli anten; niezgodna wymaga ustawienia
 *  i weryfikacji, a błąd przerywa konfigurację.
 *  \param result Wynik odczytu i porównania mocy portów.
 */
void handlePowerConfigurationResult(ConfigurationCheck result);
/**
 *  Przerwanie automatycznej konfiguracji przez ustawienie stanu Error.
 *  \param reason Powód przerwania wypisywany na port szeregowy.
 */
void failReaderConfiguration(const char* reason);
/**
 *  Sprawdzenie, czy automat konfiguracji wykonuje obecnie któryś z kroków.
 *  \return false dla Idle, Completed i Error; true dla pozostałych stanów.
 */
bool readerConfigurationIsActive();

bool requestReaderInfo();
bool handleReaderInfo(size_t responseLength);
bool requestReaderTemperature();
void handleReaderTemperature(size_t responseLength);
bool requestWorkMode();
bool handleWorkMode(size_t responseLength);
bool requestReadRegion();
ConfigurationCheck handleReadRegion(size_t responseLength);
bool requestSetRegion();
bool handleSetRegion(size_t responseLength);
bool requestReadAntennaPower();
ConfigurationCheck handleReadAntennaPower(size_t responseLength);
bool requestSetRfPower();
bool handleSetRfPower(size_t responseLength);
bool requestEnableAntennaCheck();
bool handleEnableAntennaCheck(size_t responseLength);
bool startAntennaDetection();
void serviceAntennaDetection();
bool requestAntennaProbe(uint8_t port);
AntennaProbeResult handleAntennaProbeResponse(size_t responseLength);
void recordAntennaProbeResult(AntennaProbeResult result);
void handleAntennaProbeTimeout();
void finishAntennaDetection();
void printAntennaPorts(const char* label, uint16_t mask);
bool requestConfigureAntennas();
bool handleConfigureAntennas(size_t responseLength);
bool requestDisableTagFocus();
bool handleDisableTagFocus(size_t responseLength);
bool requestSetQAndSession();
bool handleSetQAndSession(size_t responseLength);
bool requestSetEpcMode();
bool handleSetEpcMode(size_t responseLength);
bool requestClearInventoryMask();
bool handleClearInventoryMask(size_t responseLength);
bool startFastInventoryTest(uint32_t); // Zgodność z callback.h; argument jest ignorowany.
bool requestStartFastInventory();
bool handleStartFastInventory(size_t responseLength);
bool handleFastInventoryTag(size_t responseLength);
bool requestStopFastInventory();
bool handleStopFastInventory(size_t responseLength);
void serviceFastInventory();
void resetFastInventory();
void finishFastInventory();
void publishFastInventoryResult();

////////////////////////////////////////////////////////

/**
 *  Obsługa odpowiedzi czytnika i odmierzania czasu operacji.
 *  Przetwarza ograniczoną liczbę ramek, dopasowuje odpowiedzi do komend
 *  i obsługuje przekroczenie czasu oczekiwania. Następnie aktualizuje stan
 *  inwentaryzacji oraz wykrywania anten. Wywoływana regularnie z loop().
 */
void handleReaderRequest() {
  for (uint8_t processedFrameCount = 0;
       processedFrameCount < MAX_RX_FRAMES_PER_LOOP;
       ++processedFrameCount) {

    size_t responseLength = 0;
    if (!receiveFrame(responseLength)) {
      break;
    }

    const uint8_t responseCommand = rx.buffer[2];

    switch (responseCommand) {
      case CMD_GET_READER_INFO:
        if (!finishTrackedRequest(PendingRequest::ReaderInfo)) {
          break;
        }

        if (handleReaderInfo(responseLength)) {
          continueReaderConfiguration(ReaderConfigurationStep::ReaderInfo,
                                      ReaderConfigurationStep::WorkMode);
        } else if (readerConfigurationStep == ReaderConfigurationStep::ReaderInfo) {
          failReaderConfiguration("nie można odczytać informacji o czytniku");
        }
        break;

      case CMD_GET_READER_TEMPERATURE:
        if (!finishTrackedRequest(PendingRequest::Temperature)) {
          break;
        }

        handleReaderTemperature(responseLength);

        break;

      case CMD_GET_WORK_MODE: {
        if (!finishTrackedRequest(PendingRequest::WorkMode)) {
          break;
        }

        if (handleWorkMode(responseLength)) {
          continueReaderConfiguration(ReaderConfigurationStep::WorkMode,
                                      ReaderConfigurationStep::ReadRegion);
        } else if (readerConfigurationStep == ReaderConfigurationStep::WorkMode) {
          failReaderConfiguration("czytnik nie pracuje w answering mode");
        }
        break;
      }

      case CMD_READ_REGION: {
        if (!finishTrackedRequest(PendingRequest::ReadRegion)) {
          break;
        }

        handleRegionConfigurationResult(handleReadRegion(responseLength));
        break;
      }

      case CMD_SET_REGION:
        if (!finishTrackedRequest(PendingRequest::SetRegion)) {
          break;
        }

        if (handleSetRegion(responseLength)) {
          continueReaderConfiguration(ReaderConfigurationStep::SetRegion,
                                      ReaderConfigurationStep::VerifyRegion);
        } else if (readerConfigurationStep == ReaderConfigurationStep::SetRegion) {
          failReaderConfiguration("nie można ustawić regionu");
        }
        break;

      case CMD_READ_ANTENNA_POWER: {
        if (!finishTrackedRequest(PendingRequest::ReadAntennaPower)) {
          break;
        }

        handlePowerConfigurationResult(handleReadAntennaPower(responseLength));
        break;
      }

      case CMD_SET_RF_POWER:
        if (!finishTrackedRequest(PendingRequest::SetRfPower)) {
          break;
        }

        if (handleSetRfPower(responseLength)) {
          continueReaderConfiguration(ReaderConfigurationStep::SetPower,
                                      ReaderConfigurationStep::VerifyPower);
        } else if (readerConfigurationStep == ReaderConfigurationStep::SetPower) {
          failReaderConfiguration("nie można ustawić mocy anten");
        }
        break;

      case CMD_SET_ANTENNA_CHECK:
        if (!finishTrackedRequest(PendingRequest::EnableAntennaCheck)) {
          break;
        }

        if (handleEnableAntennaCheck(responseLength)) {
          continueReaderConfiguration(ReaderConfigurationStep::EnableAntennaCheck,
                                      ReaderConfigurationStep::DetectAntennas);
        } else if (readerConfigurationStep == ReaderConfigurationStep::EnableAntennaCheck) {
          failReaderConfiguration("nie można włączyć kontroli anten");
        }
        break;

      case CMD_INVENTORY_G2: {
        if (pendingRequest != PendingRequest::ProbeAntenna) {
          Serial.println(
            "[ERROR][RFID] Odpowiedź Inventory_G2 nie pasuje do oczekiwanej komendy");
          break;
        }

        const AntennaProbeResult result =
          handleAntennaProbeResponse(responseLength);

        if (result == AntennaProbeResult::Ignore) {
          break;
        }

        if (result == AntennaProbeResult::MoreFrames) {
          // Refresh the inactivity timeout; the absolute limit stays unchanged.
          requestStartedMs = millis();
          break;
        }

        if (!finishTrackedRequest(PendingRequest::ProbeAntenna)) {
          break;
        }

        recordAntennaProbeResult(result);
      }
      break;
      case CMD_SET_ACTIVE_ANTENNAS: {
        if (!finishTrackedRequest(PendingRequest::ConfigureAntennas)) {
          break;
        }

        // Błędny ACK konfiguracji ma zatrzymać automat zamiast zostawić go w Idle.
        if (handleConfigureAntennas(responseLength)) {
          continueReaderConfiguration(ReaderConfigurationStep::ConfigureAntennas,
                                      ReaderConfigurationStep::DisableTagFocus);
        } else if (readerConfigurationStep == ReaderConfigurationStep::ConfigureAntennas) {
          failReaderConfiguration("nie można skonfigurować aktywnych anten");
        }
        break;
      }

      case CMD_SET_CFG: {
        const PendingRequest completed = pendingRequest;
        bool success = false;
        ReaderConfigurationStep expected = ReaderConfigurationStep::Idle;
        ReaderConfigurationStep next = ReaderConfigurationStep::Error;

        if (completed == PendingRequest::DisableTagFocus) {
          expected = ReaderConfigurationStep::DisableTagFocus;
          next = ReaderConfigurationStep::SetQAndSession;
          success = finishTrackedRequest(completed) && handleDisableTagFocus(responseLength);
        } else if (completed == PendingRequest::SetQAndSession) {
          expected = ReaderConfigurationStep::SetQAndSession;
          next = ReaderConfigurationStep::SetEpcMode;
          success = finishTrackedRequest(completed) && handleSetQAndSession(responseLength);
        } else if (completed == PendingRequest::SetEpcMode) {
          expected = ReaderConfigurationStep::SetEpcMode;
          next = ReaderConfigurationStep::ClearMask;
          success = finishTrackedRequest(completed) && handleSetEpcMode(responseLength);
        } else if (completed == PendingRequest::ClearInventoryMask) {
          expected = ReaderConfigurationStep::ClearMask;
          next = ReaderConfigurationStep::Completed;
          success = finishTrackedRequest(completed) && handleClearInventoryMask(responseLength);
        } else {
          Serial.println("[ERROR][RFID] Nieoczekiwana odpowiedź konfiguracji 0xEA");
        }

        if (success) {
          continueReaderConfiguration(expected, next);
        } else if (readerConfigurationStep == expected) {
          failReaderConfiguration("błąd parametru fast inventory");
        }
        break;
      }

      case CMD_START_FAST_INVENTORY:
        if (!finishTrackedRequest(PendingRequest::StartFastInventory)) {
          break;
        }

        if (!handleStartFastInventory(responseLength)) {
          fastInventoryState = FastInventoryState::Error;
        }
        break;

      case CMD_FAST_INVENTORY_TAG:
        // Ramki 0xEE są asynchroniczne i nie kończą pendingRequest.
        handleFastInventoryTag(responseLength);
        break;

      case CMD_STOP_FAST_INVENTORY:
        if (!finishTrackedRequest(PendingRequest::StopFastInventory)) {
          break;
        }

        if (!handleStopFastInventory(responseLength)) {
          fastInventoryState = FastInventoryState::Error;
        }
        break;

      default:
        Serial.printf("\n[ERROR][RFID] Nieobsługiwana odpowiedź: 0x%02X\n", static_cast<unsigned>(responseCommand));
        break;
    }
  }

  const uint32_t now = millis();
  const bool responseTimedOut =
    pendingRequest != PendingRequest::None &&
    static_cast<uint32_t>(now - requestStartedMs) > COMM_TIMEOUT_MS;

  const bool antennaProbeExceededAbsoluteLimit =
    pendingRequest == PendingRequest::ProbeAntenna &&
    static_cast<uint32_t>(now - antennaProbeStartedMs) >
      ANTENNA_PROBE_MAX_DURATION_MS;

  if (responseTimedOut || antennaProbeExceededAbsoluteLimit) {
    const PendingRequest timedOutRequest = pendingRequest;

    if (timedOutRequest == PendingRequest::ProbeAntenna) {
      Serial.printf(
        "[ERROR][RFID] Timeout testu ANT%u\n",
        static_cast<unsigned>(testedAntennaPort));
    }
    else {
      Serial.println("[ERROR][RFID] Timeout odpowiedzi");
    }

    resetResponseReceiver();
    pendingRequest = PendingRequest::None;

    if (timedOutRequest == PendingRequest::ProbeAntenna) {
      handleAntennaProbeTimeout();
    }
    else if (timedOutRequest == PendingRequest::StartFastInventory ||
             timedOutRequest == PendingRequest::StopFastInventory) {
      fastInventoryState = FastInventoryState::Error;
      Serial.println(
        "[ERROR][RFID] Nie można potwierdzić stanu fast inventory");
    }
    else if (readerConfigurationIsActive()) {
      failReaderConfiguration("timeout odpowiedzi");
    }
  }

  serviceFastInventory();
  serviceAntennaDetection();
}
/////////////////////////////////////////////////////////////////////////


// Function definitions /////////////////////////////////////////////////

/**
 *  Sprawdzenie długości ramki odpowiedzi i zgodności jej sumy CRC16.
 *  \param frame Wskaźnik na całą odebraną ramkę.
 *  \param frameLength Liczba bajtów ramki, łącznie z polem długości i CRC16.
 *  \return true dla poprawnej długości i CRC; false także dla pustego wskaźnika.
 */
bool validFrame(const uint8_t* frame, size_t frameLength)
{
  constexpr size_t MIN_RESPONSE_LENGTH = 6;
  constexpr size_t MAX_FRAME_LENGTH = 256;

  if (frame == nullptr ||
      frameLength < MIN_RESPONSE_LENGTH ||
      frameLength > MAX_FRAME_LENGTH) {
    return false;
  }

  const size_t declaredLength = static_cast<size_t>(frame[0]) + 1;
  if (declaredLength != frameLength) {
    return false;
  }

  const size_t crcIndex = frameLength - 2;
  const uint16_t receivedCrc =
    static_cast<uint16_t>(frame[crcIndex]) | // bitwise OR
    (static_cast<uint16_t>(frame[crcIndex + 1]) << 8); // bitwise left-shift
  const uint16_t calculatedCrc =
    uiCrc16(frame, static_cast<uint8_t>(crcIndex));

  return calculatedCrc == receivedCrc;
}

/**
 *  Zbudowanie ramki komendy z adresem czytnika, danymi i sumą CRC16.
 *  Kolejność pól: długość, adres, komenda, dane, młodszy i starszy bajt CRC16.
 *  \param command Kod komendy czytnika.
 *  \param data Dane komendy; nullptr jest dozwolone, gdy dataLength wynosi 0.
 *  \param dataLength Liczba bajtów danych komendy, maksymalnie 251.
 *  \param frame Bufor, w którym ma zostać zapisana gotowa ramka.
 *  \param frameCapacity Pojemność bufora frame w bajtach.
 *  \return Długość zbudowanej ramki albo 0 przy nieprawidłowych argumentach
 *  lub zbyt małym buforze.
 */
size_t buildCommandFrame(
  uint8_t        command,
  const uint8_t* data,
  size_t         dataLength,
  uint8_t*       frame,
  size_t         frameCapacity) {

  constexpr size_t MAX_DATA_LENGTH = 251;

  if (frame == nullptr) {
    return 0;
  }

  if (dataLength > 0 && data == nullptr) {
    return 0;
  }

  if (dataLength > MAX_DATA_LENGTH) {
    return 0;
  }

  const size_t frameLength = dataLength + 5;

  if (frameCapacity < frameLength) {
    return 0;
  }

  frame[0] = static_cast<uint8_t>(dataLength + 4); // Length
  frame[1] = ADDRESS; // Address
  frame[2] = command; // Command
  // frame[3] ... frame[3 + dataLength]
  if (dataLength > 0) {
    memcpy(frame + 3, data, dataLength);
  }

  const size_t crcByte = dataLength +3;
  const uint16_t crc = uiCrc16(frame, crcByte);
  frame[crcByte] = static_cast<uint8_t>(crc & 0x00FF); //bitwise AND
  frame[crcByte + 1] = static_cast<uint8_t>(crc >> 8); // bitwise shift to the right

  return frameLength;
}

/**
 *  Wyzerowanie liczników odbiornika przed składaniem kolejnej ramki.
 *  Nie usuwa danych zapisanych w rx.buffer ani bajtów oczekujących w UART.
 */
void resetResponseReceiver() {
    rx.byteCount = 0;
    rx.expectedLength = 0;
    rx.lastByteMs = 0;
}

/**
 *  Zbudowanie i wysłanie ramki komendy przez UART.
 *  Czeka na zakończenie nadawania, ale nie oczekuje na odpowiedź czytnika.
 *  \param command Kod wysyłanej komendy.
 *  \param data Dane komendy albo nullptr, jeśli komenda nie ma danych.
 *  \param dataLength Liczba bajtów danych.
 *  \return true, jeśli wysłano całą ramkę; false przy błędzie budowania lub zapisu.
 */
bool sendFrame(
  uint8_t command,
  const uint8_t* data,
  size_t dataLength) {

  if (dataLength > 0 && data == nullptr) {
    Serial.println("\n[ERROR][RFID] Brak danych wymaganych przez komendę");
    return false;
  }

  uint8_t frameBuf[256];
  const size_t frameLength = buildCommandFrame(
    command,
    data,
    dataLength,
    frameBuf,
    sizeof(frameBuf));

  if (frameLength == 0) {
    Serial.print("\n[ERROR][RFID] Nie udało się zbudować ramki komendy");
    return false;
  }

  const size_t written = RfidSerial.write(frameBuf, frameLength);
  if (written != frameLength) {
    Serial.print("\n[ERROR][RFID] Nie wysłano całej ramki komendy");
    return false;
  }

  RfidSerial.flush();
  return true;
}

/**
 *  Składanie jednej odpowiedzi z bajtów dostępnych w UART.
 *  Zachowuje niepełną ramkę do kolejnego wywołania, kontroluje odstępy między
 *  bajtami i CRC16. Gotowa ramka pozostaje w rx.buffer do dalszej obsługi.
 *  \param responseLength Długość odebranej ramki w bajtach; 0, jeśli nie ma wyniku.
 *  \return true po odebraniu poprawnej, pełnej ramki; false, gdy odbiór nie jest
 *  zakończony lub wykryto błąd.
 */
bool receiveFrame(size_t& responseLength) {

  responseLength = 0;

  while (RfidSerial.available() > 0) {
    const uint32_t now = millis();

    // Odrzuć niekompletną ramkę przed potraktowaniem kolejnego bajtu jako Len.
    if (rx.byteCount > 0 &&
        static_cast<uint32_t>(now - rx.lastByteMs) >
          INTER_BYTE_TIMEOUT_MS) {
      Serial.println(
        "\n[RFID] Przekroczono 15 ms pomiędzy bajtami");
      resetResponseReceiver();
    }

    const int value = RfidSerial.read();
    if (value < 0) {
      break;
    }

    const uint8_t readByte = static_cast<uint8_t>(value);

    if (rx.byteCount == 0) {
      if (readByte < 5) {
        continue;
      }

      rx.expectedLength = static_cast<size_t>(readByte) + 1;
      if (rx.expectedLength > sizeof(rx.buffer)) {
        Serial.printf(
          "\n[ERROR][RFID] Ramka wymaga %u bajtów, a bufor ma %u\n",
          static_cast<unsigned>(rx.expectedLength),
          static_cast<unsigned>(sizeof(rx.buffer)));
        resetResponseReceiver();
        return false;
      }
    }

    rx.buffer[rx.byteCount++] = readByte;
    rx.lastByteMs = millis();

    if (rx.byteCount == rx.expectedLength) {
      const size_t completedLength = rx.expectedLength;

      if (!validFrame(rx.buffer, completedLength)) {
        Serial.println("\n[RFID] Błąd CRC16 odpowiedzi");
        resetResponseReceiver();
        return false;
      }

      responseLength = completedLength;
      resetResponseReceiver();
      return true;
    }
  }

  if (rx.byteCount > 0 &&
      static_cast<uint32_t>(millis() - rx.lastByteMs) > INTER_BYTE_TIMEOUT_MS) {
    Serial.println("\n[RFID] Przekroczono 15 ms pomiędzy bajtami");
    resetResponseReceiver();
  }

  return false;
}

/**
 *  Wysłanie komendy i rozpoczęcie śledzenia jej odpowiedzi.
 *  Sprawdza zajętość czytnika oraz wyłączność komend konfiguracji i STOP.
 *  \param request Rodzaj żądania zapisywany w pendingRequest.
 *  \param command Kod komendy wysyłanej do czytnika.
 *  \param data Dane komendy albo nullptr przy zerowej długości.
 *  \param dataLength Liczba bajtów danych komendy.
 *  \return true po wysłaniu komendy i zapisaniu czasu oczekiwania;
 *  false, gdy komenda jest zablokowana lub transmisja się nie powiodła.
 */
bool startTrackedRequest(PendingRequest request, uint8_t command, const uint8_t* data, size_t dataLength) {

  // Rezerwujemy kanał komend dla STOP, aby diagnostyka nie wydłużyła skanowania.
  if (fastInventoryState != FastInventoryState::Idle &&
      fastInventoryState != FastInventoryState::Error &&
      request != PendingRequest::StopFastInventory) {
    Serial.println("[ERROR][RFID] Czytnik zajęty inwentaryzacją");
    return false;
  }

  const bool configurationOwnsRequest =
    readerConfigurationDispatching ||
    (readerConfigurationStep == ReaderConfigurationStep::DetectAntennas &&
     request == PendingRequest::ProbeAntenna);
  if (readerConfigurationIsActive() && !configurationOwnsRequest) {
    Serial.println("[ERROR][RFID] Trwa konfiguracja startowa czytnika");
    return false;
  }

  if (pendingRequest != PendingRequest::None) {
    Serial.print("\n[ERROR][RFID] Inna komenda nadal oczekuje");
    return false;
  }

  if (!sendFrame(command, data, dataLength)) {
    Serial.printf("\n[ERROR][RFID] Nie wysłano komendy 0x%02X", command);
    return false;
  }

  pendingRequest = request;
  requestStartedMs = millis();

  return true;
}

/**
 *  Zakończenie oczekiwania na odpowiedź właściwego rodzaju komendy.
 *  Nie sprawdza statusu wykonania komendy zwróconego przez czytnik.
 *  \param expectedRequest Rodzaj żądania odpowiadający odebranej ramce.
 *  \return true po wyzerowaniu pendingRequest; false, gdy oczekuje inne żądanie.
 */
bool finishTrackedRequest(PendingRequest expectedRequest) {
  if (pendingRequest != expectedRequest) {
    Serial.println(
      "[ERROR][RFID] Odpowiedź nie pasuje do oczekiwanej komendy");
    return false;
  }

  pendingRequest = PendingRequest::None;
  return true;
}

/**
 *  Przygotowanie widoku statusu i danych odpowiedzi znajdującej się w rx.buffer.
 *  Sprawdza zakres długości ramki, zakładając wcześniejszą kontrolę CRC16.
 *  Widok nie kopiuje danych i traci aktualność po nadpisaniu bufora odbiornika.
 *  \param operationName Nazwa operacji używana w komunikatach błędów.
 *  \param responseLength Liczba bajtów całej odebranej ramki.
 *  \param response Widok wypełniany po sprawdzeniu długości; zerowany przed kontrolą.
 *  \return true, jeśli długość pozwala odczytać status i wyznaczyć dane.
 */
bool prepareResponse(
  const char* operationName,
  size_t responseLength,
  RfidResponseView& response) {

  response = {};

  if (responseLength > sizeof(rx.buffer)) {
    Serial.printf(
      "[ERROR][RFID] %s: długość ramki przekracza rozmiar bufora\n",
      operationName);
    return false;
  }

  constexpr size_t MIN_RESPONSE_LENGTH = 6;
  if (responseLength < MIN_RESPONSE_LENGTH) {
    Serial.printf(
      "[ERROR][RFID] %s: ramka odpowiedzi jest za krótka\n",
      operationName);
    return false;
  }

  response.status = rx.buffer[3];
  response.data = rx.buffer + 4;
  response.dataLength = responseLength - MIN_RESPONSE_LENGTH;
  return true;
}

/**
 *  Sprawdzenie odpowiedzi, dla której poprawny status wykonania wynosi 0x00.
 *  Oprócz długości ramki kontroluje status i minimalną ilość danych.
 *  \param operationName Nazwa operacji używana w komunikatach błędów.
 *  \param responseLength Liczba bajtów całej ramki w rx.buffer.
 *  \param minimumDataLength Minimalna wymagana liczba bajtów danych odpowiedzi.
 *  \param response Widok poprawnej odpowiedzi; w razie błędu pozostaje wyzerowany.
 *  \return true dla statusu 0x00 i wystarczającej ilości danych; false przy błędzie.
 */
bool prepareSuccessfulResponse(
  const char* operationName,
  size_t responseLength,
  size_t minimumDataLength,
  RfidResponseView& response) {

  if (!prepareResponse(operationName, responseLength, response)) {
    return false;
  }

  if (response.status != 0x00) {
    Serial.printf(
      "[ERROR][RFID] %s: czytnik zwrócił status 0x%02X\n",
      operationName,
      static_cast<unsigned>(response.status));
    response = {};
    return false;
  }

  if (response.dataLength < minimumDataLength) {
    Serial.printf(
      "[ERROR][RFID] %s: oczekiwano co najmniej %u bajtów danych, odebrano %u\n",
      operationName,
      static_cast<unsigned>(minimumDataLength),
      static_cast<unsigned>(response.dataLength));
    response = {};
    return false;
  }

  return true;
}

/**
 *  Wysłanie zapytania o wersję firmware i model czytnika.
 *  \return true, gdy wysłano komendę i rozpoczęto oczekiwanie na odpowiedź.
 */
bool requestReaderInfo() {
  return startTrackedRequest(PendingRequest::ReaderInfo, CMD_GET_READER_INFO, nullptr, 0);
}

/**
 *  Sprawdzenie odpowiedzi i wyświetlenie wersji firmware oraz modelu czytnika.
 *  \param responseLength Długość całej ramki odpowiedzi w buforze rx.buffer.
 *  \return true, gdy odpowiedź potwierdza powodzenie i zawiera wymagane dane.
 */
bool handleReaderInfo(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Odbieranie informacji czytnika",
        responseLength,
        3,
        response)) {
    return false;
  }

  Serial.printf(
    "Wersja firmware: %u.%02u, model: 0x%02X\n",
    static_cast<unsigned>(response.data[0]),
    static_cast<unsigned>(response.data[1]),
    static_cast<unsigned>(response.data[2]));

  return true;
}

/**
 *  Wysłanie zapytania o temperaturę czytnika.
 *  \return true, gdy wysłano komendę i rozpoczęto oczekiwanie na odpowiedź.
 */
bool requestReaderTemperature() {
  return startTrackedRequest(PendingRequest::Temperature, CMD_GET_READER_TEMPERATURE, nullptr, 0);
}

/**
 *  Sprawdzenie odpowiedzi i wyświetlenie temperatury czytnika w stopniach Celsjusza.
 *  Uwzględnia znak temperatury przekazany przez czytnik.
 *  \param responseLength Długość całej ramki odpowiedzi w buforze rx.buffer.
 */
void handleReaderTemperature(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Odczyt temperatury czytnika",
        responseLength,
        2,
        response)) {
    return;
  }

  Serial.print("\n[RFID] Temperatura czytnika: ");
  // plus or minus
  if (response.data[0] == 0) {
    Serial.print("-");
  }
  Serial.printf("%d℃", static_cast<unsigned>(response.data[1]));
}

/**
 *  Wysłanie zapytania o tryb pracy czytnika.
 *  \return true, gdy wysłano komendę i rozpoczęto oczekiwanie na odpowiedź.
 */
bool requestWorkMode() {
  return startTrackedRequest(PendingRequest::WorkMode, CMD_GET_WORK_MODE, nullptr, 0);
}

/**
 *  Odczyt trybu pracy i sprawdzenie, czy czytnik pracuje w answering mode.
 *  \param responseLength Długość całej ramki odpowiedzi w buforze rx.buffer.
 *  \return true, gdy odpowiedź jest poprawna i tryb pracy ma wartość 0x00.
 */
bool handleWorkMode(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Tryb pracy czytnika",
        responseLength,
        1,
        response)) {
    return false;
  }

  const uint8_t workMode = response.data[0];

  Serial.print("\n[RFID] Tryb czytnika: ");
  Serial.printf(
    "0x%02X",
    static_cast<unsigned>(workMode));

  if (workMode != 0x00) {
    Serial.println(
      "\n[ERROR][RFID] Czytnik nie jest w answering mode");
    return false;
  }

  return true;
}

/**
 *  Wysłanie zapytania o region i zakres kanałów częstotliwości czytnika.
 *  \return true, gdy wysłano komendę i rozpoczęto oczekiwanie na odpowiedź.
 */
bool requestReadRegion() {
  return startTrackedRequest(
    PendingRequest::ReadRegion,
    CMD_READ_REGION,
    nullptr,
    0);
}

/**
 *  Porównanie odczytanego regionu i granic kanałów z konfiguracją EU3 programu.
 *  \param responseLength Długość całej ramki odpowiedzi w buforze rx.buffer.
 *  \return Matches dla zgodnych ustawień, NeedsUpdate dla wymaganej zmiany
 *  lub Error, gdy odpowiedź jest niepoprawna.
 */
ConfigurationCheck handleReadRegion(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Odczyt regionu",
        responseLength,
        3,
        response)) {
    return ConfigurationCheck::Error;
  }

  const uint8_t frequencyBand = response.data[0];
  const uint8_t maximumChannel = response.data[1];
  const uint8_t minimumChannel = response.data[2];

  Serial.printf(
    "[RFID] FreBand=0x%02X, MaxFre=0x%02X, MinFre=0x%02X\n",
    static_cast<unsigned>(frequencyBand),
    static_cast<unsigned>(maximumChannel),
    static_cast<unsigned>(minimumChannel));

  const bool regionMatches =
    frequencyBand == RFID_REGION_EU3 &&
    maximumChannel == RFID_REGION_MAX_CHANNEL &&
    minimumChannel == RFID_REGION_MIN_CHANNEL;

  if (regionMatches) {
    Serial.println(
      "[RFID] Region jest poprawny: EU3 865.7-867.5 MHz");
    return ConfigurationCheck::Matches;
  }

  Serial.println("[RFID] Region wymaga ustawienia");
  return ConfigurationCheck::NeedsUpdate;
}

/**
 *  Wysłanie komendy ustawiającej region EU3 i skonfigurowany zakres kanałów.
 *  Ustawienie jest tymczasowe i nie jest zachowywane po wyłączeniu zasilania.
 *  \return true, gdy wysłano komendę i rozpoczęto oczekiwanie na odpowiedź.
 */
bool requestSetRegion() {
  const uint8_t data[] = {
    0x01, // temporary setting, do not save on power-off
    RFID_REGION_EU3,
    RFID_REGION_MAX_CHANNEL,
    RFID_REGION_MIN_CHANNEL
  };

  return startTrackedRequest(
    PendingRequest::SetRegion,
    CMD_SET_REGION,
    data,
    sizeof(data));
}

/**
 *  Sprawdzenie potwierdzenia tymczasowego ustawienia regionu czytnika.
 *  \param responseLength Długość całej ramki odpowiedzi w buforze rx.buffer.
 *  \return true, gdy odebrano poprawne potwierdzenie powodzenia komendy.
 */
bool handleSetRegion(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Ustawienie regionu EU3",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.println("[RFID] Region ustawiony tymczasowo");
  return true;
}

/**
 *  Wysłanie zapytania o moc wszystkich portów antenowych.
 *  \return true, gdy wysłano komendę i rozpoczęto oczekiwanie na odpowiedź.
 */
bool requestReadAntennaPower() {
  return startTrackedRequest(
    PendingRequest::ReadAntennaPower,
    CMD_READ_ANTENNA_POWER,
    nullptr,
    0);
}

/**
 *  Wyświetlenie mocy portów antenowych i porównanie ich z RFID_POWER_DBM.
 *  \param responseLength Długość całej ramki odpowiedzi w buforze rx.buffer.
 *  \return Matches, gdy moc wszystkich portów jest zgodna, NeedsUpdate dla
 *  wymaganej zmiany lub Error, gdy odpowiedź jest niepoprawna.
 */
ConfigurationCheck handleReadAntennaPower(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Odczyt mocy portów antenowych",
        responseLength,
        ANTENNA_PORT_COUNT,
        response)) {
    return ConfigurationCheck::Error;
  }

  bool allPowersMatch = true;

  for (size_t i = 0; i < ANTENNA_PORT_COUNT; i++) {
    const uint8_t powerDbm =
      static_cast<uint8_t>(response.data[i] & 0x7F);

    Serial.printf(
      "[RFID] ANT%u: %u dBm\n",
      static_cast<unsigned>(i + 1),
      static_cast<unsigned>(powerDbm));

    if (powerDbm != RFID_POWER_DBM) {
      allPowersMatch = false;
    }
  }

  if (allPowersMatch) {
    Serial.printf(
      "[RFID] Wszystkie porty mają ustawione %u dBm\n",
      static_cast<unsigned>(RFID_POWER_DBM));
    return ConfigurationCheck::Matches;
  }

  Serial.println("[RFID] Moc portów wymaga ustawienia");
  return ConfigurationCheck::NeedsUpdate;
}

/**
 *  Wysłanie komendy ustawiającej moc RFID_POWER_DBM na wszystkich portach.
 *  Ustawienie jest tymczasowe i nie jest zachowywane po wyłączeniu zasilania.
 *  \return true, gdy wysłano komendę i rozpoczęto oczekiwanie na odpowiedź.
 */
bool requestSetRfPower() {

  uint8_t data[ANTENNA_PORT_COUNT];
  const uint8_t temporaryPower =
    static_cast<uint8_t>(0x80 | RFID_POWER_DBM);

  for (size_t i = 0; i < ANTENNA_PORT_COUNT; i++) {
    data[i] = temporaryPower;
  }

  return startTrackedRequest(
    PendingRequest::SetRfPower,
    CMD_SET_RF_POWER,
    data,
    sizeof(data));
}

/**
 *  Sprawdzenie potwierdzenia tymczasowego ustawienia mocy portów antenowych.
 *  \param responseLength Długość całej ramki odpowiedzi w buforze rx.buffer.
 *  \return true, gdy odebrano poprawne potwierdzenie powodzenia komendy.
 */
bool handleSetRfPower(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Ustawienie mocy portów antenowych",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.println("[RFID] Moc wszystkich portów ustawiona tymczasowo");
  return true;
}

/**
 *  Wysłanie komendy włączającej kontrolę połączenia anten w czytniku.
 *  \return true, gdy wysłano komendę i rozpoczęto oczekiwanie na odpowiedź.
 */
bool requestEnableAntennaCheck() {
  const uint8_t data[] = {
    0x01
  };

  return startTrackedRequest(PendingRequest::EnableAntennaCheck, CMD_SET_ANTENNA_CHECK, data, sizeof(data));
}

/**
 *  Sprawdzenie potwierdzenia włączenia kontroli połączenia anten.
 *  \param responseLength Długość całej ramki odpowiedzi w buforze rx.buffer.
 *  \return true, gdy odebrano poprawne potwierdzenie powodzenia komendy.
 */
bool handleEnableAntennaCheck(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Włączenie Antenna Check",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.println("[RFID] Kontrola anten została włączona");
  return true;
}

/**
 *  Przygotowanie nieblokującego sprawdzania wszystkich portów antenowych.
 *  Zeruje poprzednie wyniki i początkowo oznacza każdy port jako nieznany.
 *  Kolejne próby są uruchamiane przez serviceAntennaDetection().
 *  \return true, gdy rozpoczęto wykrywanie, lub false, gdy czytnik jest zajęty.
 */
bool startAntennaDetection() {
  if (fastInventoryState != FastInventoryState::Idle ||
      (readerConfigurationIsActive() && !readerConfigurationDispatching)) {
    Serial.println("[ERROR][RFID] Czytnik jest zajęty");
    return false;
  }

  if (antennaDetectionActive) {
    Serial.println("[ERROR][RFID] Sprawdzanie anten już trwa");
    return false;
  }

  if (pendingRequest != PendingRequest::None) {
    Serial.println(
      "[ERROR][RFID] Nie można rozpocząć sprawdzania anten, inna komenda oczekuje");
    return false;
  }

  testedAntennaPort = 1;
  activeAntennaMask = 0;
  disconnectedAntennaMask = 0;
  unknownAntennaMask = ALL_ANTENNA_PORTS_MASK;
  antennaDetectionActive = true;
  nextAntennaProbeEarliestMs = millis();

  Serial.printf(
    "\n[RFID] Rozpoczynam sprawdzanie %u portów antenowych\n",
    static_cast<unsigned>(ANTENNA_PORT_COUNT));
  return true;
}

/**
 *  Obsługa kolejnego kroku wykrywania anten z głównej pętli programu.
 *  Wysyła próbę dla bieżącego portu, gdy nie trwa inne żądanie i upłynęła
 *  wymagana przerwa. Błąd wysłania zapisuje jako wynik nieznany.
 */
void serviceAntennaDetection() {
  if (!antennaDetectionActive ||
      pendingRequest != PendingRequest::None) {
    return;
  }

  if (testedAntennaPort < 1 ||
      testedAntennaPort > ANTENNA_PORT_COUNT) {
    finishAntennaDetection();
    return;
  }

  const uint32_t now = millis();
  if (static_cast<int32_t>(
        now - nextAntennaProbeEarliestMs) < 0) {
    return;
  }

  if (!requestAntennaProbe(testedAntennaPort)) {
    Serial.printf(
      "[ERROR][RFID] Nie udało się wysłać testu ANT%u\n",
      static_cast<unsigned>(testedAntennaPort));
    recordAntennaProbeResult(AntennaProbeResult::Unknown);

    if (antennaDetectionActive) {
      nextAntennaProbeEarliestMs =
        millis() + ANTENNA_PROBE_TIMEOUT_RECOVERY_MS;
    }
  }
}

/**
 *  Wysłanie krótkiej inwentaryzacji G2 sprawdzającej wybrany port antenowy.
 *  Zapamiętuje czas rozpoczęcia próby do kontroli jej maksymalnego czasu.
 *  \param port Numer portu od 1 do ANTENNA_PORT_COUNT.
 *  \return true, gdy wysłano komendę i rozpoczęto oczekiwanie na odpowiedź.
 */
bool requestAntennaProbe(uint8_t port) {
  if (port < 1 || port > ANTENNA_PORT_COUNT) {
    Serial.printf(
      "[ERROR][RFID] Nieprawidłowy port anteny: %u\n",
      static_cast<unsigned>(port));
    return false;
  }

  const uint8_t data[] = {
    ANTENNA_PROBE_Q_VALUE,
    ANTENNA_PROBE_SESSION,
    ANTENNA_PROBE_TARGET,
    static_cast<uint8_t>(
      ANTENNA_SELECTOR_BASE | (port - 1)),
    ANTENNA_PROBE_SCAN_TIME
  };

  Serial.printf(
    "\n[RFID] Test ANT%u (%u ms)\n",
    static_cast<unsigned>(port),
    static_cast<unsigned>(ANTENNA_PROBE_SCAN_TIME) * 100U);

  if (!startTrackedRequest(
        PendingRequest::ProbeAntenna,
        CMD_INVENTORY_G2,
        data,
        sizeof(data))) {
    return false;
  }

  antennaProbeStartedMs = requestStartedMs;
  return true;
}

/**
 *  Określenie stanu połączenia anteny na podstawie odpowiedzi próby G2.
 *  Status 0xF8 oznacza nieudaną kontrolę połączenia. Końcowe statusy
 *  inwentaryzacji i błędy tagów potwierdzają połączenie także bez odczytu EPC.
 *  Ramki zapowiadające dalsze dane wymagają kontynuacji odbioru, a odpowiedzi
 *  dotyczące innego portu są pomijane.
 *  \param responseLength Długość całej ramki odpowiedzi w buforze rx.buffer.
 *  \return Connected, Disconnected lub Unknown dla wyniku próby,
 *  MoreFrames przy oczekiwaniu na dalsze dane lub Ignore dla pomijanej ramki.
 */
AntennaProbeResult handleAntennaProbeResponse(
  size_t responseLength) {

  RfidResponseView response;
  if (!prepareResponse(
        "Odpowiedź testu połączenia anteny",
        responseLength,
        response)) {
    return AntennaProbeResult::Unknown;
  }

  if (!antennaDetectionActive ||
      testedAntennaPort < 1 ||
      testedAntennaPort > ANTENNA_PORT_COUNT) {
    Serial.println(
      "[ERROR][RFID] Odebrano wynik testu bez aktywnego portu");
    return AntennaProbeResult::Ignore;
  }

  const bool inventoryStatusWithAntennaNumber =
    response.status >= STATUS_INVENTORY_COMPLETED &&
    response.status <= STATUS_INVENTORY_MEMORY_FULL;

  if (inventoryStatusWithAntennaNumber) {
    constexpr size_t MIN_INVENTORY_DATA_LENGTH = 2; // Ant + Num

    if (response.dataLength < MIN_INVENTORY_DATA_LENGTH) {
      Serial.printf(
        "[ERROR][RFID] ANT%u: odpowiedź inventory ma za mało danych\n",
        static_cast<unsigned>(testedAntennaPort));

      // Status 0x03 still announces another frame, so keep draining it.
      if (response.status == STATUS_INVENTORY_MORE_FRAMES) {
        return AntennaProbeResult::MoreFrames;
      }

      return AntennaProbeResult::Unknown;
    }

    const uint8_t expectedAntenna =
      static_cast<uint8_t>(testedAntennaPort - 1);

    if (response.data[0] != expectedAntenna) {
      Serial.printf(
        "[RFID] Pomijam odpowiedź ANT%u podczas oczekiwania na ANT%u\n",
        static_cast<unsigned>(response.data[0] + 1),
        static_cast<unsigned>(testedAntennaPort));
      return AntennaProbeResult::Ignore;
    }
  }

  switch (response.status) {
    case STATUS_INVENTORY_MORE_FRAMES:
      return AntennaProbeResult::MoreFrames;

    case STATUS_INVENTORY_COMPLETED:
    case STATUS_INVENTORY_TIMEOUT:
    case STATUS_INVENTORY_MEMORY_FULL:
    case STATUS_TAG_COMMUNICATION_ERROR:
    case STATUS_NO_OPERABLE_TAGS:
    case STATUS_TAG_ERROR:
      return AntennaProbeResult::Connected;

    case STATUS_ANTENNA_CONNECTION_ERROR:
      return AntennaProbeResult::Disconnected;

    default:
      Serial.printf(
        "[ERROR][RFID] ANT%u: nieoczekiwany status 0x%02X\n",
        static_cast<unsigned>(testedAntennaPort),
        static_cast<unsigned>(response.status));
      return AntennaProbeResult::Unknown;
  }
}

/**
 *  Zapisanie końcowego wyniku bieżącego portu w odpowiedniej masce anten.
 *  Przechodzi do następnego portu lub kończy wykrywanie po ostatniej próbie.
 *  \param result Końcowy wynik: Connected, Disconnected albo Unknown.
 */
void recordAntennaProbeResult(AntennaProbeResult result) {
  if (!antennaDetectionActive ||
      testedAntennaPort < 1 ||
      testedAntennaPort > ANTENNA_PORT_COUNT) {
    return;
  }

  const uint16_t bit = antennaBit(testedAntennaPort);

  activeAntennaMask &= static_cast<uint16_t>(~bit);
  disconnectedAntennaMask &= static_cast<uint16_t>(~bit);
  unknownAntennaMask &= static_cast<uint16_t>(~bit);

  switch (result) {
    case AntennaProbeResult::Connected:
      activeAntennaMask |= bit;
      Serial.printf(
        "[RFID] ANT%u: podłączona\n",
        static_cast<unsigned>(testedAntennaPort));
      break;

    case AntennaProbeResult::Disconnected:
      disconnectedAntennaMask |= bit;
      Serial.printf(
        "[RFID] ANT%u: nie przeszła kontroli połączenia (0xF8)\n",
        static_cast<unsigned>(testedAntennaPort));
      break;

    case AntennaProbeResult::Unknown:
      unknownAntennaMask |= bit;
      Serial.printf(
        "[RFID] ANT%u: wynik nieznany\n",
        static_cast<unsigned>(testedAntennaPort));
      break;

    case AntennaProbeResult::Ignore:
    case AntennaProbeResult::MoreFrames:
      return;
  }

  ++testedAntennaPort;

  if (testedAntennaPort > ANTENNA_PORT_COUNT) {
    finishAntennaDetection();
  }
  else {
    nextAntennaProbeEarliestMs = millis();
  }
}

/**
 *  Oznaczenie wyniku bieżącej próby jako nieznanego po przekroczeniu czasu.
 *  Opóźnia próbę następnego portu, aby umożliwić odebranie spóźnionej ramki.
 */
void handleAntennaProbeTimeout() {
  recordAntennaProbeResult(AntennaProbeResult::Unknown);

  if (antennaDetectionActive) {
    // Leave time to drain a delayed frame before testing another port.
    nextAntennaProbeEarliestMs =
      millis() + ANTENNA_PROBE_TIMEOUT_RECOVERY_MS;
  }
}

/**
 *  Zakończenie wykrywania i wyświetlenie list podłączonych, odłączonych
 *  oraz nierozpoznanych anten. W konfiguracji startowej przechodzi do
 *  ustawienia aktywnych anten lub zgłasza błąd, jeśli nie wykryto żadnej.
 */
void finishAntennaDetection() {
  antennaDetectionActive = false;
  testedAntennaPort = 0;

  Serial.println("\n[RFID] Zakończono sprawdzanie anten");
  printAntennaPorts("Podłączone", activeAntennaMask);
  printAntennaPorts("Odłączone", disconnectedAntennaMask);
  printAntennaPorts("Nieznane", unknownAntennaMask);

  if (activeAntennaMask == 0) {
    Serial.println(
      "[ERROR][RFID] Nie wykryto żadnej aktywnej anteny");
    if (readerConfigurationStep == ReaderConfigurationStep::DetectAntennas) {
      failReaderConfiguration("nie wykryto aktywnej anteny");
    }
    return;
  }

  continueReaderConfiguration(ReaderConfigurationStep::DetectAntennas,
                              ReaderConfigurationStep::ConfigureAntennas);
}

/**
 *  Wyświetlenie maski anten i odpowiadających jej numerów portów.
 *  \param label Opis wyświetlany przed maską i listą anten.
 *  \param mask Maska portów, w której bit 0 odpowiada ANT1.
 */
void printAntennaPorts(const char* label, uint16_t mask) {
  Serial.printf(
    "[RFID] %s [maska 0x%04X]: ",
    label,
    static_cast<unsigned>(mask));

  bool first = true;

  for (uint8_t port = 1; port <= ANTENNA_PORT_COUNT; ++port) {
    if ((mask & antennaBit(port)) == 0) {
      continue;
    }

    if (!first) {
      Serial.print(", ");
    }

    Serial.printf("ANT%u", static_cast<unsigned>(port));
    first = false;
  }

  if (first) {
    Serial.print("brak");
  }

  Serial.println();
}

/**
 *  Wysłanie komendy tymczasowo aktywującej anteny z activeAntennaMask.
 *  Pusta maska uniemożliwia wysłanie komendy.
 *  \return true, gdy wysłano komendę i rozpoczęto oczekiwanie na odpowiedź.
 */
bool requestConfigureAntennas() {
  if (activeAntennaMask == 0) {
    return false;
  }

  const uint8_t data[] = {
    0x01, // don't save
    static_cast<uint8_t>(activeAntennaMask >> 8), // antennas 9 - 16
    static_cast<uint8_t>(activeAntennaMask) // antennas 1 - 8
  };

  return startTrackedRequest(
    PendingRequest::ConfigureAntennas,
    CMD_SET_ACTIVE_ANTENNAS,
    data,
    sizeof(data));
}


/**
 *  Sprawdzenie potwierdzenia ustawienia maski aktywnych anten.
 *  \param responseLength Długość całej ramki odpowiedzi w buforze rx.buffer.
 *  \return true, gdy odebrano poprawne potwierdzenie powodzenia komendy.
 */
bool handleConfigureAntennas(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Ustawianie maski aktywnych anten",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.print("\n[RFID] Skonfigurowano multipleksację anten");
  return true;
}

/**
 *  Wysłanie komendy tymczasowo wyłączającej TagFocus dla inwentaryzacji.
 *  \return true, gdy wysłano komendę i rozpoczęto oczekiwanie na odpowiedź.
 */
bool requestDisableTagFocus() {
  const uint8_t data[] = {
    0x01, // temporary setting, do not save on power-off
    CFG_TAG_FOCUS,
    TAG_FOCUS  // disable TagFocus
  };

  if (!startTrackedRequest(
        PendingRequest::DisableTagFocus,
        CMD_SET_CFG,
        data,
        sizeof(data))) {
    return false;
  }

  return true;
}

/**
 *  Sprawdzenie potwierdzenia wyłączenia TagFocus.
 *  \param responseLength Długość całej ramki odpowiedzi w buforze rx.buffer.
 *  \return true, gdy odebrano poprawne potwierdzenie powodzenia komendy.
 */
bool handleDisableTagFocus(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Wyłączanie TagFocus",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.println("[RFID] TagFocus wyłączony");
  return true;
}

/**
 *  Wysłanie komendy tymczasowo ustawiającej Q_VALUE i SESSION dla inwentaryzacji.
 *  \return true, gdy wysłano komendę i rozpoczęto oczekiwanie na odpowiedź.
 */
bool requestSetQAndSession() {
  const uint8_t data[] = {
    0x01, // temporary setting, do not save on power-off
    CFG_Q_AND_SESSION,
    Q_VALUE,
    SESSION
  };

  if (!startTrackedRequest(
        PendingRequest::SetQAndSession,
        CMD_SET_CFG,
        data,
        sizeof(data))) {
    return false;
  }

  return true;
}

/**
 *  Sprawdzenie potwierdzenia ustawienia parametru Q i sesji inwentaryzacji.
 *  \param responseLength Długość całej ramki odpowiedzi w buforze rx.buffer.
 *  \return true, gdy odebrano poprawne potwierdzenie powodzenia komendy.
 */
bool handleSetQAndSession(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Ustawianie parametru Q i Session",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.printf("\n[RFID] Ustawiono Q = %d, Session = S%d", Q_VALUE, SESSION);
  return true;
}

/**
 *  Wysłanie komendy tymczasowo wybierającej zwracanie EPC podczas inwentaryzacji.
 *  Zerowa długość TID wybiera tryb odczytu EPC.
 *  \return true, gdy wysłano komendę i rozpoczęto oczekiwanie na odpowiedź.
 */
bool requestSetEpcMode() {
  const uint8_t data[] = {
    0x01, // temporary setting, do not save on power-off
    CFG_TID,
    0x06, // default TID word address, ignored for EPC mode
    0x00  // LenTID=0 selects EPC inventory
  };

  if (!startTrackedRequest(
        PendingRequest::SetEpcMode,
        CMD_SET_CFG,
        data,
        sizeof(data))) {
    return false;
  }

  return true;
}

/**
 *  Sprawdzenie potwierdzenia ustawienia zwracania EPC podczas inwentaryzacji.
 *  \param responseLength Długość całej ramki odpowiedzi w buforze rx.buffer.
 *  \return true, gdy odebrano poprawne potwierdzenie powodzenia komendy.
 */
bool handleSetEpcMode(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Ustawianie trybu danych fast inventory",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.println("[RFID] Skonfigurowano zwracanie EPC (LenTID=0)");
  return true;
}

/**
 *  Wysłanie komendy tymczasowo wyłączającej filtrowanie tagów maską EPC.
 *  Zerowa długość maski pozwala objąć inwentaryzacją wszystkie tagi.
 *  \return true, gdy wysłano komendę i rozpoczęto oczekiwanie na odpowiedź.
 */
bool requestClearInventoryMask() {
  const uint8_t data[] = {
    0x01, // temporary setting, do not save on power-off
    CFG_MASK,
    0x01,       // EPC memory
    0x00, 0x20, // default mask bit address
    0x00        // MaskLen=0 disables filtering
  };

  if (!startTrackedRequest(
        PendingRequest::ClearInventoryMask,
        CMD_SET_CFG,
        data,
        sizeof(data))) {
    return false;
  }

  return true;
}

/**
 *  Sprawdzenie potwierdzenia wyłączenia maski tagów podczas inwentaryzacji.
 *  \param responseLength Długość całej ramki odpowiedzi w buforze rx.buffer.
 *  \return true, gdy odebrano poprawne potwierdzenie powodzenia komendy.
 */
bool handleClearInventoryMask(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Wyłączanie maski fast inventory",
        responseLength,
        0,
        response)) {
    return false;
  }

  Serial.println("[RFID] Maska tagów wyłączona");
  return true;
}

/**
 *  Wyczyszczenie listy EPC, flag błędów i czasów przed nową inwentaryzacją.
 *  Nie wysyła komend ani nie zmienia etapu zapisanego w fastInventoryState.
 */
void resetFastInventory() {
  memset(fastInventoryTags, 0, sizeof(fastInventoryTags));
  fastInventoryTagCount = 0;
  fastInventoryOverflow = false;
  fastInventoryInvalidTagReceived = false;
  fastInventoryStartedMs = 0;
  fastInventoryLastFrameMs = 0;
  fastInventoryDrainStartedMs = 0;
}

/**
 *  Uruchomienie pojedynczej inwentaryzacji przez dawny interfejs callback.h.
 *  Nienazwany argument liczbowy jest ignorowany, także gdy wynosi 0.
 *  Każde przyjęte żądanie oznacza jeden skan, bez serii i automatycznych powtórzeń.
 *  \return Wynik requestStartFastInventory(): true po wysłaniu START,
 *  false, jeśli nie można rozpocząć inwentaryzacji.
 */
bool startFastInventoryTest(uint32_t) {
  return requestStartFastInventory();
}

/**
 *  Wysłanie START dla jednej inwentaryzacji po zakończeniu konfiguracji.
 *  Wymaga wolnego czytnika, aktywnej anteny i zakończonego wykrywania portów.
 *  Czyści poprzedni wynik; odliczanie 5 sekund rozpoczyna się dopiero po
 *  otrzymaniu poprawnego potwierdzenia START.
 *  \return true po wysłaniu START i przejściu do Starting; false, jeśli warunki
 *  startu nie są spełnione lub wysłanie komendy się nie powiodło.
 */
bool requestStartFastInventory() {
  if (fastInventoryState != FastInventoryState::Idle ||
      readerConfigurationStep != ReaderConfigurationStep::Completed ||
      antennaDetectionActive ||
      pendingRequest != PendingRequest::None ||
      activeAntennaMask == 0) {
    Serial.println(
      "[ERROR][RFID] Fast inventory nie może zostać uruchomione");
    return false;
  }

  resetFastInventory();

  const uint8_t data[] = {
    FAST_INVENTORY_TARGET
  };

  if (!startTrackedRequest(
        PendingRequest::StartFastInventory,
        CMD_START_FAST_INVENTORY,
        data,
        sizeof(data))) {
    return false;
  }

  fastInventoryState = FastInventoryState::Starting;
  return true;
}

/**
 *  Sprawdzenie potwierdzenia START i rozpoczęcie odmierzania czasu skanowania.
 *  Poprawna odpowiedź przełącza inwentaryzację do stanu Running.
 *  \param responseLength Liczba bajtów całej ramki odpowiedzi w rx.buffer.
 *  \return true dla poprawnego potwierdzenia; false przy błędzie odpowiedzi.
 */
bool handleStartFastInventory(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Uruchamianie fast inventory",
        responseLength,
        0,
        response)) {
    return false;
  }

  // Pięć sekund od potwierdzenia START; odbiór danych po STOP jest osobnym etapem.
  fastInventoryStartedMs = millis();
  fastInventoryLastFrameMs = fastInventoryStartedMs;
  fastInventoryState = FastInventoryState::Running;

  Serial.printf(
    "[RFID] Rozpoczęto inwentaryzację na %lu ms\n",
    static_cast<unsigned long>(FAST_INVENTORY_TIME_MS));
  return true;
}

/**
 *  Odczytanie EPC z asynchronicznej ramki tagu i dodanie go do wyniku.
 *  Przyjmuje tagi w stanach Starting, Running, Stopping i Draining.
 *  Rozpoznaje dodatkowe pola ramki, pomija TID w trybie FastID i nie zapisuje
 *  duplikatów EPC. Nie kończy żądania oczekującego w pendingRequest.
 *  \param responseLength Liczba bajtów całej ramki tagu w rx.buffer.
 *  \return true, jeśli zapisano nowy EPC lub znaleziono już zapisany;
 *  false przy nieaktywnym odbiorze, nieprawidłowych danych lub przepełnieniu.
 */
bool handleFastInventoryTag(size_t responseLength) {
  const bool inventoryAcceptsTags =
    fastInventoryState == FastInventoryState::Starting ||
    fastInventoryState == FastInventoryState::Running ||
    fastInventoryState == FastInventoryState::Stopping ||
    fastInventoryState == FastInventoryState::Draining;

  if (!inventoryAcceptsTags) {
    return false;
  }

  fastInventoryLastFrameMs = millis();

  constexpr size_t MIN_FAST_TAG_FRAME_LENGTH = 9;
  if (responseLength < MIN_FAST_TAG_FRAME_LENGTH ||
      rx.buffer[3] != 0x00) {
    fastInventoryInvalidTagReceived = true;
    return false;
  }

  const size_t dataLength = responseLength - 6;
  const uint8_t* data = rx.buffer + 4;
  const uint8_t antennaRaw = data[0];
  const uint8_t lengthFlags = data[1];
  const size_t identifierLength = lengthFlags & 0x3F;
  const bool hasPhaseAndFrequency =
    (lengthFlags & 0x40) != 0;
  const bool hasFastId = (lengthFlags & 0x80) != 0;

  const size_t expectedDataLength =
    3 + identifierLength +
    (hasPhaseAndFrequency ? 7 : 0);

  if (antennaRaw >= ANTENNA_PORT_COUNT ||
      identifierLength == 0 ||
      identifierLength > MAX_FAST_EPC_LENGTH ||
      dataLength != expectedDataLength) {
    fastInventoryInvalidTagReceived = true;
    return false;
  }

  size_t epcLength = identifierLength;

  // W trybie FastID ostatnie 12 bajtów identyfikatora stanowi TID.
  if (hasFastId) {
    constexpr size_t FAST_ID_TID_LENGTH = 12;

    if (identifierLength <= FAST_ID_TID_LENGTH) {
      fastInventoryInvalidTagReceived = true;
      return false;
    }

    epcLength -= FAST_ID_TID_LENGTH;
  }

  const uint8_t* epc = data + 2;

  for (size_t i = 0; i < fastInventoryTagCount; ++i) {
    const FastInventoryTag& tag = fastInventoryTags[i];

    if (tag.epcLength == epcLength &&
        memcmp(tag.epc, epc, epcLength) == 0) {
      return true;
    }
  }

  if (fastInventoryTagCount >= MAX_FAST_INVENTORY_TAGS) {
    fastInventoryOverflow = true;
    return false;
  }

  FastInventoryTag& tag =
    fastInventoryTags[fastInventoryTagCount++];
  tag = FastInventoryTag{};
  memcpy(tag.epc, epc, epcLength);
  tag.epcLength = static_cast<uint8_t>(epcLength);
  return true;
}

/**
 *  Zakończenie inwentaryzacji po odebraniu wszystkich końcowych danych.
 *  Przepełnienie tablicy EPC ustawia Error i blokuje publikację niepełnej listy.
 *  W pozostałych przypadkach zgłasza odrzucone dane tagów, jeśli takie były,
 *  przechodzi do Idle i wywołuje publikację wyniku przez MQTT.
 */
void finishFastInventory() {
  // Nie publikujemy niepełnej listy EPC po przepełnieniu bufora.
  if (fastInventoryOverflow) {
    fastInventoryState = FastInventoryState::Error;
    Serial.println(
      "[ERROR][RFID] Za dużo unikalnych EPC; wynik inwentaryzacji odrzucony");
    return;
  }

  if (fastInventoryInvalidTagReceived) {
    Serial.println(
      "[ERROR][RFID] Pominięto nieprawidłowe dane tagów podczas inwentaryzacji");
  }

  fastInventoryState = FastInventoryState::Idle;
  Serial.println("[RFID] Inwentaryzacja zakończona");
  publishFastInventoryResult();
}

/**
 *  Wysłanie komendy STOP i rozpoczęcie oczekiwania na jej potwierdzenie.
 *  \return true po wysłaniu komendy i przejściu do Stopping;
 *  false, jeśli nie udało się rozpocząć żądania.
 */
bool requestStopFastInventory() {
  if (!startTrackedRequest(
        PendingRequest::StopFastInventory,
        CMD_STOP_FAST_INVENTORY,
        nullptr,
        0)) {
    return false;
  }

  fastInventoryState = FastInventoryState::Stopping;
  return true;
}

/**
 *  Sprawdzenie potwierdzenia STOP i rozpoczęcie odbioru końcowych danych.
 *  Poprawna odpowiedź ustawia stan Draining i czas rozpoczęcia tego etapu.
 *  \param responseLength Liczba bajtów całej ramki odpowiedzi w rx.buffer.
 *  \return true dla poprawnego potwierdzenia; false przy błędzie odpowiedzi.
 */
bool handleStopFastInventory(size_t responseLength) {
  RfidResponseView response;
  if (!prepareSuccessfulResponse(
        "Zatrzymywanie fast inventory",
        responseLength,
        0,
        response)) {
    return false;
  }

  const uint32_t now = millis();
  fastInventoryDrainStartedMs = now;
  fastInventoryLastFrameMs = now;
  fastInventoryState = FastInventoryState::Draining;

  return true;
}

/**
 *  Obsługa czasu skanowania i zakończenia pojedynczej inwentaryzacji.
 *  Po upływie 5 sekund od potwierdzenia START wysyła STOP. Po potwierdzeniu
 *  STOP czeka na pusty odbiornik i co najmniej 50 ms ciszy. Jeśli po 500 ms
 *  warunki zakończenia nadal nie są spełnione, ustawia Error i blokuje publikację.
 *  Wywoływana z handleReaderRequest(); opóźnienie głównej pętli opóźnia
 *  również obsługę tych terminów.
 */
void serviceFastInventory() {
  const uint32_t now = millis();

  if (fastInventoryState == FastInventoryState::Running &&
      static_cast<uint32_t>(now - fastInventoryStartedMs) >=
        FAST_INVENTORY_TIME_MS) {
    if (!requestStopFastInventory()) {
      fastInventoryState = FastInventoryState::Error;
      Serial.println(
        "[ERROR][RFID] Nie wysłano komendy stop fast inventory");
    }

    return;
  }

  if (fastInventoryState != FastInventoryState::Draining) {
    return;
  }

  const bool receiverIsEmpty =
    RfidSerial.available() == 0 &&
    rx.byteCount == 0;
  const bool drainRanLongEnough =
    static_cast<uint32_t>(now - fastInventoryDrainStartedMs) >=
      FAST_INVENTORY_DRAIN_QUIET_MS;
  const bool noRecentTagFrames =
    static_cast<uint32_t>(now - fastInventoryLastFrameMs) >=
      FAST_INVENTORY_DRAIN_QUIET_MS;
  const bool drainLimitReached =
    static_cast<uint32_t>(now - fastInventoryDrainStartedMs) >=
      FAST_INVENTORY_DRAIN_MAX_MS;

  if (receiverIsEmpty &&
      drainRanLongEnough &&
      noRecentTagFrames) {
    // Publikujemy wynik dopiero po odebraniu ostatnich tagów z bufora RX.
    finishFastInventory();
    return;
  }

  if (drainLimitReached) {
    // Nie publikujemy wyniku, gdy odbiór końcowych danych nie został zakończony.
    fastInventoryState = FastInventoryState::Error;
    Serial.println(
      "[ERROR][RFID] Nie opróżniono RX po fast inventory; wynik odrzucony");
  }
}

/**
 *  Przygotowanie i wysłanie listy EPC na topic reader_read_tags.
 *  Tworzy JSON w formacie {"tags":["AABB",...]} w kolejności zapisanych tagów;
 *  pusty wynik ma postać {"tags":[]}. Dobiera rozmiar bufora MQTT do wiadomości.
 *  Brak pamięci na JSON lub bufor MQTT przerywa publikację z komunikatem błędu.
 */
void publishFastInventoryResult() {
  constexpr char HEX_DIGITS[] = "0123456789ABCDEF";

  // {"tags":["AABB",...]}: dwa znaki na każdy bajt EPC oraz cudzysłowy
  // i przecinki dla każdego elementu.
  size_t payloadLength = sizeof("{\"tags\":[]}") - 1;
  for (size_t i = 0; i < fastInventoryTagCount; ++i) {
    payloadLength +=
      (2U * fastInventoryTags[i].epcLength) + 2U;
    if (i > 0) {
      ++payloadLength;
    }
  }

  String payload;
  if (!payload.reserve(payloadLength)) {
    Serial.println(
      "[ERROR][MQTT] Brak pamięci na JSON wyniku inwentaryzacji");
    return;
  }

  payload = "{\"tags\":[";
  for (size_t i = 0; i < fastInventoryTagCount; ++i) {
    const FastInventoryTag& tag = fastInventoryTags[i];

    if (i > 0) {
      payload += ',';
    }
    payload += '"';

    for (size_t byteIndex = 0;
         byteIndex < tag.epcLength;
         ++byteIndex) {
      const uint8_t value = tag.epc[byteIndex];
      payload += HEX_DIGITS[value >> 4];
      payload += HEX_DIGITS[value & 0x0F];
    }

    payload += '"';
  }
  payload += "]}";

  // Domyślny bufor PubSubClient (256 B) może być za mały dla listy EPC.
  const size_t mqttBufferSize =
    payload.length() + strlen(reader_read_tags.c_str()) + 16U;
  if (!client.setBufferSize(mqttBufferSize)) {
    Serial.println(
      "[ERROR][MQTT] Nie można powiększyć bufora wyniku inwentaryzacji");
    return;
  }

  publishMessage(reader_read_tags.c_str(), payload, false, false);
}


#include "reader_configuration.h"

#endif
