#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <optional>

namespace fs = std::filesystem;
/**
 * @brief File stream for handling operations on the "wowExe" file.
 *
 * This file stream is used to read from and write to the "wowExe" file within the application.
 * The stream must be properly opened before use and should be checked for any errors during operations.
 */
std::fstream wowExe;
/**
 * @brief The expected size for the executable file.
 *
 * This constant defines the expected file size in bytes for the executable.
 * It is used during the validation process to ensure that the file has not
 * been modified or corrupted.
 */
constexpr std::streamsize kExpectedSize = 0x757C00;

/**
 * Checks for errors in a given output stream and logs an error message if any error is detected.
 *
 * @param stream The output stream to check for errors.
 * @param message The error message to log if the stream is in a failed state.
 * @return True if the stream is in a failed state and the error message was logged, otherwise false.
 */
bool handleStreamError(const std::ostream &stream, const std::string &message) {
    if (!stream) {
        std::cerr << message << std::endl;
        return true;
    }
    return false;
}

// Utility function to write data at a specific position (for ranges)
template<typename T>
/**
 * Writes a sequence of bytes to a specific position in a binary stream.
 *
 * This function attempts to write a sequence of byte values starting at a specified
 * position in an external file stream referred to by the global variable `wowExe`.
 * If the stream is not open, an error message is displayed and the function returns.
 * The function checks for errors at each step: repositioning the write pointer
 * and writing each value.
 *
 * @tparam T The type of elements in the vector to be written to the stream.
 * @param pos The position in the stream where the writing should start.
 * @param values A vector containing the values to be written to the stream.
 */
void writeBytesAt(const std::streampos pos, const std::vector<T> &values) {
    if (!wowExe) {
        std::cerr << "Stream is not open\n";
        return;
    }
    wowExe.clear();
    wowExe.seekp(pos);
    if (handleStreamError(wowExe, "Failed to set position in the stream"))
        return;

    for (const auto &value: values) {
        wowExe.write(reinterpret_cast<const char *>(&value), sizeof(value));
        if (handleStreamError(wowExe, "Failed to write to the stream"))
            return;
    }
}

/**
 * @brief Writes a byte to a specific position in a file.
 *
 * This function writes a single byte to a given position in the already opened file stream `wowExe`.
 * If the file is not open, it prints an error message and returns immediately.
 * It clears the stream's error flags, seeks to the specified position, and performs the write operation.
 * After seeking and writing, it checks for stream errors using `handleStreamError`.
 *
 * @param pos The position in the file where the byte should be written.
 * @param value The byte value to write to the file.
 */
void writeByteAt(const std::streampos &pos, const uint8_t value) {
    if (!wowExe.is_open()) {
        std::cerr << "File is not open.\n";
        return;
    }
    wowExe.clear();
    wowExe.seekp(pos);
    if (handleStreamError(wowExe, "Unable to seek position"))
        return;

    wowExe.write(reinterpret_cast<const char *>(&value), sizeof(value));
    handleStreamError(wowExe, "Unable to write to file");
}

/**
 * Writes a specified number of repeated bytes to a file stream at a given position.
 *
 * This function writes `n` bytes with the value `value` to the `wowExe` file stream starting at the specified position `pos`.
 * It first checks if the file stream `wowExe` is open. Then, it attempts to seek to the given position. If seeking to the
 * position fails, or if the write operation itself fails, appropriate error messages are displayed.
 *
 * @param pos  The position in the file to start writing bytes.
 * @param value The byte value to be written repeatedly.
 * @param n The number of bytes to write.
 */
void writeRepeatedBytesAt(const std::streampos &pos, const uint8_t value, const size_t n) {
    if (!wowExe) {
        std::cerr << "File stream is not open.\n";
        return;
    }
    wowExe.clear();
    wowExe.seekp(pos);
    if (handleStreamError(wowExe, "Failed to seek to position"))
        return;

    const std::vector buffer(n, value);
    const auto dataSize = static_cast<std::streamsize>(buffer.size());
    wowExe.write(reinterpret_cast<const char*>(buffer.data()), dataSize);
    handleStreamError(wowExe, "Write operation failed");
}

/**
 * @brief Creates a backup of the specified file.
 *
 * This function attempts to create a backup copy of the file specified by `filepath`.
 * The backup file will have a ".backup" extension appended to the original file name.
 * If the backup is successfully created, the path to the backup file is returned.
 * If the backup creation fails, an empty optional is returned.
 *
 * @param filepath The path to the file that needs to be backed up.
 * @return A std::optional containing the backup file path if the backup is successful;
 *         otherwise, an empty std::optional.
 */
[[nodiscard]] std::optional<std::string> createBackup(const std::string &filepath) {
    std::string backupPath = filepath + ".backup";
    try {
        fs::copy(filepath, backupPath, fs::copy_options::overwrite_existing);
        std::cout << "Backup created at: " << backupPath << "\n";
        return backupPath;
    } catch (const std::exception &e) {
        std::cerr << "Failed to create backup: " << e.what() << "\n";
        return std::nullopt;
    }
}

/**
 * Validates whether the given file path points to a valid executable file.
 *
 * This function performs the following checks on the specified file:
 *  1. Verifies that the file exists.
 *  2. Opens the file in binary mode to ensure it is accessible.
 *  3. Checks that the file size matches the expected size.
 *
 * @param filepath The path to the executable file to be validated.
 * @return true if the executable is valid, false otherwise.
 */
[[nodiscard]] bool validateExecutable(const std::string &filepath) {
    if (!fs::exists(filepath)) {
        std::cerr << "Executable not found.\n";
        return false;
    }
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open executable for validation.\n";
        return false;
    }
    file.seekg(0, std::ios::end);
    if (file.tellg() != kExpectedSize) {
        std::cerr << "Validation failed: unexpected file size.\n";
        return false;
    }
    std::cout << "Executable validation passed.\n";
    return true;
}

/**
 * Restores a backup of a file.
 *
 * This function checks if a backup file with the same name as the input file
 * parameter exists but with a ".backup" extension. If it exists, the function
 * attempts to remove the original file and rename the backup file to the
 * original file name.
 *
 * @param wow The path to the original file for which the backup should be restored.
 * @return true if the backup was successfully restored, false otherwise.
 */
bool restoreBackup(const std::string &wow) {
    if (const std::string backupPath = wow + ".backup"; fs::exists(backupPath)) {
        try {
            fs::remove(wow);
            fs::rename(backupPath, wow);
            return true;
        } catch (const fs::filesystem_error &e) {
            std::cerr << "Failed to restore backup: " << e.what() << "\n";
            return false;
        }
    }
    std::cerr << "Backup not found.\n";
    return false;
}

/**
 * @brief Main function to patch the World of Warcraft executable.
 *
 * This function performs several operations to patch the World of Warcraft executable
 * located at the provided path. It verifies if the path is valid, creates a backup
 * of the executable, validates the executable before patching, and applies several
 * patches to it. The patches include fixes such as resolving a remote code execution
 * exploit, enabling full screen mode from windowed mode, making certain animations
 * and actions consistent, among others.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments, where `argv[1]` should be the path
 * to the World of Warcraft executable.
 *
 * @return An integer indicating the status of the execution. Returns `EXIT_SUCCESS`
 * if patching is completed successfully, or `EXIT_FAILURE` if an error occurs at any
 * stage of the process.
 */
int main(const int argc, char **argv) {
    constexpr int errorState = EXIT_SUCCESS;
    if (argc < 2) {
        std::cerr << "World of Warcraft exe path not provided!\n";
        return EXIT_FAILURE;
    }

    const std::string wowPath = argv[1];

    if (!fs::exists(wowPath)) {
        std::cerr << "Executable not found at: " << wowPath << "\n";
        return EXIT_FAILURE;
    }

    if (!createBackup(wowPath)) {
        std::cerr << "Backup creation failed. Aborting.\n";
        return EXIT_FAILURE;
    }

    if (!validateExecutable(wowPath)) {
        std::cerr << "Executable validation failed. Aborting.\n";
        return EXIT_FAILURE;
    }

    wowExe.open(wowPath, std::ios::in | std::ios::out | std::ios::binary);
    if (!wowExe) {
        std::cerr << "Failed to open executable for patching.\n";
        return EXIT_FAILURE;
    }

    std::vector<std::pair<std::streampos, std::vector<std::uint8_t> > > patches = {
        // Remote code execution exploit
        {0x2A7, {0xC0}},
        // Windowed mode to full screen
        {0xE94, {0xEB}},
        // Melee swing on right-click
        {0x2E1C67, std::vector<uint8_t>(11, 0x90)},
        // NPC attack animation when turning
        {0x33D7C9, {0xEB}},
        // "Ghost" attack when NPC evades combat
        {0x355BF, {0xEB}},
        // Missing pre-cast animation for spells
        {0x33E0D6, std::vector<uint8_t>(22, 0x90)},
        // Patch mail timeout
        {0x16D899, {0x05, 0x01, 0x00, 0x00, 0x00}},
        // Area trigger timer precision
        {0x2DB241, {50}},
        // Blue Moon
        {0x5CFBC0, {0xC7, 0x05, 0x74, 0x8E, 0xD3, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xC3}},
        // Mouse flickering and camera snapping issue when mouse has high report rate
        {0x469A2C, {0xE9, 0x71, 0xF0, 0x0B, 0x00, 0xF8, 0x13, 0xD4, 0x00, 0x8B, 0x1D, 0xFC}},
        {
            0x528AA2, {
                0x8D, 0x4D, 0xF0, 0x51, 0x57, 0xFF, 0x15, 0xDC, 0xF5, 0x9D, 0x00, 0x8B, 0x45, 0xF0, 0x8B, 0x15,
                0xF8,
                0x13, 0xD4, 0x00, 0xE9, 0x7A, 0x0F, 0xF4, 0xFF
            }
        },
        {
            0x4691B1, {
                0x89, 0xE5, 0x8B, 0x05, 0xFC, 0x13, 0xD4, 0x00, 0x8B, 0x0D, 0xF8, 0x13, 0xD4, 0x00, 0xEB, 0xC2,
                0x7D,
                0x03, 0x83, 0xC1, 0x01, 0x83, 0xC0, 0x32, 0x83, 0xC1, 0x32, 0x3B, 0x0D, 0xEC, 0xBC, 0xCA, 0x00,
                0x7E,
                0x03, 0x83, 0xE9, 0x01, 0x3B, 0x05, 0xF0, 0xBC, 0xCA, 0x00, 0x7E, 0x03, 0x83, 0xE8, 0x01, 0x83,
                0xE9,
                0x32, 0x83, 0xE8, 0x32, 0x89, 0x0D, 0xF8, 0x13, 0xD4, 0x00, 0x89, 0x05, 0xFC, 0x13, 0xD4, 0x00,
                0x89,
                0xEC, 0x5D, 0xE9, 0xB4, 0xF7, 0xFF, 0xFF, 0xEC, 0x5D, 0xC3, 0xC3
            }
        },
        {
            0x469183, std::vector<uint8_t>{
                0x83, 0xF8, 0x32, 0x7D, 0x03, 0x83, 0xC0, 0x01, 0x83, 0xF9, 0x32, 0xEB, 0x31
            }
        }
    };

    for (const auto &[pos, data]: patches) {
        writeBytesAt(pos, data);
    }

    wowExe.close();
    std::cout << "Patching completed successfully.\n";

    return errorState;
}
