
#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <format>
#include <print>

namespace FileTools {


void ReplacePointByCommaInPlace(std::filesystem::path const& thePath) {
   constexpr std::size_t uBufferSize = 64u * 1024u;

   std::fstream theFile { thePath, std::ios::in | std::ios::out | std::ios::binary };

   if (!theFile) {
      throw std::runtime_error(std::format("Opening file failed for '{}'", thePath.string()));
      }

   std::array<char, uBufferSize> aBuffer {};

   while (theFile.read(aBuffer.data(), static_cast<std::streamsize>(aBuffer.size()))
          || theFile.gcount() > 0) {
      std::streamsize const iCount = theFile.gcount();

      std::ranges::replace(aBuffer.begin(), aBuffer.begin() + iCount, '.', ',');

      theFile.clear();

      theFile.seekp(-iCount, std::ios::cur);
      if (!theFile) {
         throw std::runtime_error(std::format("Seeking file failed for '{}'", thePath.string()));
         }

      theFile.write(aBuffer.data(), iCount);
      if (!theFile) {
         throw std::runtime_error(std::format("Writing file failed for '{}'", thePath.string()));
         }

      theFile.flush();

      if (!theFile) {
         throw std::runtime_error(std::format("Flushing file failed for '{}'", thePath.string()));
         }
      }

   if (!theFile.eof()) {
      throw std::runtime_error(std::format("Reading file failed for '{}'", thePath.string()));
      }
   }


 }

int main(void) {
   try {
   std::filesystem::path file = std::filesystem::path { "e:" } / "Test" / "testdata_20000_rows.txt";
      FileTools::ReplacePointByCommaInPlace(file);
      std::println("file '{}' processed successfully", file.string());
      }
   catch (std::exception const& ex) {
      std::println("error while processing file: '{}'", ex.what());
      }

   return 0;
   }
