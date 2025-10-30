#ifndef __NFC_READER_HPP__
#define __NFC_READER_HPP__

#include <stdint.h>
#include <stdbool.h>
#include "esp_log.h"

#include "arduino/mfrc522.hpp"

#define THREAD_COMMAND_NFC_SECTION  (1000)
#define THREAD_COMMAND_NFC_CARD     (1 + THREAD_COMMAND_NFC_SECTION)

void NfcReaderTask(void *args);





#endif // __NFC_READER_HPP__